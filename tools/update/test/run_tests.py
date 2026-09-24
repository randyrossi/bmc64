#!/usr/bin/env python3
"""Host tests for the BMC64 updater (src/update/).

Builds the planner/apply/zip code with the PC's C compiler against a folder
that stands in for the SD card, then runs update scenarios end to end with
synthetic releases: plan, apply, backups, downgrade, repair, bad packages and
power cuts at every step of the commit.

  python3 tools/update/test/run_tests.py
  python3 tools/update/test/run_tests.py --cache DIR   # also a real-release
                                                      # test, using the zips
                                                      # gen_update_manifest.py
                                                      # seed downloaded
"""

import argparse
import hashlib
import os
import random
import shutil
import subprocess
import sys
import tempfile
import time
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
SRC = os.path.join(REPO, "src", "update")
GEN = os.path.join(REPO, "tools", "update", "gen_update_manifest.py")
ZLIB = os.path.join(REPO, "third_party", "zlib")
SOURCES = ["update_apply.c", "update_plan.c", "update_manifest.c",
           "update_zip.c", "update_hash.c"]
ZLIB_SOURCES = ["inflate.c", "inftrees.c", "inffast.c", "zutil.c", "crc32.c",
                "adler32.c"]

failures = []


def check(cond, what):
    if not cond:
        failures.append(what)
        print("  FAIL: " + what)


# ---- helpers ----

def build(work):
    exe = os.path.join(work, "update_cli")
    # HOST_CC, not CC: a build environment may point CC at the Arm compiler.
    cc = os.environ.get("HOST_CC", "cc")
    common = ["-g", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
              "-DZ_SOLO", "-I", ZLIB]
    # zlib is third-party code, built as the kernel builds it (no -Werror).
    objs = []
    for s in ZLIB_SOURCES:
        obj = os.path.join(work, "zlib_" + s.replace(".c", ".o"))
        subprocess.run([cc, "-c"] + common + ["-o", obj, os.path.join(ZLIB, s)], check=True)
        objs.append(obj)
    cmd = [cc, "-std=gnu11", "-Wall", "-Wextra", "-Werror"] + common + [
           "-I", SRC, "-o", exe,
           os.path.join(HERE, "update_cli.c"), os.path.join(HERE, "fs_posix.c")]
    cmd += [os.path.join(SRC, s) for s in SOURCES] + objs
    subprocess.run(cmd, check=True)
    return exe


def cli(exe, card, *args, crash_after=None, free_kb=None):
    env = dict(os.environ, UPDATE_TEST_ROOT=card)
    if crash_after is not None:
        env["UPDATE_TEST_CRASH_AFTER"] = str(crash_after)
    if free_kb is not None:
        env["UPDATE_TEST_FREE_KB"] = str(free_kb)
    r = subprocess.run([exe] + list(args), env=env, capture_output=True, text=True)
    if r.returncode not in (0, 1, 3):
        print(r.stdout, r.stderr)
        raise SystemExit("update_cli crashed (%d)" % r.returncode)
    return r.returncode, r.stdout


def parse_plan(out):
    plan = {"items": {}}
    for line in out.splitlines():
        w = line.split()
        if w[0] == "target":
            plan["target"], plan["direction"], plan["up_to_date"] = w[1], int(w[3]), int(w[5])
            plan["legacy"] = int(w[7])
        elif w[0] == "item":
            plan["items"][w[6]] = {"group": w[1], "status": w[2], "choice": w[3],
                                   "this_board": w[4] == "1"}
        elif w[0] == "apply":
            plan["apply"] = int(w[1])
        elif w[0] == "error":
            plan["error"] = line[6:]
    return plan


def tree(root):
    out = {}
    for folder, dirs, names in os.walk(root):
        for d in dirs:
            out[os.path.relpath(os.path.join(folder, d), root) + "/"] = "dir"
        for n in names:
            p = os.path.join(folder, n)
            with open(p, "rb") as f:
                out[os.path.relpath(p, root)] = hashlib.sha256(f.read()).hexdigest()
    return out


def read(card, path):
    with open(os.path.join(card, path), "rb") as f:
        return f.read()


def write(card, path, data):
    full = os.path.join(card, path)
    os.makedirs(os.path.dirname(full), exist_ok=True)
    with open(full, "wb") as f:
        f.write(data if isinstance(data, bytes) else data.encode())


# ---- synthetic releases ----

DIRS = ["disks/", "disks/C64/", "tapes/", "C64/", "firmware/"]


def blob(seed, size):
    # Half random, half repetitive text, so deflate uses all block types.
    rnd = random.Random(seed)
    text = ("BMC64 %d " % seed).encode() * (size // 20)
    noise = bytes(rnd.getrandbits(8) for _ in range(size // 2))
    return (noise + text)[:size]


def release_files(version):
    n = {"v1.0.0": 0, "v1.1.0": 1, "v1.2.0": 2}[version]
    files = {
        "config.txt": "disable_overscan=1\n",
        "cmdline.txt": "machine_timing=pal-hdmi\n",
        "machines.txt": "[C64/PAL/HDMI/720p]\n",
        "README.md": "BMC64 %s\n" % version,
        "C64/rpi_pos.vkm": "# keymap %d\n" % min(n, 1),
        "C64/rpi_sym.vkm": "# sym keymap\n",
        "firmware/wifi.bin": blob(900, 3000),
        "bootcode.bin": blob(800, 5000),
        "start.elf": blob(801 + min(n, 1), 60000),
        "fixup.dat": blob(802 + min(n, 1), 700),
    }
    for i, k in enumerate(["kernel.img", "kernel.img.c128", "kernel7.img",
                           "kernel8-32.img", "kernel8-32.img.c128"]):
        files[k] = blob(100 * n + i, 40000 + 997 * n + i)
    if n >= 1:
        files["C64/new.vkm"] = "# new keymap\n"
    return {k: v if isinstance(v, bytes) else v.encode() for k, v in files.items()}


def make_package(work, version, history, with_manifest=True, tamper=None):
    stage = os.path.join(work, "stage-" + version)
    shutil.rmtree(stage, ignore_errors=True)
    for path, data in release_files(version).items():
        write(stage, path, data)
    for d in DIRS:
        os.makedirs(os.path.join(stage, d), exist_ok=True)
    if with_manifest:
        subprocess.run([sys.executable, GEN, "release", "--stage", stage, "--version",
                        version, "--history", history, "--update-history"],
                       check=True, capture_output=True)
    if tamper:
        tamper(stage)
    zpath = os.path.join(work, "bmc64-%s.files.zip" % version)
    with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
        for folder, dirs, names in os.walk(stage):
            for d in dirs:
                rel = os.path.relpath(os.path.join(folder, d), stage) + "/"
                zf.writestr(rel, b"")
            for name in names:
                full = os.path.join(folder, name)
                rel = os.path.relpath(full, stage)
                # Some entries stored, like the real releases.
                zf.write(full, rel, compress_type=zipfile.ZIP_STORED
                         if name.endswith(".dat") else zipfile.ZIP_DEFLATED)
    return stage, zpath


def install_fresh(card, stage):
    shutil.rmtree(card, ignore_errors=True)
    shutil.copytree(stage, card)


def drop_zip(card, zpath):
    shutil.copy(zpath, os.path.join(card, "bmc64-update.zip"))


# ---- tests ----

def test_inflate(exe, work):
    print("inflate / zip reader")
    card = os.path.join(work, "inflate")
    os.makedirs(card, exist_ok=True)
    samples = {
        "empty": b"",
        "tiny": b"a",
        "text": b"The quick brown fox jumps over the lazy dog. " * 5000,
        "random": bytes(random.Random(1).getrandbits(8) for _ in range(200000)),
        "mixed": blob(7, 300000),
        "runs": b"\0" * 100000 + b"\1" * 70000,
    }
    zpath = os.path.join(card, "t.zip")
    with zipfile.ZipFile(zpath, "w") as zf:
        for name, data in samples.items():
            for level in (0, 1, 6, 9):
                info = zipfile.ZipInfo("%s-%d" % (name, level))
                info.compress_type = zipfile.ZIP_STORED if level == 0 else zipfile.ZIP_DEFLATED
                zf.writestr(info, data, compresslevel=level or None)
    for name, data in samples.items():
        for level in (0, 1, 6, 9):
            out = os.path.join(work, "out.bin")
            rc, _ = cli(exe, card, "extract", "/t.zip", "%s-%d" % (name, level), out)
            ok = rc == 0 and open(out, "rb").read() == data
            check(ok, "extract %s level %d" % (name, level))

    # A corrupted entry must be rejected (CRC).
    raw = bytearray(open(zpath, "rb").read())
    raw[len(raw) // 3] ^= 0xFF
    open(os.path.join(card, "bad.zip"), "wb").write(raw)
    bad = 0
    for name in samples:
        for level in (0, 9):
            rc, _ = cli(exe, card, "extract", "/bad.zip", "%s-%d" % (name, level),
                        os.path.join(work, "out.bin"))
            bad += rc != 0
    check(bad >= 1, "a corrupted zip entry is rejected")


def test_update(exe, work):
    print("update v1.0.0 -> v1.1.0")
    hist = os.path.join(work, "history.txt")
    stage0, zip0 = make_package(work, "v1.0.0", hist)
    stage1, zip1 = make_package(work, "v1.1.0", hist)

    card = os.path.join(work, "card")
    install_fresh(card, stage0)
    # The user's own files and edits.
    write(card, "settings.txt", "webui_enabled=1\n")
    write(card, "C64/kernal", blob(5, 8192))
    write(card, "config.txt", "disable_overscan=1\nkernel=kernel8-32.img.c128\n")
    write(card, "C64/rpi_sym.vkm", "# my own keymap\n")
    write(card, "backup/mystuff/keep.txt", "mine\n")
    drop_zip(card, zip1)
    before = tree(card)

    rc, out = cli(exe, card, "plan", "1.0.0", "kernel8-32.img")
    plan = parse_plan(out)
    items = plan["items"]
    check(plan.get("target") == "v1.1.0" and plan["direction"] == -1, "plan target/direction")
    check(all(items.get(k, {}).get("status") == "update" for k in
              ["kernel.img", "kernel7.img", "kernel8-32.img", "kernel8-32.img.c128"]),
          "kernels are updates")
    check(items["kernel8-32.img"]["this_board"] and not items["kernel7.img"]["this_board"],
          "this board's kernels")
    check(items["C64/rpi_pos.vkm"]["status"] == "update" and
          items["C64/rpi_pos.vkm"]["choice"] == "replace", "untouched keymap is replaced")
    check(items["C64/new.vkm"]["status"] == "new", "new file is added")
    check(items["config.txt"]["status"] == "changed" and
          items["config.txt"]["choice"] == "keep", "user's config.txt is kept by default")
    check(items["C64/rpi_sym.vkm"]["status"] == "changed", "user's keymap shows as changed")
    check("bootcode.bin" not in items and "machines.txt" not in items, "unchanged files not listed")
    check(tree(card) == before, "planning changes nothing")

    rc, out = cli(exe, card, "apply", "1.0.0", "kernel8-32.img")
    plan = parse_plan(out)
    check(plan.get("apply") == 0, "apply succeeds: " + out.splitlines()[-1])
    new = release_files("v1.1.0")
    old = release_files("v1.0.0")
    for path in ["kernel8-32.img", "kernel.img", "C64/rpi_pos.vkm", "C64/new.vkm",
                 "start.elf", "fixup.dat", "README.md"]:
        check(read(card, path) == new[path], "installed " + path)
    check(read(card, "config.txt").startswith(b"disable_overscan=1\nkernel="), "config.txt kept")
    check(read(card, "C64/rpi_sym.vkm") == b"# my own keymap\n", "user's keymap kept")
    check(read(card, "settings.txt") == b"webui_enabled=1\n", "settings.txt untouched")
    check(read(card, "backup/mystuff/keep.txt") == b"mine\n", "user's /backup folder untouched")
    check(read(card, "backup/kernel/v1.0.0/kernel8-32.img") == old["kernel8-32.img"],
          "old kernel in /backup/kernel/v1.0.0")
    check(read(card, "backup/v1.0.0/C64/rpi_pos.vkm") == old["C64/rpi_pos.vkm"],
          "old keymap in /backup/v1.0.0")
    check(read(card, "backup/v1.0.0/start.elf") == old["start.elf"], "old firmware backed up")
    check(os.path.exists(os.path.join(card, "backup/v1.0.0/.bmc64-backup")), "backup marker")
    check(b"target v1.0.0" in read(card, "backup/v1.0.0/bmc64-manifest.txt"),
          "old manifest backed up")
    check(b"target v1.1.0" in read(card, "bmc64-manifest.txt"), "manifest now v1.1.0")
    report = read(card, "backup/v1.0.0/update-report.txt").decode()
    check("Kept (not replaced)" in report and "config.txt" in report, "report lists kept files")
    check(os.path.exists(os.path.join(card, "backup/v1.0.0/RESTORE.txt")), "RESTORE.txt")
    check(not os.path.exists(os.path.join(card, "bmc64-update.zip")), "zip removed")
    check(not os.path.exists(os.path.join(card, "update_tmp")), "staging removed")
    check(os.path.isdir(os.path.join(card, "disks/C64")), "folders from the zip exist")
    reference = tree(card)

    print("power cut at every step of the commit")
    for n in range(0, 200):
        install_fresh(card, stage0)
        write(card, "settings.txt", "webui_enabled=1\n")
        write(card, "C64/kernal", blob(5, 8192))
        write(card, "config.txt", "disable_overscan=1\nkernel=kernel8-32.img.c128\n")
        write(card, "C64/rpi_sym.vkm", "# my own keymap\n")
        write(card, "backup/mystuff/keep.txt", "mine\n")
        drop_zip(card, zip1)
        rc, _ = cli(exe, card, "apply", "1.0.0", "kernel8-32.img", crash_after=n)
        if rc != 3:
            check(tree(card) == reference, "no crash at step %d" % n)
            print("  %d crash points checked" % n)
            break
        rc, out = cli(exe, card, "resume")
        check("resumed 1" in out, "resume after power cut %d" % n)
        check(tree(card) == reference, "same result after power cut at step %d" % n)

    print("unfinished journal is discarded")
    install_fresh(card, stage0)
    drop_zip(card, zip1)
    write(card, "update_tmp/journal.txt", "bmc64-update-journal 1\nmove /a /b\n")
    rc, out = cli(exe, card, "resume")
    check("resumed 0" in out and not os.path.exists(os.path.join(card, "update_tmp")) and
          os.path.exists(os.path.join(card, "bmc64-update.zip")), "unfinished journal discarded")

    return hist, stage0, zip0, stage1, zip1


def test_second_update_and_downgrade(exe, work, hist, stage0, zip0, stage1, zip1):
    print("second update keeps one backup; kernel backups are kept")
    stage2, zip2 = make_package(work, "v1.2.0", hist)
    card = os.path.join(work, "card2")
    install_fresh(card, stage0)
    write(card, "backup/v9.9.9/mine.txt", "not made by BMC64\n")
    drop_zip(card, zip1)
    cli(exe, card, "apply", "1.0.0", "kernel8-32.img")
    drop_zip(card, zip2)
    rc, out = cli(exe, card, "apply", "1.1.0", "kernel8-32.img")
    check(parse_plan(out).get("apply") == 0, "second update applies")
    check(not os.path.exists(os.path.join(card, "backup/v1.0.0")), "older backup removed")
    check(os.path.exists(os.path.join(card, "backup/v1.1.0/.bmc64-backup")), "new backup")
    check(os.path.exists(os.path.join(card, "backup/kernel/v1.0.0/kernel7.img")) and
          os.path.exists(os.path.join(card, "backup/kernel/v1.1.0/kernel7.img")),
          "kernel backups of every version kept")
    check(os.path.exists(os.path.join(card, "backup/v9.9.9/mine.txt")),
          "a /backup/v* folder without the marker is left alone")

    print("downgrade v1.2.0 -> v1.0.0 (old package, card manifest gives history)")
    drop_zip(card, zip0)
    rc, out = cli(exe, card, "plan", "1.2.0", "kernel8-32.img")
    plan = parse_plan(out)
    check(plan.get("direction") == 1, "downgrade detected")
    check(plan["items"].get("C64/rpi_pos.vkm", {}).get("status") == "update",
          "newer release's untouched file is an update, not 'changed'")
    rc, out = cli(exe, card, "apply", "1.2.0", "kernel8-32.img")
    check(parse_plan(out).get("apply") == 0, "downgrade applies")
    check(read(card, "kernel7.img") == release_files("v1.0.0")["kernel7.img"],
          "downgraded kernel installed")

    print("repair: same version with a missing and a damaged file")
    install_fresh(card, stage1)
    os.remove(os.path.join(card, "C64/rpi_pos.vkm"))
    write(card, "kernel7.img", b"damaged")
    drop_zip(card, zip1)
    rc, out = cli(exe, card, "plan", "1.1.0", "kernel8-32.img")
    plan = parse_plan(out)
    check(plan.get("direction") == 0 and set(plan["items"]) ==
          {"C64/rpi_pos.vkm", "kernel7.img"}, "repair lists only the broken files")
    rc, out = cli(exe, card, "apply", "1.1.0", "kernel8-32.img")
    check(parse_plan(out).get("apply") == 0 and
          read(card, "kernel7.img") == release_files("v1.1.0")["kernel7.img"], "repair applies")

    print("choosing Keep for a file leaves it alone")
    install_fresh(card, stage0)
    drop_zip(card, zip1)
    rc, out = cli(exe, card, "apply", "1.0.0", "kernel8-32.img", "keep", "kernel7.img",
                  "keep", "start.elf")
    check(read(card, "kernel7.img") == release_files("v1.0.0")["kernel7.img"],
          "kept kernel unchanged")
    check(read(card, "start.elf") == release_files("v1.0.0")["start.elf"], "kept firmware unchanged")


def without_manifest(zpath, out):
    """A copy of a release zip without its manifest, like releases made before
    the updater."""
    with zipfile.ZipFile(zpath) as src, zipfile.ZipFile(out, "w") as dst:
        for info in src.infolist():
            if info.filename != "bmc64-manifest.txt":
                dst.writestr(info, src.read(info))
    return out


def test_older_release(exe, work, stage0, zip0, stage1):
    print("older release without a manifest (from before the updater)")
    legacy = without_manifest(zip0, os.path.join(work, "legacy-v1.0.0.zip"))
    card = os.path.join(work, "card4")
    install_fresh(card, stage1)   # v1.1.0: its manifest's history knows v1.0.0
    drop_zip(card, legacy)
    rc, out = cli(exe, card, "plan", "1.1.0", "kernel8-32.img")
    plan = parse_plan(out)
    check(plan.get("target") == "v1.0.0" and plan.get("legacy") == 1 and
          plan.get("direction") == 1, "older release recognised from the card's manifest")
    check(plan["items"].get("kernel7.img", {}).get("status") == "update" and
          "C64/new.vkm" not in plan["items"], "older release is planned like a downgrade")
    rc, out = cli(exe, card, "apply", "1.1.0", "kernel8-32.img")
    check(parse_plan(out).get("apply") == 0, "older release applies")
    old = release_files("v1.0.0")
    check(read(card, "kernel7.img") == old["kernel7.img"] and
          read(card, "C64/rpi_pos.vkm") == old["C64/rpi_pos.vkm"], "older release installed")
    check(b"target v1.1.0" in read(card, "bmc64-manifest.txt"),
          "the card keeps its manifest (the older release has none)")
    report = read(card, "backup/v1.1.0/update-report.txt").decode()
    check("before the updater" in report, "report says the updater can't be used again")

    install_fresh(card, stage1)
    os.remove(os.path.join(card, "bmc64-manifest.txt"))
    drop_zip(card, legacy)
    rc, out = cli(exe, card, "plan", "1.1.0", "kernel8-32.img")
    check(rc == 1 and "card has no" in out, "without the card's manifest it is refused")

    tampered = os.path.join(work, "legacy-tampered.zip")
    with zipfile.ZipFile(legacy) as src, zipfile.ZipFile(tampered, "w") as dst:
        for info in src.infolist():
            data = src.read(info)
            if info.filename == "kernel8-32.img":
                data = bytes([data[0] ^ 1]) + data[1:]   # same size, other bytes
            dst.writestr(info, data)
    install_fresh(card, stage1)
    drop_zip(card, tampered)
    before = tree(card)
    rc, out = cli(exe, card, "apply", "1.1.0", "kernel8-32.img")
    check(rc == 1 and "damaged" in out and tree(card) == before,
          "a changed file in an older release stops the update, card unchanged")

    missing = os.path.join(work, "legacy-missing.zip")
    with zipfile.ZipFile(legacy) as src, zipfile.ZipFile(missing, "w") as dst:
        for info in src.infolist():
            if info.filename != "README.md":
                dst.writestr(info, src.read(info))
    install_fresh(card, stage1)
    drop_zip(card, missing)
    rc, out = cli(exe, card, "plan", "1.1.0", "kernel8-32.img")
    check(rc == 1 and "doesn't match" in out, "an incomplete older release is refused")


def test_bad_packages(exe, work, hist, stage0):
    print("bad packages")
    card = os.path.join(work, "card3")

    _, nomanifest = make_package(work, "v1.1.0", hist, with_manifest=False)
    install_fresh(card, stage0)
    drop_zip(card, nomanifest)
    rc, out = cli(exe, card, "plan", "1.0.0", "kernel8-32.img")
    check(rc == 1 and "bmc64-manifest.txt" in out,
          "a zip without a manifest that the card doesn't know is refused")

    _, missing = make_package(work, "v1.1.0", hist,
                              tamper=lambda s: os.remove(os.path.join(s, "kernel7.img")))
    install_fresh(card, stage0)
    drop_zip(card, missing)
    rc, out = cli(exe, card, "plan", "1.0.0", "kernel8-32.img")
    check(rc == 1 and "incomplete" in out, "zip missing a listed file is refused")

    def unsafe(stage):
        path = os.path.join(stage, "bmc64-manifest.txt")
        text = open(path).read().replace(" C64/new.vkm", " ../evil.txt")
        open(path, "w").write(text)
    _, evil = make_package(work, "v1.1.0", hist, tamper=unsafe)
    install_fresh(card, stage0)
    drop_zip(card, evil)
    rc, out = cli(exe, card, "plan", "1.0.0", "kernel8-32.img")
    check(rc == 1 and "unsafe" in out, "unsafe path is refused")

    _, good = make_package(work, "v1.1.0", hist)
    install_fresh(card, stage0)
    drop_zip(card, good)
    before = tree(card)
    rc, out = cli(exe, card, "apply", "1.0.0", "kernel8-32.img", free_kb=10)
    check(rc == 1 and "space" in out and tree(card) == before,
          "not enough space: refused, card unchanged")

    open(os.path.join(card, "bmc64-update.zip"), "wb").write(b"not a zip")
    rc, out = cli(exe, card, "plan", "1.0.0", "kernel8-32.img")
    check(rc == 1 and "not a zip" in out, "a non-zip is refused")


def test_real_release(exe, work, cache):
    old = os.path.join(cache, "bmc64-v5.1.9.files.zip")
    new = os.path.join(cache, "bmc64-v5.1.10.files.zip")
    if not (os.path.exists(old) and os.path.exists(new)):
        print("real release test skipped (need v5.1.9 and v5.1.10 zips in --cache)")
        return
    print("real release: v5.1.9 card, v5.1.10 package")
    stage = os.path.join(work, "real-stage")
    with zipfile.ZipFile(new) as zf:
        zf.extractall(stage)
    hist = os.path.join(REPO, "release", "manifest_history.txt")
    tmp_hist = os.path.join(work, "real-history.txt")
    shutil.copy(hist, tmp_hist)
    subprocess.run([sys.executable, GEN, "release", "--stage", stage, "--version",
                    "v5.1.10", "--history", tmp_hist], check=True, capture_output=True)
    package = os.path.join(work, "real-package.zip")
    with zipfile.ZipFile(new) as src, zipfile.ZipFile(package, "w") as dst:
        for info in src.infolist():
            dst.writestr(info, src.read(info))
        dst.write(os.path.join(stage, "bmc64-manifest.txt"), "bmc64-manifest.txt",
                  compress_type=zipfile.ZIP_DEFLATED)
    card = os.path.join(work, "real-card")
    shutil.rmtree(card, ignore_errors=True)
    with zipfile.ZipFile(old) as zf:
        zf.extractall(card)
    drop_zip(card, package)
    rc, out = cli(exe, card, "plan", "5.1.9", "kernel8-32.img")
    plan = parse_plan(out)
    check(set(plan["items"]) == {k for k in plan["items"] if k.startswith("kernel")} and
          len(plan["items"]) == 16, "v5.1.9 -> v5.1.10 changes exactly the 16 kernels")
    start = time.time()
    rc, out = cli(exe, card, "apply", "5.1.9", "kernel8-32.img")
    check(parse_plan(out).get("apply") == 0, "real update applies")
    with zipfile.ZipFile(new) as zf:
        same = all(read(card, i.filename) == zf.read(i) for i in zf.infolist()
                   if not i.is_dir())
    check(same, "card matches v5.1.10 after the update")
    print("  applied in %.1f s on this PC (with sanitizers)" % (time.time() - start))

    print("real release: official v5.1.9 zip (no manifest) as a downgrade")
    drop_zip(card, old)
    rc, out = cli(exe, card, "plan", "5.1.11", "kernel8-32.img")
    plan = parse_plan(out)
    check(plan.get("target") == "v5.1.9" and plan.get("legacy") == 1,
          "official v5.1.9 zip recognised from the card's manifest")
    rc, out = cli(exe, card, "apply", "5.1.11", "kernel8-32.img")
    check(parse_plan(out).get("apply") == 0, "official v5.1.9 zip applies")
    with zipfile.ZipFile(old) as zf:
        same = all(read(card, i.filename) == zf.read(i) for i in zf.infolist()
                   if not i.is_dir())
    check(same, "card matches v5.1.9 after the downgrade")


def find_node():
    node = shutil.which("node")
    if node:
        return node
    cache = os.path.join(os.environ.get("XDG_CACHE_HOME", os.path.expanduser("~/.cache")),
                         "bmc64", "toolchains")
    if os.path.isdir(cache):
        for d in sorted(os.listdir(cache)):
            candidate = os.path.join(cache, d, "bin", "node")
            if d.startswith("node-") and os.path.exists(candidate):
                return candidate
    return None


def test_web(work, zpath):
    print("web UI update modules (Node)")
    node = find_node()
    if not node:
        print("  skipped: no Node.js (source get_gnu_toolchain.sh)")
        return
    r = subprocess.run([node, os.path.join(HERE, "web.test.mjs"), zpath],
                       capture_output=True, text=True)
    for line in r.stdout.splitlines():
        print("  " + line.strip())
    check(r.returncode == 0, "web tests" + (": " + r.stderr[-500:] if r.stderr else ""))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cache", help="folder with official release zips")
    ap.add_argument("--keep", action="store_true", help="keep the work folder")
    opts = ap.parse_args()
    work = tempfile.mkdtemp(prefix="bmc64-update-test-")
    try:
        exe = build(work)
        test_inflate(exe, work)
        hist, stage0, zip0, stage1, zip1 = test_update(exe, work)
        test_second_update_and_downgrade(exe, work, hist, stage0, zip0, stage1, zip1)
        test_older_release(exe, work, stage0, zip0, stage1)
        test_bad_packages(exe, work, hist, stage0)
        test_web(work, zip1)
        if opts.cache:
            test_real_release(exe, work, opts.cache)
    finally:
        if opts.keep:
            print("work folder: " + work)
        else:
            shutil.rmtree(work, ignore_errors=True)
    if failures:
        print("\n%d FAILED" % len(failures))
        sys.exit(1)
    print("\nall updater tests passed")


if __name__ == "__main__":
    main()
