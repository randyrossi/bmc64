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

import json
import re
import sys
import time
from pathlib import Path
from typing import Callable
from urllib.error import HTTPError
from framework.errors import TestFailure
from framework.reporting import WaitProgress, progress, save_json
from hardware.api import Api
from support.constants import MACHINES


def read_manifest(path: Path) -> list[dict[str, str]]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, list) or not value:
        raise ValueError("suite manifest must be a nonempty JSON array")
    seen = set()
    for item in value:
        if (not isinstance(item, dict) or set(item) != {"machine", "profile"}
                or not isinstance(item["machine"], str) or item["machine"] not in MACHINES
                or not isinstance(item["profile"], str) or not item["profile"].strip()
                or any(char in item["profile"] for char in "\r\n\0")):
            raise ValueError("each suite case needs a supported machine and a profile name/ID")
        if item["machine"] in seen:
            raise ValueError(f"duplicate machine: {item['machine']}")
        seen.add(item["machine"])
    return value


def profile_values(data: bytes) -> dict[str, str]:
    result = {}
    for line in data.decode("utf-8-sig").splitlines():
        line = line.strip()
        if not line or line.startswith(("#", ";")):
            continue
        key, separator, value = line.partition("=")
        if not separator:
            raise ValueError("invalid profile.txt line: " + line)
        key = key.strip()
        if key in result:
            raise ValueError("duplicate profile.txt key: " + key)
        result[key] = value.strip()
    return result


def resolve_profiles(api: Api, manifest: list[dict[str, str]]) -> list[dict[str, str]]:
    listing = api.get_json("/api/fs/list", path="/profiles")
    entries, volume = listing.get("entries"), listing.get("vol")
    if (not isinstance(entries, list) or not isinstance(volume, str) or not volume
            or listing.get("truncated") is not False):
        raise TestFailure("profile_preflight", "invalid or truncated /profiles listing")
    profiles = []
    for entry in entries:
        if not isinstance(entry, dict):
            raise TestFailure("profile_preflight", "malformed /profiles directory entry")
        identity = entry.get("name")
        if entry.get("dir") is not True or identity == "main":
            continue
        if not isinstance(identity, str) or not re.fullmatch(r"[a-z0-9-]{1,32}", identity):
            raise TestFailure("profile_preflight", f"invalid profile directory ID: {identity!r}")
        values = profile_values(api.request(
            "/api/fs/download", vol=volume, path=f"/profiles/{identity}/profile.txt"))
        profiles.append({**values, "id": identity})
    resolved = []
    for case in manifest:
        matches = [item for item in profiles
                   if case["profile"] in (item["id"], item.get("name"))]
        if len(matches) != 1:
            raise TestFailure("profile_preflight",
                              f"{case['profile']}: expected one name/ID match; found {len(matches)}")
        profile = matches[0]
        machine = profile.get("machine", "").split("/", 1)[0]
        if machine != case["machine"]:
            raise TestFailure("profile_preflight",
                              f"{profile['id']}: expected {case['machine']}, configured {machine}")
        attached = [key for key, value in profile.items()
                    if value and (key == "autostart" or key.startswith("disk_"))]
        if attached:
            raise TestFailure("profile_preflight",
                              f"{profile['id']}: remove test-profile media: {', '.join(attached)}")
        resolved.append({**case, "id": profile["id"], "name": profile.get("name", ""),
                         "volume": volume})
    if len({case["id"] for case in resolved}) != len(resolved):
        raise TestFailure("profile_preflight", "multiple cases resolve to the same profile")
    return resolved


def status(api: Api) -> dict[str, object]:
    value = api.get_json("/api/status")
    uptime = value.get("uptime_secs")
    if (not isinstance(value.get("profile_id"), str)
            or not re.fullmatch(r"[a-z0-9-]{1,32}", value["profile_id"])
            or not isinstance(value.get("machine"), str) or value["machine"] not in MACHINES
            or isinstance(uptime, bool) or not isinstance(uptime, int) or uptime < 0
            or not isinstance(value.get("profile_error"), str)):
        raise TestFailure("api_status", "status lacks valid profile, machine, uptime or profile_error")
    return value


def transient(error: OSError) -> bool:
    return not isinstance(error, HTTPError) or error.code >= 500


def recover_api(api: Api, timeout: float, label: str = "HTTP control") -> dict[str, object]:
    deadline = time.monotonic() + timeout
    last_error = "no response"
    updates = WaitProgress(label, timeout)
    while time.monotonic() < deadline:
        try:
            current = status(api)
        except OSError as error:
            if not transient(error):
                raise
            last_error = str(error)
        else:
            updates.update("available", force=True)
            return current
        updates.update("still waiting for the status endpoint")
        time.sleep(0.3)
    raise TestFailure("http_unavailable", f"{label} did not return: {last_error}")


def switch_profile(api: Api, case: dict[str, str], timeout: float,
                   health: Callable[[], None], output: Path) -> dict[str, object]:
    before = status(api)
    save_json(output / "status-before.json", before)
    restarting = before["profile_id"] == case["id"]
    endpoint = "/api/reboot" if restarting else "/api/profiles/start"
    query = {} if restarting else {"id": case["id"], "once": "1"}
    action = "reboot" if restarting else "profile start"
    progress(f"{case['machine']}: {action} request via {endpoint}")
    requested = time.monotonic()
    updates = WaitProgress(f"{case['machine']} profile switch to {case['id']}", timeout)
    try:
        reply = json.loads(api.request(endpoint, b"", **query))
    except HTTPError:
        raise
    except OSError as error:
        # A restart can drop the HTTP reply after the start request was applied.
        save_json(output / "switch-request.json", {
            "endpoint": endpoint, "outcome": "unknown", "error": str(error), "retried": False,
        })
        print(f"{case['machine']}: {action} reply lost ({error}); "
              "verifying profile boot without retrying the request", file=sys.stderr, flush=True)
    else:
        if not isinstance(reply, dict) or reply.get("ok") is not True:
            raise TestFailure("profile_switch", f"{action} was not accepted")
        save_json(output / "switch-request.json", {"endpoint": endpoint, "outcome": "accepted"})
    deadline = requested + timeout
    last = before
    detail = "waiting for restart and verified profile/core"
    with (output / "status-poll.jsonl").open("w", encoding="utf-8") as log:
        while time.monotonic() < deadline:
            updates.update(detail)
            health()
            try:
                current = status(api)
            except OSError as error:
                if not transient(error):
                    raise
                log.write(json.dumps({"monotonic": time.monotonic(), "error": str(error)}) + "\n")
                detail = "HTTP temporarily unavailable during restart"
            else:
                last = current
                detail = (f"reported {current['profile_id']} / {current['machine']}, "
                          f"uptime {current['uptime_secs']}s; waiting for verified transition")
                log.write(json.dumps({"monotonic": time.monotonic(), **current}) + "\n")
                transitioned = (before["profile_id"] != current["profile_id"]
                                or current["uptime_secs"] < before["uptime_secs"]
                                or current["uptime_secs"] < time.monotonic() - requested - 1)
                # Reboot does not clear the previous start error until the new boot.
                stale_error = (restarting and not transitioned
                               and current["profile_error"] == before["profile_error"])
                if current["profile_error"] and not stale_error:
                    raise TestFailure("profile_switch", f"profile start failed: {current['profile_error']}")
                if (current["profile_id"] == case["id"]
                        and current["machine"] == case["machine"] and transitioned):
                    save_json(output / "status-booted.json", current)
                    updates.update("profile and core verified", force=True)
                    return current
            log.flush()
            time.sleep(0.3)
    raise TestFailure("profile_switch_timeout",
                      f"no verified switch to {case['id']} / {case['machine']}; last status: {last}")
