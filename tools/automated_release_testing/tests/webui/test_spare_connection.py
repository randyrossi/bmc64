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

"""Use the Web UI the way a browser that opens spare connections does.

Browsers (Safari in particular) open connections before they need them and
send a request on one seconds later. Each spare here is a plain TCP
connection that sends nothing for a while and then asks for /api/status; it
must get an answer. A spare sitting idle must also not hold up a request on
another connection.
"""

import json
import socket
import time
from urllib.parse import urlsplit

from framework.case import BmcTest
from framework.errors import TestFailure
from framework.reporting import progress, save_json

# How long each spare stays idle before its request: past the 2 s after
# which older BMC64 builds close an idle connection, and much longer.
SPARE_DELAYS = (3.0, 10.0)
# A request on a new connection while a spare is idle: answered within this.
HELD_UP_LIMIT = 1.0
# Waiting for a reply once a request is sent.
REPLY_TIMEOUT = 15.0


class Test(BmcTest):
    name = "webui-spare-connection"
    description = "Web UI: answer requests on connections that sat idle, without holding up others"
    machine = "C64"
    order = 90

    def before_test(self) -> None:
        # Keep an already running C64 test profile; this needs no fresh boot.
        self.start_profile(fresh=False)

    def run_test(self) -> None:
        spares = []
        self.report["spares"] = spares

        self.stage("idle_spare")
        for delay in SPARE_DELAYS:
            self.health()
            progress(f"Opening a spare connection and leaving it idle for {delay:g}s")
            spare = self.connect()
            try:
                time.sleep(delay)
                started = time.monotonic()
                status, body = self.ask_status(spare, f"after {delay:g}s idle")
                seconds = time.monotonic() - started
            finally:
                spare.close()
            spares.append({"idle_s": delay, "status": status,
                           "reply_s": round(seconds, 3)})
            progress(f"Answered after {delay:g}s idle: HTTP {status} in {seconds:.2f}s")
        self.passed("idle_spare_answered")

        self.stage("spare_holds_up")
        self.health()
        progress("Asking on a new connection while a spare connection is idle")
        spare = self.connect()
        try:
            started = time.monotonic()
            self.api.get_json("/api/status")
            seconds = time.monotonic() - started
        finally:
            spare.close()
        self.report["held_up_s"] = round(seconds, 3)
        progress(f"Answered in {seconds:.2f}s with a spare connection idle")
        if seconds > HELD_UP_LIMIT:
            raise TestFailure("spare_holds_up", f"a request took {seconds:.2f}s while "
                              f"another connection sat idle (limit {HELD_UP_LIMIT:g}s)")
        self.passed("spare_does_not_hold_up")

        self.stage("final_status")
        progress("Checking the Web UI still answers")
        save_json(self.output / "status-after.json", self.api.get_json("/api/status"))
        save_json(self.output / "spares.json", spares)

    # Steps used above.

    def address(self) -> tuple[str, int]:
        parts = urlsplit(self.api.url)
        if parts.scheme != "http":
            raise TestFailure("spare_setup", f"needs a plain HTTP Web UI, not {self.api.url}")
        return parts.hostname, parts.port or 80

    def connect(self) -> socket.socket:
        """A connection that sends nothing yet, as a browser's spare."""
        try:
            return socket.create_connection(self.address(), timeout=REPLY_TIMEOUT)
        except OSError as error:
            raise TestFailure("spare_connect", f"cannot connect to the Web UI: "
                              f"{error}") from error

    def ask_status(self, connection: socket.socket, when: str) -> tuple[int, dict]:
        """Send GET /api/status on CONNECTION and read the whole reply."""
        host, port = self.address()
        headers = [f"GET /api/status HTTP/1.1", f"Host: {host}:{port}",
                   "Connection: close"]
        headers += [f"{name}: {value}" for name, value in self.api.headers.items()]
        request = ("\r\n".join(headers) + "\r\n\r\n").encode("ascii")
        reply = b""
        try:
            connection.sendall(request)
            while True:
                chunk = connection.recv(65536)
                if not chunk:
                    break
                reply += chunk
        except OSError as error:
            raise TestFailure("spare_dropped", f"the request on a spare connection {when} "
                              f"failed: {error}") from error
        if not reply:
            raise TestFailure("spare_dropped", f"a spare connection {when} was closed "
                              "without an answer")
        head, _, body = reply.partition(b"\r\n\r\n")
        line = head.split(b"\r\n", 1)[0].decode("latin-1")
        parts = line.split()
        if len(parts) < 2 or not parts[1].isdigit():
            raise TestFailure("spare_reply", f"a spare connection {when} got {line!r}")
        status = int(parts[1])
        if status != 200:
            raise TestFailure("spare_reply", f"a spare connection {when} got HTTP {status}")
        try:
            return status, json.loads(body)
        except ValueError as error:
            raise TestFailure("spare_reply", f"a spare connection {when} got a reply "
                              "that isn't JSON") from error
