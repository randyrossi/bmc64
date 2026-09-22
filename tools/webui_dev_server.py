#!/usr/bin/env python3
"""Local preview server for the BMC64 web UI.

Serves src/webui/assets/ exactly as the on-device server would (root
paths like /style.css and /js/app.js resolve), plus mock implementations of
the /api/* endpoints so the dashboard and file browser actually work
without a Raspberry Pi.

    python3 tools/webui_dev_server.py            # http://localhost:8000
    python3 tools/webui_dev_server.py --root ~/  # browse a real folder
    python3 tools/webui_dev_server.py --no-watch # disable live reload

With live reload on (default) the browser refreshes whenever a file in
src/webui/assets/ changes, so you can edit and watch side by side.

Before serving, the tests in tools/webui_test/ are run (needs Node.js 22.12+
on PATH; `source get_gnu_toolchain.sh` first if you don't have one). A
failure refuses to start, printing what failed. Pass --skip-tests to start
anyway; a missing Node.js only prints a warning and does not block starting.
"""

import argparse
import base64
import datetime as _dt
import json
import os
import posixpath
import shutil
import subprocess
import sys
import urllib.parse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASSET_DIR = os.path.join(REPO_ROOT, "src", "webui", "assets")
WEBUI_TEST_RUNNER = os.path.join(REPO_ROOT, "tools", "webui_test", "run_tests.mjs")


def run_webui_tests():
    """Runs tools/webui_test/ under Node.js. Returns True unless the tests
    ran and failed (a missing/too-old Node.js only warns and returns True,
    same as it not blocking a dev preview)."""
    node = shutil.which("node")
    if not node:
        print("  tests  : SKIPPED (no Node.js on PATH; source get_gnu_toolchain.sh)")
        return True
    print("  tests  : running tools/webui_test/ ...")
    result = subprocess.run([node, WEBUI_TEST_RUNNER], cwd=REPO_ROOT)
    if result.returncode == 0:
        return True
    if result.returncode == 2:
        print("  tests  : SKIPPED (%s)" % node)
        return True
    print("Web UI tests failed; not starting. Pass --skip-tests to start anyway.")
    return False

CONTENT_TYPES = {
    ".html": "text/html; charset=utf-8",
    ".js": "application/javascript; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".json": "application/json",
    ".png": "image/png",
    ".svg": "image/svg+xml",
    ".ico": "image/x-icon",
}

RELOAD_SNIPPET = """
<script>
(function () {
  var last = null;
  setInterval(function () {
    fetch("/__version", { cache: "no-store" })
      .then(function (r) { return r.text(); })
      .then(function (v) {
        if (last === null) { last = v; }
        else if (v !== last) { location.reload(); }
      })
      .catch(function () {});
  }, 700);
})();
</script>
"""

# Largest file the editor will open or save (matches webui_fs.cpp).
EDIT_MAX = 256 * 1024

ARGS = None


def assets_version():
    newest = 0.0
    for folder, _dirs, names in os.walk(ASSET_DIR):
        for name in names:
            newest = max(newest, os.path.getmtime(os.path.join(folder, name)))
    return "%.3f" % newest


def safe_join(root, rel):
    rel = urllib.parse.unquote(rel or "/")
    parts = [p for p in rel.split("/") if p not in ("", ".")]
    if any(p == ".." for p in parts):
        return None
    return os.path.join(root, *parts)


BAD_NAME_TEXT = ("invalid name: use plain ASCII without / \\ : * ? \" < > |, and no "
                 "leading space or trailing space or dot\n")


def valid_entry_name(name):
    """Same rule as IsValidEntryName in webui_fs.cpp."""
    if not name or len(name) > 200:
        return False
    if name[0] == " " or name[-1] in " .":
        return False
    return all(0x20 <= ord(c) < 0x7F and c not in '/\\:*?"<>|' for c in name)


def iso(ts):
    return _dt.datetime.fromtimestamp(ts).strftime("%Y-%m-%dT%H:%M:%S")


class Handler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def log_message(self, fmt, *a):
        sys.stderr.write("  %s\n" % (fmt % a))

    # -- helpers ----------------------------------------------------------

    def _send(self, code, body, ctype="text/plain; charset=utf-8", extra=None):
        if isinstance(body, str):
            body = body.encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        for k, v in (extra or {}).items():
            self.send_header(k, v)
        self.end_headers()
        if self.command != "HEAD":
            self.wfile.write(body)

    def _json(self, code, obj):
        self._send(code, json.dumps(obj), "application/json")

    # -- auth ---------------------------------------------------------

    def _check_auth(self):
        if not ARGS.pin:
            return True
        hdr = self.headers.get("Authorization", "")
        if hdr.startswith("Basic "):
            try:
                decoded = base64.b64decode(hdr[6:]).decode("utf-8", "replace")
                if decoded.split(":", 1)[-1] == ARGS.pin:
                    return True
            except Exception:
                pass
        try:
            self._drain_body()
        except Exception:
            pass
        self.send_response(401)
        self.send_header(
            "WWW-Authenticate",
            'Basic realm="BMC64 Web UI - any username, PIN as password"')
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.send_header("Content-Length", "0")
        self.end_headers()
        return False

    # -- routing --------------------------------------------------------

    def do_GET(self):
        if self._check_auth():
            self.route()

    def do_HEAD(self):
        if self._check_auth():
            self.route()

    def do_POST(self):
        if not self._check_auth():
            return
        parsed = urllib.parse.urlparse(self.path)
        path = parsed.path
        if path == "/api/reboot":
            self._drain_body()
            sys.stderr.write("  [mock] /api/reboot (no-op)\n")
            return self._json(202, {"ok": True})
        if path == "/api/reset":
            self._drain_body()
            sys.stderr.write("  [mock] /api/reset (no-op)\n")
            return self._json(202, {"ok": True})
        if path == "/api/webui/disable":
            self._drain_body()
            sys.stderr.write("  [mock] /api/webui/disable - stopping server\n")
            self._json(200, {"ok": True})
            import threading
            threading.Thread(target=self.server.shutdown, daemon=True).start()
            return
        if path == "/api/fs/upload":
            return self.api_fs_upload(parsed.query)
        if path == "/api/fs/save":
            return self.api_fs_save(parsed.query)
        if path == "/api/fs/delete":
            return self.api_fs_delete(parsed.query)
        if path == "/api/fs/mkdir":
            return self.api_fs_mkdir(parsed.query)
        if path == "/api/fs/rename":
            return self.api_fs_rename(parsed.query)
        if path == "/api/fs/autostart":
            return self.api_fs_autostart(parsed.query)
        return self._send(404, "not found\n")

    def _drain_body(self):
        length = int(self.headers.get("Content-Length") or 0)
        while length > 0:
            chunk = self.rfile.read(min(length, 65536))
            if not chunk:
                break
            length -= len(chunk)

    def route(self):
        parsed = urllib.parse.urlparse(self.path)
        path = parsed.path
        query = parsed.query

        if path == "/__version":
            return self._send(200, assets_version())
        if path == "/api/status":
            return self.api_status()
        if path == "/api/volumes":
            return self.api_volumes()
        if path == "/api/fs/list":
            return self.api_fs_list(query)
        if path == "/api/fs/download":
            return self.api_fs_download(query)
        if path.startswith("/api/"):
            return self._send(404, "unknown mock endpoint\n")
        return self.static(path)

    # -- static assets -------------------------------------------------

    def static(self, path):
        if path in ("/", "/index.html"):
            with open(os.path.join(ASSET_DIR, "index.html"), "rb") as fh:
                html = fh.read().decode("utf-8")
            if ARGS.watch:
                html = html.replace("</body>", RELOAD_SNIPPET + "</body>", 1)
            return self._send(200, html, CONTENT_TYPES[".html"])

        name = posixpath.normpath(path).lstrip("/")
        full = os.path.join(ASSET_DIR, name)
        if not full.startswith(ASSET_DIR) or not os.path.isfile(full):
            return self._send(404, "not found\n")
        ext = os.path.splitext(full)[1].lower()
        with open(full, "rb") as fh:
            self._send(200, fh.read(),
                       CONTENT_TYPES.get(ext, "application/octet-stream"))

    # -- mock API -----------------------------------------------------

    def api_status(self):
        self._json(200, {
            "hostname": "bmc64-dev",
            "version": "5.1.2",
            "machine": "C64",
            "model": "Raspberry Pi 3 Model B (dev mock)",
            "ip": "127.0.0.1",
            "net_status": 13,
            "net_text": "connected (Wi-Fi)",
            "uptime_secs": 8123,
            "soc_temp_c": 48.6,
            "throttled": int(ARGS.throttled, 0),
        })

    def api_volumes(self):
        usage = os.statvfs(ARGS.root)
        total_kb = usage.f_blocks * usage.f_frsize // 1024
        free_kb = usage.f_bavail * usage.f_frsize // 1024
        self._json(200, {"volumes": [{
            "id": "SD", "kind": "sdcard",
            "total_kb": total_kb, "free_kb": free_kb,
        }]})

    def api_fs_list(self, query):
        q = urllib.parse.parse_qs(query)
        rel = (q.get("path") or ["/"])[0]
        target = safe_join(ARGS.root, rel)
        if target is None or not os.path.isdir(target):
            return self._send(404, "no such directory\n")
        entries = []
        dir_parts = [p for p in rel.split("/") if p not in ("", ".")]
        for name in sorted(os.listdir(target)):
            p = os.path.join(target, name)
            try:
                st = os.stat(p)
            except OSError:
                continue
            is_dir = os.path.isdir(p)
            entry = {
                "name": name,
                "size": 0 if is_dir else st.st_size,
                "dir": is_dir,
                "mtime": iso(st.st_mtime),
            }
            if (not is_dir and st.st_size <= EDIT_MAX
                    and self.is_editable(dir_parts + [name])):
                entry["edit"] = True
            if self.is_protected(dir_parts + [name]):
                entry["protected"] = True
            entries.append(entry)
            if len(entries) >= 6000:
                break
        self._json(200, {
            "vol": "SD",
            "path": "/" + "/".join(p for p in rel.split("/") if p not in ("", ".")),
            "entries": entries,
            "truncated": False,
        })

    def api_fs_download(self, query):
        q = urllib.parse.parse_qs(query)
        rel = (q.get("path") or [""])[0]
        target = safe_join(ARGS.root, rel)
        if target is None or not os.path.isfile(target):
            return self._send(404, "cannot open file\n")
        with open(target, "rb") as fh:
            data = fh.read()
        self._send(200, data, "application/octet-stream", {
            "Content-Disposition": 'attachment; filename="%s"'
                                   % os.path.basename(target),
        })

    # Same lists as webui_fs.cpp: upload/delete refuse PROTECTED, and only the
    # editor's save endpoint may change EDITABLE files (root folder only) or
    # keyboard mapping files (*.vkm, any folder).
    CONFIG_FILES = {
        "settings.txt", "settings-c128.txt", "settings-vic20.txt",
        "settings-plus4.txt", "settings-plus4emu.txt", "settings-pet.txt",
        "wpa_supplicant.conf", "cmdline.txt", "config.txt", "machines.txt",
    }
    PROTECTED = CONFIG_FILES | {"bmc64.log"}
    EDITABLE = CONFIG_FILES | {"vice.ini"}

    @classmethod
    def is_protected(cls, parts):
        """parts: the path segments below --root, the name last. The entries
        the device will not rename or delete."""
        return bool(parts) and (parts[0].lower() == "firmware"
                                or parts[-1].lower() in cls.PROTECTED)

    @classmethod
    def is_editable(cls, parts):
        """parts: the path segments below --root, the file name last."""
        name = parts[-1].lower() if parts else ""
        if name.endswith(".vkm") and len(name) > 4:
            return True
        return len(parts) == 1 and name in cls.EDITABLE

    def api_fs_upload(self, query):
        length = self.headers.get("Content-Length")
        if length is None:
            self._drain_body()
            return self._send(411, "Content-Length header required\n")
        length = int(length)
        q = urllib.parse.parse_qs(query)
        rel = (q.get("path") or [""])[0]
        target = safe_join(ARGS.root, rel)
        if target is None or rel in ("", "/"):
            self._drain_body()
            return self._send(400, "bad path\n")
        parts = [p for p in rel.split("/") if p not in ("", ".")]
        if (parts and parts[0].lower() == "firmware") or \
           os.path.basename(target).lower() in self.PROTECTED:
            self._drain_body()
            return self._send(403, "that path is protected\n")
        if os.path.isdir(target):
            self._drain_body()
            return self._send(409, "target is a directory\n")
        overwrite = (q.get("overwrite") or ["0"])[0] == "1"
        if os.path.exists(target) and not overwrite:
            self._drain_body()
            return self._send(409, "file exists\n")

        os.makedirs(os.path.dirname(target), exist_ok=True)
        tmp = target + ".part"
        got = 0
        try:
            with open(tmp, "wb") as fh:
                while got < length:
                    chunk = self.rfile.read(min(length - got, 65536))
                    if not chunk:
                        break
                    fh.write(chunk)
                    got += len(chunk)
        except OSError as e:
            if os.path.exists(tmp):
                os.remove(tmp)
            return self._send(507, "write failed: %s\n" % e)
        if got != length:
            os.remove(tmp)
            return self._send(400, "upload truncated\n")
        os.replace(tmp, target)
        # Like the device: keep the file's original (local) modified time.
        mtime = (q.get("mtime") or [""])[0]
        try:
            ts = _dt.datetime.strptime(mtime, "%Y-%m-%dT%H:%M:%S").timestamp()
            os.utime(target, (ts, ts))
        except (ValueError, OverflowError, OSError):
            pass
        sys.stderr.write("  [mock] uploaded %s (%d bytes)\n" % (target, got))
        self._json(200, {"ok": True, "size": got})

    def api_fs_save(self, query):
        if not self.headers.get("X-BMC64-Web"):
            self._drain_body()
            return self._send(403, "missing X-BMC64-Web header\n")
        length = self.headers.get("Content-Length")
        if length is None:
            self._drain_body()
            return self._send(411, "Content-Length header required\n")
        length = int(length)
        q = urllib.parse.parse_qs(query)
        rel = (q.get("path") or [""])[0]
        target = safe_join(ARGS.root, rel)
        parts = [p for p in rel.split("/") if p not in ("", ".")]
        if target is None or not self.is_editable(parts):
            self._drain_body()
            return self._send(403, "that file cannot be edited\n")
        if length > EDIT_MAX:
            self._drain_body()
            return self._send(413, "file too large to edit\n")
        if os.path.isdir(target):
            self._drain_body()
            return self._send(409, "target is a directory\n")

        data = self.rfile.read(length)
        if len(data) != length:
            return self._send(400, "upload truncated\n")
        if b"\0" in data:
            return self._send(400, "not a text file\n")

        # Same .part / .bak dance as the device.
        tmp = target + ".part"
        with open(tmp, "wb") as fh:
            fh.write(data)
        if os.path.exists(target):
            os.replace(target, target + ".bak")
        os.replace(tmp, target)
        sys.stderr.write("  [mock] saved %s (%d bytes)\n" % (target, length))
        self._json(200, {"ok": True, "size": length})

    def api_fs_autostart(self, query):
        self._drain_body()
        q = urllib.parse.parse_qs(query)
        rel = (q.get("path") or [""])[0]
        target = safe_join(ARGS.root, rel)
        if target is None or rel in ("", "/"):
            return self._send(400, "bad path\n")
        ext = os.path.splitext(target)[1].lstrip(".").lower()
        if ext not in ("d64", "d71", "d81", "d82", "g64", "x64", "t64",
                       "tap", "prg", "p00", "crt"):
            return self._send(400, "not an autostartable file type\n")
        if not os.path.isfile(target):
            return self._send(404, "no such file\n")
        sys.stderr.write("  [mock] autostart %s (no-op)\n" % target)
        self._json(202, {"ok": True})

    def api_fs_delete(self, query):
        self._drain_body()
        q = urllib.parse.parse_qs(query)
        rel = (q.get("path") or [""])[0]
        target = safe_join(ARGS.root, rel)
        if target is None or rel in ("", "/"):
            return self._send(400, "bad path\n")
        parts = [p for p in rel.split("/") if p not in ("", ".")]
        if (parts and parts[0].lower() == "firmware") or \
           os.path.basename(target).lower() in self.PROTECTED:
            return self._send(403, "that path is protected\n")
        # Like the device: any "recursive" parameter needs the custom header,
        # and =1 deletes a folder together with its contents.
        if "recursive" in q and not self.headers.get("X-BMC64-Web"):
            return self._send(403, "missing X-BMC64-Web header\n")
        if (q.get("recursive") or [""])[0] == "1" and os.path.isdir(target):
            return self._delete_tree(target)
        try:
            if os.path.isdir(target):
                os.rmdir(target)
            elif os.path.exists(target):
                os.remove(target)
            else:
                return self._send(404, "no such file\n")
        except OSError as e:
            return self._send(409, "cannot delete: %s\n" % e)
        sys.stderr.write("  [mock] deleted %s\n" % target)
        self._json(200, {"ok": True})

    def _delete_tree(self, target):
        """Folder plus contents (the device also stops at 24 levels deep)."""
        removed = sum(len(d) + len(f) for _, d, f in os.walk(target)) + 1
        try:
            shutil.rmtree(target)
        except OSError as e:
            return self._send(409, "could not delete everything: %s\n" % e)
        sys.stderr.write("  [mock] deleted folder tree %s (%d items)\n" % (target, removed))
        self._json(200, {"ok": True})

    def api_fs_mkdir(self, query):
        self._drain_body()
        if not self.headers.get("X-BMC64-Web"):
            return self._send(403, "missing X-BMC64-Web header\n")
        q = urllib.parse.parse_qs(query)
        rel = (q.get("path") or [""])[0]
        target = safe_join(ARGS.root, rel)
        parts = [p for p in rel.split("/") if p not in ("", ".")]
        if target is None or not parts:
            return self._send(400, "bad path\n")
        if not valid_entry_name(parts[-1]):
            return self._send(400, BAD_NAME_TEXT)
        if self.is_protected(parts):
            return self._send(403, "that path is protected\n")
        if os.path.lexists(target):
            return self._send(409, "already exists\n")
        if not os.path.isdir(os.path.dirname(target)):
            return self._send(404, "parent folder not found\n")
        try:
            os.mkdir(target)
        except OSError as e:
            return self._send(500, "cannot create folder: %s\n" % e)
        sys.stderr.write("  [mock] created folder %s\n" % target)
        self._json(200, {"ok": True})

    def api_fs_rename(self, query):
        self._drain_body()
        if not self.headers.get("X-BMC64-Web"):
            return self._send(403, "missing X-BMC64-Web header\n")
        q = urllib.parse.parse_qs(query)
        rel = (q.get("path") or [""])[0]
        new_name = (q.get("to") or [""])[0]
        target = safe_join(ARGS.root, rel)
        parts = [p for p in rel.split("/") if p not in ("", ".")]
        if target is None or not parts:
            return self._send(400, "bad path\n")
        if not valid_entry_name(new_name):
            return self._send(400, BAD_NAME_TEXT)
        if self.is_protected(parts) or self.is_protected(parts[:-1] + [new_name]):
            return self._send(403, "that path is protected\n")
        if not os.path.lexists(target):
            return self._send(404, "no such file\n")
        new_target = os.path.join(os.path.dirname(target), new_name)
        if parts[-1] == new_name:
            return self._json(200, {"ok": True})
        # On the card a change of case only is the same name, not a clash.
        if os.path.lexists(new_target) and not os.path.samefile(target, new_target):
            return self._send(409, "already exists\n")
        try:
            os.rename(target, new_target)
        except OSError as e:
            return self._send(500, "rename failed: %s\n" % e)
        sys.stderr.write("  [mock] renamed %s -> %s\n" % (target, new_target))
        self._json(200, {"ok": True})


def main():
    global ARGS
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--port", type=int, default=8000)
    parser.add_argument("--root", default=REPO_ROOT,
                        help="folder the mock file browser serves (default: repo root)")
    parser.add_argument("--throttled", default="0x0",
                        help="mock /get_throttled bitmask, e.g. 0x50000 for past under-voltage")
    parser.add_argument("--pin", default="",
                        help="require this PIN via HTTP Basic Auth (like the device)")
    parser.add_argument("--no-watch", dest="watch", action="store_false",
                        help="disable browser live reload")
    parser.add_argument("--skip-tests", action="store_true",
                        help="start even if tools/webui_test/ fails or can't run")
    ARGS = parser.parse_args()
    ARGS.root = os.path.abspath(os.path.expanduser(ARGS.root))

    if not os.path.isdir(ASSET_DIR):
        sys.exit("asset directory not found: " + ASSET_DIR)

    print("BMC64 web UI dev server")
    if not ARGS.skip_tests and not run_webui_tests():
        sys.exit(1)

    server = ThreadingHTTPServer(("0.0.0.0", ARGS.port), Handler)
    print("  assets : %s" % ASSET_DIR)
    print("  browse : %s" % ARGS.root)
    print("  reload : %s" % ("on" if ARGS.watch else "off"))
    print("  pin    : %s" % (ARGS.pin if ARGS.pin else "(none)"))
    print("  open   : http://localhost:%d/" % ARGS.port)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
