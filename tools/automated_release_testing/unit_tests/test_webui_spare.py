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

"""The Web UI spare connection test against a local stand-in for BMC64."""

import argparse
import socket
import tempfile
import threading
import time
import unittest
from pathlib import Path
from unittest.mock import patch

from framework.context import TestContext
from framework.errors import TestFailure
from framework.rig import Rig
from tests.webui import test_spare_connection
from tests.webui.test_spare_connection import Test

STATUS = b'{"profile_id": "c64-test", "machine": "C64"}'


class FakeServer:
    """Answers GET /api/status on each connection. With `idle_close`, a
    connection that sends nothing for that many seconds is closed instead,
    as BMC64 did before it kept idle connections open."""

    def __init__(self, idle_close: float | None = None):
        self.idle_close = idle_close
        self.listener = socket.create_server(("127.0.0.1", 0))
        self.port = self.listener.getsockname()[1]
        self.requests: list[bytes] = []
        threading.Thread(target=self.serve, daemon=True).start()

    def serve(self) -> None:
        while True:
            try:
                connection, _ = self.listener.accept()
            except OSError:
                return
            threading.Thread(target=self.answer, args=(connection,), daemon=True).start()

    def answer(self, connection: socket.socket) -> None:
        with connection:
            connection.settimeout(self.idle_close or 30)
            try:
                request = connection.recv(4096)
            except socket.timeout:
                return
            self.requests.append(request)
            connection.sendall(b"HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                               b"Content-Length: %d\r\nConnection: close\r\n\r\n"
                               % len(STATUS) + STATUS)

    def close(self) -> None:
        self.listener.close()


class FakeApi:
    def __init__(self, server: FakeServer, delay: float = 0.0):
        self.url = f"http://127.0.0.1:{server.port}"
        self.headers = {"X-BMC64-Web": "1"}
        self.delay = delay

    def get_json(self, endpoint, **query):
        time.sleep(self.delay)
        return {"profile_id": "c64-test", "machine": "C64"}


class WebUiSpareTest(unittest.TestCase):
    def run_case(self, server: FakeServer, api_delay: float = 0.0) -> dict:
        self.addCleanup(server.close)
        directory = Path(self.enterContext(tempfile.TemporaryDirectory()))
        report = {"run_id": "abc123", "checks": []}
        context = TestContext(argparse.Namespace(execute=True, timeout=5), directory, report,
                              rig=Rig(api=FakeApi(server, api_delay), pico=None,
                                      capture=None, uart=None),
                              profile={"id": "c64-test", "volume": "SD", "machine": "C64"})
        with patch.object(test_spare_connection, "SPARE_DELAYS", (0.3,)), \
                patch.object(Test, "start_profile"), \
                patch("tests.webui.test_spare_connection.progress"):
            Test().invoke(context)
        return report

    def test_a_spare_that_waited_gets_an_answer(self):
        server = FakeServer()
        report = self.run_case(server)
        self.assertEqual(report["status"], "PASS")
        self.assertEqual(report["checks"], ["idle_spare_answered", "spare_does_not_hold_up"])
        self.assertEqual(report["spares"][0]["status"], 200)
        self.assertTrue(server.requests[0].startswith(b"GET /api/status HTTP/1.1\r\n"))
        self.assertIn(b"X-BMC64-Web: 1\r\n", server.requests[0])

    def test_a_spare_closed_while_idle_fails(self):
        with self.assertRaises(TestFailure) as raised:
            self.run_case(FakeServer(idle_close=0.1))
        self.assertEqual(raised.exception.category, "spare_dropped")
        self.assertIn("after 0.3s idle", str(raised.exception))

    def test_a_request_held_up_by_a_spare_fails(self):
        with patch.object(test_spare_connection, "HELD_UP_LIMIT", 0.1):
            with self.assertRaises(TestFailure) as raised:
                self.run_case(FakeServer(), api_delay=0.3)
        self.assertEqual(raised.exception.category, "spare_holds_up")

    def test_only_plain_http_is_supported(self):
        server = FakeServer()
        self.addCleanup(server.close)
        test = Test()
        test.context = TestContext(argparse.Namespace(execute=True, timeout=5),
                                   Path(self.enterContext(tempfile.TemporaryDirectory())),
                                   {"run_id": "abc123", "checks": []},
                                   rig=Rig(api=FakeApi(server), pico=None, capture=None,
                                           uart=None),
                                   profile={"id": "c64-test", "volume": "SD", "machine": "C64"})
        test.api.url = "https://127.0.0.1"
        with self.assertRaises(TestFailure) as raised:
            test.address()
        self.assertEqual(raised.exception.category, "spare_setup")


if __name__ == "__main__":
    unittest.main()
