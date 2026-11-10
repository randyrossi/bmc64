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


"""Profile networking shared by rig configuration and test preflight."""

import re
from urllib.error import HTTPError

from framework.errors import TestFailure
from framework.reporting import progress
from hardware.api import Api


NETWORK_DEVICES = {"wifi": "2", "ethernet": "1"}
CONNECTED_STATUS = {"wifi": 13, "ethernet": 3}


def read_settings(api: Api, profile: dict[str, str]) -> tuple[str, bytes]:
    folder = f"/profiles/{profile['id']}"
    path = f"{folder}/settings-{profile['machine'].lower()}.txt"
    try:
        data = api.request("/api/fs/download", vol=profile["volume"], path=path)
    except HTTPError as error:
        if error.code != 404:
            raise
        path = f"{folder}/settings.txt"
        progress(f"{profile['id']}: machine settings missing; reading legacy {path}")
        data = api.request("/api/fs/download", vol=profile["volume"], path=path)
    return path, data


def setting(data: bytes, key: str) -> str | None:
    values = re.findall(r"^[ \t]*" + re.escape(key) + r"[ \t]*=[ \t]*([^\r\n]*)",
                        data.decode("utf-8-sig"), re.MULTILINE)
    if len(values) > 1:
        raise ValueError(f"duplicate {key} setting")
    return values[0].strip() if values else None


def network_settings(data: bytes, network: str) -> bytes:
    value = NETWORK_DEVICES[network]
    previous = setting(data, "network_device")
    text = data.decode("utf-8-sig")
    if previous is None:
        newline = "\r\n" if "\r\n" in text else "\n"
        if text and not text.endswith(("\r", "\n")):
            text += newline
        text += f"network_device={value}{newline}"
    else:
        text = re.sub(r"^([ \t]*network_device[ \t]*=[ \t]*)[^\r\n]*",
                      lambda match: match[1] + value, text, flags=re.MULTILINE)
    bom = b"\xef\xbb\xbf" if data.startswith(b"\xef\xbb\xbf") else b""
    return bom + text.encode("utf-8")


def validate_network(api: Api, profiles: list[dict[str, str]], network: str | None) -> None:
    if network is None:
        return
    expected = NETWORK_DEVICES[network]
    for profile in profiles:
        path, data = read_settings(api, profile)
        if setting(data, "network_device") != expected:
            raise TestFailure(
                "profile_network",
                f"{path}: expected {network} (network_device={expected}); "
                "run ./configure_test_setup to update the test profiles")
