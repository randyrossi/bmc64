#!/usr/bin/env python3
#
# BMC64 - A bare metal Commodore emulator for Raspberry Pi
# Copyright (C) 2026 aminch <https://github.com/aminch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

"""The Web UI upload batch test against an in-memory stand-in for BMC64."""

import argparse
import io
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
from urllib.error import HTTPError, URLError

from framework.context import TestContext
from framework.errors import TestFailure
from framework.rig import Rig
from tests.webui import test_upload_batch
from tests.webui.test_upload_batch import Test

SMALL_BATCH = [3000, 5000, 70000]


class FakeApi:
    """Keeps uploaded files in a dict; `fail` maps an upload number to an error."""

    url = "http://127.0.0.1"

    def __init__(self, fail: dict[int, Exception] | None = None, corrupt: bool = False,
                 free_kb: int = 1024 * 1024):
        self.free_kb = free_kb
        self.files: dict[str, bytes] = {}
        self.folders: set[str] = set()
        self.fail = fail or {}
        self.corrupt = corrupt
        self.uploads = 0
        self.deleted: list[str] = []

    def request(self, endpoint, data=None, timeout=None, **query):
        path = query.get("path")
        if endpoint == "/api/fs/mkdir":
            self.folders.add(path)
        elif endpoint == "/api/fs/upload":
            self.uploads += 1
            if self.uploads in self.fail:
                raise self.fail[self.uploads]
            self.files[path] = bytes(data)
        elif endpoint == "/api/fs/download":
            body = self.files[path]
            return body[:-1] if self.corrupt else body
        elif endpoint == "/api/fs/delete":
            self.deleted.append(path)
        return b""

    def get_json(self, endpoint, **query):
        if endpoint == "/api/status":
            return {"profile_id": "c64-test", "machine": "C64"}
        if endpoint == "/api/volumes":
            return {"volumes": [{"id": "SD", "kind": "sdcard", "free_kb": self.free_kb}]}
        folder = query["path"]
        return {"entries": [{"name": name.rsplit("/", 1)[1], "size": len(body)}
                            for name, body in self.files.items()
                            if name.startswith(folder + "/")]}


def quiet_record() -> dict:
    return {"host_tcp": None, "bmc64_log": None, "header_ms": None}


class WebUiUploadTest(unittest.TestCase):
    def run_case(self, api: FakeApi) -> tuple[Test, dict]:
        directory = Path(self.enterContext(tempfile.TemporaryDirectory()))
        report = {"run_id": "abc123", "checks": []}
        context = TestContext(argparse.Namespace(execute=True, timeout=5), directory, report,
                              rig=Rig(api=api, pico=None, capture=None, uart=None),
                              profile={"id": "c64-test", "volume": "SD", "machine": "C64"})
        test = Test()
        with patch.object(test_upload_batch, "BATCH", SMALL_BATCH), \
                patch.object(Test, "start_profile"), \
                patch("tests.webui.test_upload_batch.progress"):
            test.invoke(context)
        return test, report

    def test_every_file_is_uploaded_listed_read_back_and_removed(self):
        api = FakeApi()
        _, report = self.run_case(api)
        self.assertEqual(report["status"], "PASS")
        self.assertEqual(report["checks"], ["upload_batch", "listing_sizes", "read_back"])
        self.assertEqual([len(body) for body in api.files.values()], SMALL_BATCH)
        self.assertEqual(api.folders, {"/bmc64-upload-abc123"})
        self.assertEqual(api.deleted, ["/bmc64-upload-abc123"])

    def test_file_contents_repeat_for_a_run_and_differ_between_files(self):
        self.assertEqual(test_upload_batch.file_data("r", 1, 64),
                         test_upload_batch.file_data("r", 1, 64))
        self.assertNotEqual(test_upload_batch.file_data("r", 1, 64),
                            test_upload_batch.file_data("r", 2, 64))

    def test_a_rejected_upload_fails_with_its_http_reply(self):
        error = HTTPError("http://pi/api/fs/upload", 400, "Bad Request", {},
                          io.BytesIO(b"upload truncated\n"))
        api = FakeApi(fail={2: error})
        with self.assertRaises(TestFailure) as raised:
            self.run_case(api)
        self.assertEqual(raised.exception.category, "upload_rejected")
        self.assertIn("upload 2/3", str(raised.exception))
        self.assertIn("HTTP 400", str(raised.exception))
        self.assertIn("upload truncated", str(raised.exception))
        # The folder is still removed after a failure.
        self.assertEqual(api.deleted, ["/bmc64-upload-abc123"])

    def test_an_upload_with_no_answer_is_reported_as_stalled(self):
        api = FakeApi(fail={3: URLError(TimeoutError("timed out"))})
        with self.assertRaises(TestFailure) as raised:
            self.run_case(api)
        self.assertEqual(raised.exception.category, "upload_stalled")
        self.assertIn("upload 3/3", str(raised.exception))

    def test_too_little_free_space_fails_before_uploading(self):
        api = FakeApi(free_kb=100)
        with self.assertRaises(TestFailure) as raised:
            self.run_case(api)
        self.assertEqual(raised.exception.category, "free_space")
        self.assertEqual(api.uploads, 0)

    def test_large_files_get_longer_to_answer(self):
        self.assertEqual(test_upload_batch.request_timeout(0), test_upload_batch.REQUEST_TIMEOUT)
        self.assertGreater(test_upload_batch.request_timeout(64 * 1024 * 1024), 300)

    def test_each_upload_records_the_pcs_tcp_counter_changes(self):
        readings = iter([{"RetransSegs": 10, "TCPTimeouts": 1},
                         {"RetransSegs": 13, "TCPTimeouts": 2}] * len(SMALL_BATCH))
        with patch.object(test_upload_batch, "host_tcp_counters", lambda: next(readings)):
            _, report = self.run_case(FakeApi())
        self.assertEqual([record["host_tcp"] for record in report["uploads"]],
                         [{"RetransSegs": 3, "TCPTimeouts": 1}] * len(SMALL_BATCH))
        self.assertEqual(report["upload_summary"]["retransmitted_segments"],
                         3 * len(SMALL_BATCH))

    def test_counters_that_cannot_be_read_are_left_out(self):
        self.assertIsNone(test_upload_batch.counter_change(None, {"RetransSegs": 1}))
        with patch.object(test_upload_batch, "host_tcp_counters", lambda: None):
            _, report = self.run_case(FakeApi())
        self.assertEqual(report["status"], "PASS")
        self.assertTrue(all(record["host_tcp"] is None for record in report["uploads"]))

    def test_the_progress_line_names_resends_long_waits_and_slow_headers(self):
        record = {"host_tcp": {"RetransSegs": 4, "TCPTimeouts": 1},
                  "bmc64_log": {"longest_wait_ms": 1200, "long_waits": 3},
                  "header_ms": 3000,
                  "dropped": {"ms": 8000, "bytes": 0, "client": "192.168.68.5:41234",
                              "this_pc": True}}
        self.assertEqual(test_upload_batch.describe(record),
                         " — PC resent 4 segment(s), 1 after a timeout; "
                         "BMC64 longest wait 1200 ms, 3 wait(s) of 100 ms+; "
                         "headers took 3000 ms; "
                         "BMC64 first dropped another connection after 8000 ms "
                         "(0 bytes, from 192.168.68.5:41234, this PC)")
        older = {**quiet_record(), "dropped": {"ms": 8000}}
        self.assertEqual(test_upload_batch.describe(older),
                         " — BMC64 first dropped another connection after 8000 ms")

    def test_dropped_connection_lines_old_and_new_are_read(self):
        new = ("webui: Dropped a connection after 8000 ms: Receive returned 0 with 0 "
               "bytes of request, from 192.168.68.5:41234")
        match = test_upload_batch.DROPPED.search(new)
        self.assertEqual(match.groups(), ("8000", "0", "192.168.68.5", "41234"))
        old = ("webui: Dropped a connection after 8000 ms: Receive returned 0 with 0 "
               "bytes of request")
        self.assertEqual(test_upload_batch.DROPPED.search(old).groups(),
                         ("8000", None, None, None))
        quiet = {"host_tcp": {"RetransSegs": 0}, "bmc64_log": None, "header_ms": None}
        self.assertEqual(test_upload_batch.describe(quiet), "")

    def test_the_waits_are_read_from_bmc64s_received_line(self):
        line = ("webui: Received 1048576 bytes in 3386 ms (309 KB/s), 17 batches of "
                "61680 bytes avg; waiting 3199 ms, SD writes 173 ms, other tasks 12 ms; "
                "longest wait 2900 ms, 2 waits of 100 ms or more")
        waits = test_upload_batch.WAITS.search(line)
        self.assertEqual((waits[1], waits[2]), ("2900", "2"))
        self.assertEqual(test_upload_batch.RECEIVED_LINE.search(line).groups(),
                         ("1048576", "3386", "309", "17"))

    def test_the_short_received_line_of_a_normal_upload_is_read(self):
        line = "webui: Received 1048576 bytes in 690 ms (1519 KB/s)"
        self.assertEqual(test_upload_batch.RECEIVED_LINE.search(line).groups(),
                         ("1048576", "690", "1519", None))
        self.assertIsNone(test_upload_batch.WAITS.search(line))

    def test_a_file_that_reads_back_different_fails(self):
        with self.assertRaises(TestFailure) as raised:
            self.run_case(FakeApi(corrupt=True))
        self.assertEqual(raised.exception.category, "upload_corrupt")


if __name__ == "__main__":
    unittest.main()
