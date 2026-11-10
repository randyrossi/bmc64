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

"""Upload a batch of files through the Web UI and read every one back.

Uploads them one request at a time, back to back, as the Files view does
when several files are dropped or picked at once.
"""

import hashlib
import random
import re
import socket
import time
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.parse import urlsplit

from framework.case import BmcTest
from framework.errors import TestFailure
from framework.reporting import progress, save_json

# Many disk-image-sized files, then 1 MB files, then large files, ending
# with one bigger than a BMC64 update package (about 51 MB): about 111 MB in
# one go, well past the several MB that has made uploads stall.
D64_SIZE = 174848
MB = 1024 * 1024
BATCH = [D64_SIZE] * 30 + [MB] * 10 + [16 * MB] * 2 + [64 * MB]
# BMC64 gives up on a stalled upload after 8 s and answers; no answer at all
# within this means the Web UI is stuck. Large files get longer, allowing
# for a slow 256 KB/s.
REQUEST_TIMEOUT = 60.0
# How long to wait for the UART line BMC64 logs after each upload.
UART_LINE_WAIT = 3.0
# BMC64 logs a short Received line for a normal upload; when something
# waited 100 ms or more it adds batches, where the time went, the longest
# single wait for data and how many waits took 100 ms or more.
RECEIVED_LINE = re.compile(
    r"Received (\d+) bytes in (\d+) ms \((\d+) KB/s\)(?:, (\d+) batches)?")
WAITS = re.compile(r"longest wait (\d+) ms, (\d+) waits of 100 ms or more")
# Logged when a request's headers were slow to arrive.
SLOW_HEADERS = re.compile(r"Request headers took (\d+) ms")
# Logged when a connection is dropped part way through a request; the Web
# UI handles one connection at a time, so this holds up the next one.
# (Idle connections that sent nothing are dropped after 2 s without a log.)
DROPPED = re.compile(r"Dropped a connection after (\d+) ms"
                     r"(?:.*? with (\d+) bytes of request, from ([\d.]+):(\d+))?")
# What BMC64 logs when an upload stops arriving before it is complete.
TRUNCATED_LINE = re.compile(r"Upload truncated at \d+ of \d+ bytes")
# This PC's TCP counters (Linux), read around each upload: resent segments
# show the network lost data. They count all of the PC's TCP traffic, so
# keep other network use low during a run.
HOST_COUNTERS = {
    "/proc/net/snmp": ("Tcp", ("RetransSegs",)),
    "/proc/net/netstat": ("TcpExt", ("TCPTimeouts", "TCPFastRetrans",
                                     "TCPSynRetrans", "TCPLossProbes")),
}


def request_timeout(size: int) -> float:
    return REQUEST_TIMEOUT + size / (256 * 1024)


def host_tcp_counters() -> dict[str, int] | None:
    """This PC's TCP retransmit counters, or None where they can't be read."""
    counters = {}
    try:
        for path, (prefix, names) in HOST_COUNTERS.items():
            rows = [line.split() for line in Path(path).read_text().splitlines()
                    if line.startswith(prefix + ":")]
            if len(rows) < 2:
                continue
            values = dict(zip(rows[0][1:], rows[1][1:]))
            counters.update({name: int(values[name]) for name in names if name in values})
    except OSError:
        return None
    return counters or None


def counter_change(before: dict[str, int] | None,
                   after: dict[str, int] | None) -> dict[str, int] | None:
    if before is None or after is None:
        return None
    return {name: after[name] - before[name] for name in after if name in before}


def file_data(run_id: str, index: int, size: int) -> bytes:
    """Random bytes that depend only on the run ID and file, so a run can be repeated."""
    return random.Random(f"{run_id}-{index}").randbytes(size)


def describe(record: dict) -> str:
    """What the PC and BMC64 saw during one upload, for the progress line."""
    parts = []
    host = record["host_tcp"]
    if host and host.get("RetransSegs"):
        parts.append(f"PC resent {host['RetransSegs']} segment(s), "
                     f"{host.get('TCPTimeouts', 0)} after a timeout")
    log = record["bmc64_log"] or {}
    if log.get("long_waits"):
        parts.append(f"BMC64 longest wait {log['longest_wait_ms']} ms, "
                     f"{log['long_waits']} wait(s) of 100 ms+")
    if record["header_ms"] is not None:
        parts.append(f"headers took {record['header_ms']} ms")
    dropped = record.get("dropped")
    if dropped:
        text = f"BMC64 first dropped another connection after {dropped['ms']} ms"
        if dropped.get("client"):
            text += (f" ({dropped['bytes']} bytes, from {dropped['client']}"
                     + (", this PC" if dropped.get("this_pc") else ", not this PC") + ")")
        parts.append(text)
    return " — " + "; ".join(parts) if parts else ""


class Test(BmcTest):
    name = "webui-upload-batch"
    description = "Web UI: upload a several-MB batch of files back to back and read each back"
    machine = "C64"
    order = 80

    def before_test(self) -> None:
        # Keep an already running C64 test profile; uploads don't need a fresh boot.
        self.start_profile(fresh=False)
        self.volume = self.profile["volume"]
        self.stage("free_space")
        volumes = self.api.get_json("/api/volumes")["volumes"]
        free_kb = next((volume["free_kb"] for volume in volumes
                        if volume["id"] == self.volume), 0)
        needed_kb = sum(BATCH) // 1024 + 16 * 1024
        if free_kb < needed_kb:
            raise TestFailure("free_space", f"the SD card has {free_kb} KB free; "
                              f"the batch needs {needed_kb} KB")
        self.folder = f"/bmc64-upload-{self.report['run_id']}"
        self.stage("create_folder")
        progress(f"Creating {self.folder} for the uploads")
        self.api.request("/api/fs/mkdir", b"", timeout=15, vol=self.volume, path=self.folder)
        self.context.register("delete upload folder", lambda: self.api.request(
            "/api/fs/delete", b"", timeout=REQUEST_TIMEOUT,
            vol=self.volume, path=self.folder, recursive="1"))

    def run_test(self) -> None:
        run_id = self.report["run_id"]
        uploads = []
        self.report["uploads"] = uploads

        self.stage("upload_batch")
        total = sum(BATCH)
        progress(f"Uploading {len(BATCH)} files, {total // 1024} KB in all, back to back")
        for index, size in enumerate(BATCH, 1):
            path = f"{self.folder}/upload-{index:02d}.bin"
            data = file_data(run_id, index, size)
            self.health()
            uart_lines = self.uart_lines()
            counters = host_tcp_counters()
            seconds = self.upload(path, data, index, uart_lines)
            record = {"path": path, "size": size, "seconds": round(seconds, 2),
                      "kb_per_s": round(size / 1024 / seconds) if seconds else None,
                      "sha256": hashlib.sha256(data).hexdigest(),
                      "host_tcp": counter_change(counters, host_tcp_counters()),
                      "bmc64_log": self.wait_received_line(uart_lines, size),
                      "header_ms": self.slow_header_ms(uart_lines),
                      "dropped": self.dropped_connection(uart_lines)}
            uploads.append(record)
            progress(f"Uploaded {index}/{len(BATCH)}: {size // 1024} KB in {seconds:.1f}s"
                     + (f" ({record['kb_per_s']} KB/s)" if record["kb_per_s"] else "")
                     + describe(record))
        self.passed("upload_batch")
        self.summarise(uploads)

        self.stage("list_folder")
        progress("Checking the folder lists every file at its full size")
        listing = self.api.get_json("/api/fs/list", vol=self.volume, path=self.folder)
        save_json(self.output / "listing.json", listing)
        sizes = {entry["name"]: entry.get("size") for entry in listing["entries"]}
        for record in uploads:
            name = record["path"].rsplit("/", 1)[1]
            if sizes.get(name) != record["size"]:
                raise TestFailure("upload_listing", f"{name} is listed with size "
                                  f"{sizes.get(name)}, expected {record['size']}")
        self.passed("listing_sizes")

        self.stage("read_back")
        progress("Downloading every file and comparing it with what was sent")
        for record in uploads:
            self.health()
            got = self.api.request("/api/fs/download", timeout=request_timeout(record["size"]),
                                   vol=self.volume, path=record["path"])
            if hashlib.sha256(got).hexdigest() != record["sha256"]:
                raise TestFailure("upload_corrupt", f"{record['path']} came back different "
                                  f"({len(got)} of {record['size']} bytes)")
        self.passed("read_back")

        self.stage("final_status")
        progress("Checking the Web UI still answers")
        save_json(self.output / "status-after.json", self.api.get_json("/api/status"))
        missing = [record["path"] for record in uploads if record["bmc64_log"] is None]
        if self.uart and missing:
            progress(f"BMC64 logged no 'Received' line for {len(missing)} upload(s)")
        save_json(self.output / "uploads.json", uploads)

    def after_test(self) -> None:
        pass

    # Steps used above.

    def upload(self, path: str, data: bytes, index: int, uart_lines: int) -> float:
        """POST one file as the Files view does; return the time it took."""
        started = time.monotonic()
        try:
            self.api.request("/api/fs/upload", data, timeout=request_timeout(len(data)),
                             vol=self.volume, path=path)
        except HTTPError as error:
            body = error.read().decode("utf-8", errors="replace").strip()
            message = (f"upload {index}/{len(BATCH)} ({path}) got HTTP {error.code} "
                       f"after {time.monotonic() - started:.1f}s: {body}")
            logged = self.uart_line_since(uart_lines, TRUNCATED_LINE)
            if logged:
                message += f"; BMC64: {logged}"
            raise TestFailure("upload_rejected", message) from error
        except (TimeoutError, URLError, OSError) as error:
            raise TestFailure("upload_stalled", f"upload {index}/{len(BATCH)} ({path}) "
                              f"got no answer after {time.monotonic() - started:.1f}s: "
                              f"{error}") from error
        return time.monotonic() - started

    def summarise(self, uploads: list[dict]) -> None:
        """Say whether the slow uploads are the ones where the PC resent data."""
        speeds = sorted(record["kb_per_s"] for record in uploads if record["kb_per_s"])
        if not speeds:
            return
        median = speeds[len(speeds) // 2]
        slow = [record for record in uploads
                if record["kb_per_s"] and record["kb_per_s"] < median / 2]
        resent = [record for record in slow
                  if record["host_tcp"] and record["host_tcp"].get("RetransSegs")]
        total = sum((record["host_tcp"] or {}).get("RetransSegs", 0) for record in uploads)
        summary = {"median_kb_per_s": median, "slow_uploads": len(slow),
                   "slow_with_retransmits": len(resent), "retransmitted_segments": total}
        self.report["upload_summary"] = summary
        progress(f"{len(slow)} of {len(uploads)} uploads ran under half the median "
                 f"{median} KB/s; {len(resent)} of them had PC retransmits; "
                 f"{total} segments resent in all")

    def slow_header_ms(self, since: int) -> int | None:
        """How long BMC64 says this request's headers took, if it was slow."""
        line = self.uart_line_since(since, SLOW_HEADERS, wait=False)
        match = SLOW_HEADERS.search(line) if line else None
        return int(match[1]) if match else None

    def dropped_connection(self, since: int) -> dict | None:
        """A connection BMC64 dropped during this upload: how long it waited, and
        (on newer builds) how much it received and which client opened it."""
        line = self.uart_line_since(since, DROPPED, wait=False)
        match = DROPPED.search(line) if line else None
        if not match:
            return None
        dropped = {"ms": int(match[1])}
        if match[3]:
            dropped.update(bytes=int(match[2]), client=f"{match[3]}:{match[4]}",
                           this_pc=match[3] == self.local_address())
        return dropped

    def local_address(self) -> str | None:
        """This PC's address on the route to BMC64, to compare with clients it logs."""
        host = urlsplit(self.api.url).hostname
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as probe:
                probe.connect((host, 80))  # UDP: picks a route, sends nothing
                return probe.getsockname()[0]
        except OSError:
            return None

    def uart_lines(self) -> int:
        """Lines in the BMC64 UART log so far, to look for new ones after an upload."""
        if not self.uart:
            return 0
        return len(Path(self.uart.file.name).read_text(errors="replace").splitlines())

    def uart_line_since(self, since: int, pattern: re.Pattern,
                        wait: bool = True) -> str | None:
        """The first BMC64 UART line after `since` matching `pattern`, waiting briefly."""
        if not self.uart:
            return None
        deadline = time.monotonic() + (UART_LINE_WAIT if wait else 0)
        while True:
            lines = Path(self.uart.file.name).read_text(errors="replace").splitlines()
            for line in lines[since:]:
                if pattern.search(line):
                    return line.split("webui: ", 1)[-1]
            if time.monotonic() > deadline:
                return None
            time.sleep(0.2)

    def wait_received_line(self, since: int, size: int) -> dict | None:
        """BMC64's log line for an upload of `size` bytes: speed and time breakdown."""
        if not self.uart:
            return None
        deadline = time.monotonic() + UART_LINE_WAIT
        while True:
            lines = Path(self.uart.file.name).read_text(errors="replace").splitlines()
            for line in lines[since:]:
                match = RECEIVED_LINE.search(line)
                if match and int(match[1]) == size:
                    found = {"ms": int(match[2]), "kb_per_s": int(match[3]),
                             "batches": int(match[4]) if match[4] else None,
                             "line": line.split("  ", 1)[-1]}
                    waits = WAITS.search(line)
                    if waits:
                        found.update(longest_wait_ms=int(waits[1]),
                                     long_waits=int(waits[2]))
                    return found
            if time.monotonic() > deadline:
                return None
            time.sleep(0.2)
