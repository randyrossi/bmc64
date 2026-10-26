"""Offline tests of preview-led setup, profile edits and network transitions."""

import io
import json
import subprocess
import tempfile
import unittest
from contextlib import ExitStack
from pathlib import Path
from unittest.mock import Mock, patch
from urllib.error import HTTPError, URLError

from framework.errors import TestFailure
from support.network import network_settings, read_settings, validate_network
from support.profiles import read_manifest
from utils.configure_test_setup import apply_network, configure, verify_restart
from utils.preview_video import preview
from utils.runner import HERE, load_setup, parser


class Rig:
    def __init__(self):
        self.manifest = read_manifest(HERE / "suite_profiles.json")
        self.cases = [{**case, "id": case["profile"], "volume": "SD"} for case in self.manifest]
        self.files = {}
        for case in self.cases:
            folder = f"/profiles/{case['id']}"
            self.files[f"{folder}/profile.txt"] = (
                f"name={case['id']}\nmachine={case['machine']}/PAL/HDMI\n").encode()
            self.files[f"{folder}/settings-{case['machine'].lower()}.txt"] = (
                b"# retain comments\r\nnetwork_device=2\r\nwebui_enabled=1\r\nvolume=75\r\n")
        self.current = {"profile_id": "main", "machine": "C64", "uptime_secs": 100,
                        "profile_error": "", "net_status": 13, "net_text": "connected (Wi-Fi)"}
        self.requests = []
        self.saved = []
        self.fail_save = None
        self.corrupt_readback = None
        self.lost_restart_reply = False
        self.wrong_network = False

    def get_json(self, endpoint, **query):
        if endpoint == "/api/status":
            return dict(self.current)
        if endpoint == "/api/fs/list":
            return {"vol": "SD", "truncated": False,
                    "entries": [{"name": case["id"], "dir": True} for case in self.cases]}
        raise AssertionError(endpoint)

    def request(self, endpoint, data=None, **query):
        self.requests.append((endpoint, data, query))
        if endpoint == "/api/fs/download":
            if query["path"] not in self.files:
                raise HTTPError("http://fixture", 404, "missing", {}, None)
            value = self.files[query["path"]]
            if query["path"] == self.corrupt_readback and query["path"] in self.saved:
                return value + b"unexpected=1\n"
            return value
        if endpoint == "/api/fs/save":
            self.files[query["path"]] = data
            self.saved.append(query["path"])
            if query["path"] == self.fail_save:
                raise URLError("lost save reply")
            return json.dumps({"ok": True, "size": len(data)}).encode()
        if endpoint in ("/api/profiles/start", "/api/reboot"):
            c64 = self.cases[0]
            ethernet = b"network_device=1" in self.files[
                f"/profiles/{c64['id']}/settings-c64.txt"]
            ethernet = ethernet and not self.wrong_network
            self.current.update(profile_id=c64["id"], machine="C64", uptime_secs=0,
                                net_status=3 if ethernet else 13,
                                net_text="connected (Ethernet)" if ethernet else "connected (Wi-Fi)")
            if self.lost_restart_reply:
                raise URLError("restart closed the socket")
            return b'{"ok":true}'
        raise AssertionError(endpoint)


class NetworkSettingsTests(unittest.TestCase):
    def test_edits_preserve_other_settings_comments_bom_and_line_endings(self):
        original = b"\xef\xbb\xbf# test\r\n  network_device = 2\r\nwebui_enabled=1\r\nother=9"
        self.assertEqual(
            network_settings(original, "ethernet"),
            original.replace(b"network_device = 2", b"network_device = 1"))
        self.assertEqual(network_settings(b"webui_enabled=1", "wifi"),
                         b"webui_enabled=1\nnetwork_device=2\n")
        with self.assertRaisesRegex(ValueError, "duplicate"):
            network_settings(b"network_device=2\nnetwork_device=1\n", "ethernet")

    def test_legacy_settings_fallback_only_on_not_found(self):
        rig = Rig()
        case = rig.cases[0]
        modern = f"/profiles/{case['id']}/settings-c64.txt"
        legacy = f"/profiles/{case['id']}/settings.txt"
        rig.files[legacy] = rig.files.pop(modern)
        with patch("sys.stdout", new_callable=io.StringIO):
            self.assertEqual(read_settings(rig, case), (legacy, rig.files[legacy]))
        api = Mock()
        api.request.side_effect = HTTPError("http://fixture", 401, "auth", {}, None)
        with self.assertRaises(HTTPError):
            read_settings(api, case)
        self.assertEqual(api.request.call_count, 1)

    def test_preflight_requires_selected_network_but_old_setups_remain_supported(self):
        rig = Rig()
        validate_network(rig, rig.cases, None)
        validate_network(rig, rig.cases, "wifi")
        with self.assertRaisesRegex(TestFailure, "expected ethernet"):
            validate_network(rig, rig.cases, "ethernet")
        with patch("sys.stdout", new_callable=io.StringIO):
            apply_network(rig, rig.cases, "ethernet", lambda: None)
        validate_network(rig, rig.cases, "ethernet")
        self.assertEqual(len(rig.saved), 6)

    def test_no_save_if_any_profile_is_invalid(self):
        for bad_data in (b"network_device=2\nwebui_enabled=0\n",
                         b"network_device=2\nnetwork_device=1\nwebui_enabled=1\n"):
            rig = Rig()
            rig.files[f"/profiles/{rig.cases[-1]['id']}/settings-pet.txt"] = bad_data
            with self.assertRaises((ValueError, RuntimeError)):
                apply_network(rig, rig.cases, "ethernet", lambda: None)
            self.assertEqual(rig.saved, [])

    def test_partial_save_failure_reports_verified_and_unprocessed_profiles(self):
        rig = Rig()
        rig.fail_save = f"/profiles/{rig.cases[1]['id']}/settings-c128.txt"
        with patch("sys.stdout", new_callable=io.StringIO), \
                self.assertRaisesRegex(RuntimeError, "Pi was not restarted") as failure:
            apply_network(rig, rig.cases, "ethernet", lambda: None)
        self.assertIn(rig.cases[0]["id"], str(failure.exception))
        self.assertIn(rig.cases[2]["id"], str(failure.exception))
        self.assertEqual(len(rig.saved), 2)
        self.assertFalse(any(endpoint == "/api/profiles/start"
                             for endpoint, _, _ in rig.requests))

    def test_already_correct_files_are_verified_without_creating_new_backups(self):
        rig = Rig()
        with patch("sys.stdout", new_callable=io.StringIO):
            apply_network(rig, rig.cases, "wifi", lambda: None)
        self.assertEqual(rig.saved, [])
        self.assertEqual(sum(endpoint == "/api/fs/download"
                             for endpoint, _, _ in rig.requests), 12)

    def test_boot_preflight_rejects_network_mismatch_before_opening_uart_or_capture(self):
        from tests.boot.session import execute
        rig = Rig()
        with tempfile.TemporaryDirectory() as directory, \
                patch("tests.boot.session.Api", return_value=rig), \
                patch("tests.boot.session.SerialPort", side_effect=AssertionError("UART opened")), \
                patch("tests.boot.session.SuiteCapture",
                      side_effect=AssertionError("capture opened")), \
                patch("sys.stdout", new_callable=io.StringIO), ExitStack() as stack:
            args = parser().parse_args([
                "--execute", "--url", "http://192.0.2.1", "--pico-uart", "/unused",
                "--network", "ethernet"])
            with self.assertRaisesRegex(TestFailure, "expected ethernet"):
                execute(args, Path(directory), {"manifest": rig.manifest}, stack)


class SetupWorkflowTests(unittest.TestCase):
    def run_setup(self, rig, directory, *, network="ethernet", saved=None, cancel=False):
        if saved is not None:
            (directory / "test_setup.json").write_text(json.dumps(saved))
        answers = ["/dev/video4", "192.0.2.1", "y", network, "/dev/PICO", "-",
                   "", "fixture", "5", "n" if cancel else "y", "192.0.2.9", ""]
        process = Mock()
        process.poll.return_value = None
        manager = Mock()
        manager.__enter__ = Mock(return_value=process)
        manager.__exit__ = Mock(return_value=False)
        with patch("utils.configure_test_setup.preview", return_value=manager), \
                patch("utils.configure_test_setup.discover_devices", return_value=[]), \
                patch("utils.configure_test_setup.Api", return_value=rig) as api, \
                patch("utils.configure_test_setup.getpass.getpass", return_value="fixture-pin"), \
                patch("builtins.input", side_effect=answers), \
                patch("sys.stdout", new_callable=io.StringIO), \
                patch("sys.stderr", new_callable=io.StringIO):
            try:
                result = configure(directory)
            finally:
                manager.__exit__.assert_called_once()
        return result, api

    def test_preview_to_ethernet_and_new_ip_is_saved_only_after_c64_verification(self):
        rig = Rig()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            result, api = self.run_setup(rig, root)
            self.assertTrue(result)
            settings = load_setup(root / "test_setup.json")
            self.assertEqual(settings["network"], "ethernet")
            self.assertEqual(settings["url"], "http://192.0.2.9/")
            self.assertNotIn("fixture-pin", (root / "test_setup.json").read_text())
            self.assertEqual((root / "test_setup.json").stat().st_mode & 0o777, 0o600)
            self.assertEqual([call.args[0] for call in api.call_args_list],
                             ["http://192.0.2.1/", "http://192.0.2.9/"])
        self.assertEqual(len(rig.saved), 6)
        self.assertTrue(all(b"volume=75" in rig.files[path] for path in rig.saved))
        starts = [query for endpoint, _, query in rig.requests
                  if endpoint == "/api/profiles/start"]
        self.assertEqual(starts, [{"id": rig.cases[0]["id"], "once": "1"}])

    def test_wifi_choice_and_active_c64_reboot(self):
        rig = Rig()
        rig.current["profile_id"] = rig.cases[0]["id"]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.assertTrue(self.run_setup(rig, root, network="wifi")[0])
            self.assertEqual(load_setup(root / "test_setup.json")["network"], "wifi")
        self.assertEqual(rig.saved, [])
        self.assertEqual(sum(endpoint == "/api/reboot" for endpoint, _, _ in rig.requests), 1)

    def test_failure_never_reboots_or_replaces_existing_local_setup(self):
        for failure in ("save", "readback"):
            rig = Rig()
            path = f"/profiles/{rig.cases[1]['id']}/settings-c128.txt"
            if failure == "save":
                rig.fail_save = path
            else:
                rig.corrupt_readback = path
            with self.subTest(failure=failure), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                old = {"url": "http://192.0.2.1/", "pico_uart": "/dev/PICO"}
                with self.assertRaises(RuntimeError):
                    self.run_setup(rig, root, saved=old)
                self.assertEqual(load_setup(root / "test_setup.json"), old)
                self.assertFalse((root / "run_tests").exists())
            self.assertFalse(any(endpoint in ("/api/reboot", "/api/profiles/start")
                                 for endpoint, _, _ in rig.requests))

    def test_wrong_interface_after_restart_does_not_save_setup(self):
        rig = Rig()
        rig.wrong_network = True
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaisesRegex(RuntimeError, "not the selected ethernet"):
                self.run_setup(rig, root)
            self.assertFalse((root / "test_setup.json").exists())

    def test_cancel_before_updates_leaves_remote_and_local_files_unchanged(self):
        rig = Rig()
        original = dict(rig.files)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.assertFalse(self.run_setup(rig, root, cancel=True)[0])
            self.assertFalse((root / "test_setup.json").exists())
        self.assertEqual(rig.files, original)
        self.assertEqual(rig.saved, [])

    def test_lost_restart_reply_is_not_replayed(self):
        rig = Rig()
        rig.lost_restart_reply = True
        with tempfile.TemporaryDirectory() as directory:
            self.assertTrue(self.run_setup(rig, Path(directory))[0])
        self.assertEqual(sum(endpoint == "/api/profiles/start"
                             for endpoint, _, _ in rig.requests), 1)

    def test_initial_authentication_failure_closes_preview_without_writing(self):
        rig = Rig()
        rig.get_json = Mock(side_effect=HTTPError("http://fixture", 401, "PIN required", {}, None))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaises(HTTPError):
                self.run_setup(rig, root)
            self.assertFalse((root / "test_setup.json").exists())
        self.assertEqual(rig.saved, [])

    def test_interrupt_during_save_closes_preview_and_does_not_restart(self):
        rig = Rig()
        request = rig.request

        def interrupted(endpoint, data=None, **query):
            result = request(endpoint, data, **query)
            if endpoint == "/api/fs/save":
                raise KeyboardInterrupt
            return result

        rig.request = interrupted
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaises(KeyboardInterrupt):
                self.run_setup(rig, root)
            self.assertFalse((root / "test_setup.json").exists())
        self.assertEqual(len(rig.saved), 1)
        self.assertFalse(any(endpoint in ("/api/reboot", "/api/profiles/start")
                             for endpoint, _, _ in rig.requests))

    def test_unchanged_status_times_out_rather_than_accepting_a_stale_c64_boot(self):
        rig = Rig()
        rig.current["profile_id"] = rig.cases[0]["id"]
        rig.current["uptime_secs"] = 1000
        before = dict(rig.current)
        clock = [0]

        def advance(_):
            clock[0] += 1

        with patch("utils.configure_test_setup.time.monotonic", side_effect=lambda: clock[0]), \
                patch("utils.configure_test_setup.time.sleep", side_effect=advance), \
                patch("sys.stdout", new_callable=io.StringIO), \
                self.assertRaisesRegex(RuntimeError, "could not be verified"):
            verify_restart(rig, rig.cases[0], before, 0, "wifi", 1, lambda: None)


class PreviewLifetimeTests(unittest.TestCase):
    def test_preview_closes_even_if_setup_is_interrupted(self):
        process = Mock()
        process.poll.return_value = None
        with patch("utils.preview_video.subprocess.Popen", return_value=process), \
                patch("utils.preview_video.shutil.which", return_value="/usr/bin/ffplay"), \
                patch("utils.preview_video.time.sleep"), \
                patch("sys.stdout", new_callable=io.StringIO):
            with self.assertRaises(KeyboardInterrupt):
                with preview("/dev/video4"):
                    raise KeyboardInterrupt
        process.terminate.assert_called_once()
        process.wait.assert_called_once_with(timeout=5)

    def test_preview_startup_failure_is_explicit(self):
        process = Mock()
        process.poll.return_value = 1
        with patch("utils.preview_video.subprocess.Popen", return_value=process), \
                patch("utils.preview_video.shutil.which", return_value="/usr/bin/ffplay"), \
                patch("utils.preview_video.time.sleep"), \
                patch("sys.stdout", new_callable=io.StringIO), \
                self.assertRaisesRegex(RuntimeError, "preview exited"):
            with preview("/dev/video4"):
                self.fail("setup continued without preview")
        process.wait.assert_called_once()

    def test_unresponsive_preview_is_killed_and_reaped(self):
        process = Mock()
        process.poll.return_value = None
        process.wait.side_effect = [subprocess.TimeoutExpired("ffplay", 5), 0]
        with patch("utils.preview_video.subprocess.Popen", return_value=process), \
                patch("utils.preview_video.shutil.which", return_value="/usr/bin/ffplay"), \
                patch("utils.preview_video.time.sleep"), \
                patch("sys.stdout", new_callable=io.StringIO):
            with preview("/dev/video4"):
                pass
        process.kill.assert_called_once()
        self.assertEqual(process.wait.call_count, 2)
