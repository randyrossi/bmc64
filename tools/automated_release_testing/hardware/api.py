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

import base64
import json
from urllib.parse import urlencode, urlsplit
from urllib.request import Request, build_opener, HTTPRedirectHandler, ProxyHandler


class NoRedirect(HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        raise RuntimeError("unexpected HTTP redirect; check the BMC64 base URL")


class Api:
    def __init__(self, url: str, pin: str | None = None, timeout: float = 15):
        parsed = urlsplit(url)
        if (parsed.scheme not in ("http", "https") or not parsed.hostname
                or parsed.username or parsed.password or parsed.query
                or parsed.fragment or parsed.path not in ("", "/")):
            raise ValueError("use a base HTTP(S) URL without credentials or a path")
        self.url = url.rstrip("/")
        self.timeout = timeout
        self.headers = {"X-BMC64-Web": "1"}
        if pin is not None:
            encoded = base64.b64encode(f"bmc64:{pin}".encode()).decode("ascii")
            self.headers["Authorization"] = f"Basic {encoded}"
        self.opener = build_opener(ProxyHandler({}), NoRedirect())

    def use_address(self, address: str) -> None:
        """Talk to this IP address from now on, keeping the scheme and port."""
        parsed = urlsplit(self.url)
        host = f"[{address}]" if ":" in address else address
        self.url = f"{parsed.scheme}://{host}" + (f":{parsed.port}" if parsed.port else "")

    def request(self, endpoint: str, data: bytes | None = None,
                timeout: float | None = None, **query: str) -> bytes:
        suffix = "?" + urlencode(query) if query else ""
        request = Request(self.url + endpoint + suffix, data=data,
                          headers=self.headers)
        with self.opener.open(request, timeout=timeout or self.timeout) as response:
            return response.read()

    def get_json(self, endpoint: str, **query: str) -> dict[str, object]:
        result = json.loads(self.request(endpoint, **query))
        if not isinstance(result, dict):
            raise RuntimeError(f"{endpoint} did not return a JSON object")
        return result
