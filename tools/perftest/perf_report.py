#!/usr/bin/env python3
"""Summarise, validate and compare BMC64 `[perf]` lines.

BMC64 built with --perf-stats prints one JSON line per 10 second window to
/bmc64.log and the serial console:

    [perf] {"v":1,"core":"vice-3.3","win":3,"frames":500,...}

This tool reads those lines from a log file, stdin ("-") or a serial device,
and reports how much of each frame's time budget the emulator used, whether any
vertical blank was missed, and how the audio buffer behaved. A log that holds
more than one boot is split per boot (the last usable boot is analysed unless
--boot says otherwise), and the network state is read from the same log, since
a network that keeps retrying can add several ms to a frame. See
docs/architecture/PERFORMANCE_TEST_PLAN.md.

Exit status: 0 pass, 1 fail or regression, 2 no data / bad usage,
             3 run is invalid (the Pi was throttled or under-volted).

Examples:
    perf_report.py bmc64.log
    perf_report.py bmc64.log --json result.json --label "pi3 c64 stress"
    perf_report.py bmc64.log --baseline pi3_c64_stress_vice-3.3.json
    stty -F /dev/ttyUSB0 115200 raw -echo
    perf_report.py /dev/ttyUSB0 --skip 1 --windows 6 --timeout 120
"""

import argparse
import json
import re
import signal
import statistics
import sys

LINE_RE = re.compile(r"\[perf\]\s*(\{.*\})\s*$")

# A line Circle prints once at the start of every boot.
BOOT_RE = re.compile(r"Circle \d+ started on")
# Log lines that say something about the network.
NET_RE = re.compile(r"(\bwpa:|\bdhcp:|\bntp:|\bwlan:|smsc951x:|lan7800:|Wi-Fi)")
# The subset that counts as network *activity* during a measurement.
NET_EVENT_RE = re.compile(r"(\bwpa:|\bdhcp:|\bntp:|\bwlan:)")

# get_throttled bits that make a run non-comparable: under-voltage, ARM clock
# capped, throttled, soft temperature limit - now (0-3) or since boot (16-19).
THROTTLE_MASK = 0x000F000F


class Timeout(Exception):
    pass


def _on_alarm(signum, frame):
    raise Timeout()


class Boot:
    """The reports and network log lines seen between two boots."""

    def __init__(self):
        self.windows = []
        self.net = []  # (number of reports seen so far in this boot, line)

    def empty(self):
        return not self.windows and not self.net


def read_boots(path, want, timeout):
    """Return (boots, bad_line_count). Stops after `want` windows in the
    current boot, or on timeout. A new boot starts at a Circle start-up line,
    or when the window number stops increasing (a capture that began late)."""
    boots = [Boot()]
    bad = 0
    f = sys.stdin if path == "-" else open(path, "r", errors="replace")
    if timeout:
        signal.signal(signal.SIGALRM, _on_alarm)
        signal.alarm(timeout)
    try:
        for line in f:
            if BOOT_RE.search(line):
                if not boots[-1].empty():
                    boots.append(Boot())
                continue
            m = LINE_RE.search(line)
            if not m:
                if NET_RE.search(line):
                    b = boots[-1]
                    b.net.append((len(b.windows), line.strip()))
                continue
            try:
                w = json.loads(m.group(1))
            except ValueError:
                bad += 1  # truncated by serial noise or a full log queue
                continue
            if w.get("v") != 1:
                bad += 1
                continue
            b = boots[-1]
            if b.windows and w["win"] <= b.windows[-1]["win"]:
                # No start-up line was seen, so this boot's own network lines
                # were logged before we knew it had started: they are the ones
                # that arrived after the previous boot's last report.
                moved = [line for after, line in b.net if after == len(b.windows)]
                b.net = [x for x in b.net if x[0] != len(b.windows)]
                b = Boot()
                b.net = [(0, line) for line in moved]
                boots.append(b)
            b.windows.append(w)
            if want and len(b.windows) >= want:
                break
    except Timeout:
        pass
    finally:
        if timeout:
            signal.alarm(0)
        if f is not sys.stdin:
            f.close()
    return boots, bad


def choose_boot(boots, skip, which):
    """Pick the boot to analyse. Returns (boot or None, notes)."""
    notes = []
    with_w = [b for b in boots if b.windows]
    if not with_w:
        return None, notes
    if which:
        if which < 1 or which > len(with_w):
            notes.append("no boot %d: the log has %d boot(s) with reports" % (which, len(with_w)))
            return None, notes
        chosen = with_w[which - 1]
        how = "chosen with --boot"
    else:
        usable = [b for b in with_w if len(b.windows) > skip]
        chosen = usable[-1] if usable else max(with_w, key=lambda b: len(b.windows))
        how = "the last with more than %d window(s)" % skip if usable else "the longest"
    if len(boots) > 1:
        notes.append("NOTE: the log contains %d boots, %d with reports (%s windows); analysing boot %d, %s. "
                     "Use --boot N to pick another."
                     % (len(boots), len(with_w), ", ".join(str(len(b.windows)) for b in with_w),
                        with_w.index(chosen) + 1, how))
    return chosen, notes


def _log_seconds(line):
    """Seconds from a leading HH:MM:SS.cc log timestamp, or None."""
    m = re.match(r"(\d+):(\d+):(\d+)\.(\d+)\s", line)
    if not m:
        return None
    h, mi, sec, cs = m.groups()
    return int(h) * 3600 + int(mi) * 60 + int(sec) + int(cs) / 100.0


def _redact(text):
    """Strip MAC addresses, IPv4 addresses and SSIDs from a log line, so a saved
    summary can be shared or committed."""
    text = re.sub(r"\b(?:[0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}\b", "<mac>", text)
    text = re.sub(r"\b\d{1,3}(?:\.\d{1,3}){3}\b", "<ip>", text)
    return re.sub(r"SSID='[^']*'", "SSID='<ssid>'", text)


def analyse_network(boot, skip):
    """Describe the network state from a boot's log lines, and count network
    activity that fell inside the measured windows (after the first `skip`).
    The SSID and IP address are deliberately not recorded."""
    text = "\n".join(line for _, line in boot.net)
    no_wlan = "no onboard WLAN" in text
    wifi_up = "Wi-Fi initialized" in text
    tried = bool(re.search(r"wpa: Trying to associate", text))
    joined = "CTRL-EVENT-CONNECTED" in text
    fails = len(re.findall(r"dhcp: (No response|Did not receive OFFER)", text))
    ip_at = None
    for _, line in boot.net:
        if re.search(r"dhcp: IP address is", line):
            ip_at = _log_seconds(line)
            if ip_at is None:
                ip_at = -1
            break
    ntp = "ok" if "System time set from NTP" in text else (
        "failed" if "Cannot get time from NTP" in text else None)

    if ip_at is not None:
        cls = "connected"
        link = "Ethernet" if no_wlan else "Wi-Fi"
        desc = "%s connected%s" % (link, ", IP after %d s" % round(ip_at) if ip_at >= 0 else "")
    elif joined:
        cls = "no-lease"
        desc = "Wi-Fi associated, NO DHCP lease (%d DHCP failure message(s))" % fails
    elif no_wlan:
        cls = "ethernet-no-ip"
        desc = "no onboard Wi-Fi; Ethernet has no IP (probably no cable)"
    elif tried:
        cls = "wifi-not-joined"
        desc = "Wi-Fi tried to associate but never connected"
    elif wifi_up:
        cls = "wifi-idle"
        desc = "Wi-Fi hardware up, never tried to associate (no network configured?)"
    else:
        cls = "none"
        desc = "no network activity in the log"
    if ntp:
        desc += "; NTP %s" % ntp

    events = [line for after, line in boot.net if after >= skip and NET_EVENT_RE.search(line)]
    return {"state": cls, "text": desc, "dhcp_failures": fails,
            "events_in_windows": len(events), "first_events": [_redact(e)[:100] for e in events[:3]]}


def summarise(windows, skip, args):
    """Reduce kept windows to one dict of headline numbers."""
    notes = []
    kept = windows[skip:]
    if not kept:
        return None, notes

    ident = {(w["core"], w["bmc64"], w["pi"], w["nominal_us"]) for w in kept}
    if len(ident) > 1:
        notes.append("WARNING: windows come from more than one core/build/board/"
                     "timing (%s); using the first" % sorted(ident))
        first = (kept[0]["core"], kept[0]["bmc64"], kept[0]["pi"], kept[0]["nominal_us"])
        kept = [w for w in kept
                if (w["core"], w["bmc64"], w["pi"], w["nominal_us"]) == first]

    # Windows during which emulation was paused (menu open) or that are
    # otherwise partial say nothing about the workload.
    if len(kept) > 2:
        med = statistics.median(w["frames"] for w in kept)
        good = [w for w in kept if w["gaps"] == 0 and w["frames"] >= 0.8 * med]
        if len(good) != len(kept):
            notes.append("excluded %d partial/paused window(s)" % (len(kept) - len(good)))
        kept = good or kept

    frames = sum(w["frames"] for w in kept)
    if frames == 0:
        return None, notes
    nominal = kept[0]["nominal_us"]

    def wavg(key, stat="avg"):
        num = sum(w[key][stat] * w[key]["n"] for w in kept)
        den = sum(w[key]["n"] for w in kept)
        return num / den if den else 0.0

    def wavg_audio(key):
        num = sum(w["audio"][key]["avg"] * w["audio"][key]["n"] for w in kept)
        den = sum(w["audio"][key]["n"] for w in kept)
        return num / den if den else 0.0

    hist = [sum(w["busy_hist"]["n"][i] for w in kept) for i in range(8)]
    fill_mins = [w["audio"]["fill"]["min"] for w in kept if w["audio"]["fill"]["n"]]

    s = {
        "core": kept[0]["core"],
        "bmc64": kept[0]["bmc64"],
        "pi": kept[0]["pi"],
        "windows": len(kept),
        "frames": frames,
        "nominal_us": nominal,
        "busy_avg_us": round(wavg("busy")),
        "busy_max_us": max(w["busy"]["max"] for w in kept),
        "util_avg_pct": round(100.0 * wavg("busy") / nominal, 1),
        "util_max_pct": round(100.0 * max(w["busy"]["max"] for w in kept) / nominal, 1),
        "stage_avg_us": {k: round(wavg(k)) for k in
                         ("emu", "post", "ready", "swap", "tail")},
        "hist_pct_edges": kept[0]["busy_hist"]["edges_pct"],
        "hist": hist,
        "over_budget_frames": hist[7],
        "missed_vblank": sum(w["missed"] for w in kept),
        "period_max_us": max(w["period"]["max"] for w in kept),
        "audio_fill_min": min(fill_mins) if fill_mins else None,
        "audio_fill_avg": round(wavg_audio("fill")),
        "audio_fill_cap": kept[0]["audio"]["fill_cap"],
        "audio_fill_zero": sum(w["audio"]["fill_zero"] for w in kept),
        "audio_silent_packets": sum(w["audio"]["silent"] for w in kept),
        "audio_full_waits": sum(w["audio"]["full_waits"] for w in kept),
        "audio_write_avg_us": round(wavg_audio("write")),
        "audio_write_max_us": max(w["audio"]["write"]["max"] for w in kept),
        "sid_wait_avg_us": round(sum(w["sid"]["wait"]["avg"] * w["sid"]["wait"]["n"] for w in kept)
                                 / max(1, sum(w["sid"]["wait"]["n"] for w in kept))),
        "sid_wait_max_us": max(w["sid"]["wait"]["max"] for w in kept),
        "windows_dropped": kept[-1]["dropped"],
        "temp_max_c": round(max(w["env"]["temp_mc"] for w in kept) / 1000.0, 1),
        "throttled_or": 0,
        "arm_mhz": sorted({round(w["env"]["arm_hz"] / 1e6) for w in kept}),
    }
    for w in kept:
        s["throttled_or"] |= w["env"]["throttled"]

    # Windows whose slowest frame did not fit in a frame period, and whether
    # the previous report had just queried the firmware (env "fresh"), which
    # is the first thing to suspect if stalls line up with it.
    by_win = {w["win"]: w for w in windows}
    stalls = []
    for w in kept:
        wk = w.get("worst")
        if not wk or wk["busy"] < nominal:
            continue
        stages = {k: wk[k] for k in ("emu", "post", "ready", "tail")}
        top = max(stages, key=stages.get)
        prev = by_win.get(w["win"] - 1)
        stalls.append({
            "win": w["win"], "at_ms": wk["at_ms"], "busy_us": wk["busy"],
            "stage": top, "stage_us": stages[top],
            "after_env_query": bool(prev and prev["env"].get("fresh")),
        })
    s["slow_windows"] = len(stalls)
    s["slow_after_env_query"] = sum(1 for x in stalls if x["after_env_query"])
    s["slow_detail"] = stalls[:12]
    return s, notes


def verdict(s, args):
    """Absolute checks that need no baseline. Returns (status, findings)."""
    findings = []
    if not args.ignore_throttle and (s["throttled_or"] & THROTTLE_MASK):
        return 3, ["INVALID RUN: firmware throttle bits 0x%08x (under-voltage, "
                   "clock capped or throttled). Fix power/cooling and rerun."
                   % s["throttled_or"]]
    status = 0
    if s["missed_vblank"] and not args.allow_miss:
        status = 1
        findings.append("FAIL: %d missed vertical blank(s)" % s["missed_vblank"])
    if s["audio_fill_zero"]:
        status = 1
        findings.append("FAIL: audio buffer empty on %d frame(s)" % s["audio_fill_zero"])
    if s["audio_silent_packets"]:
        status = 1
        findings.append("FAIL: %d silent audio packet(s) (underrun)" % s["audio_silent_packets"])
    return status, findings


# (label, key, lower_is_worse). Compared as a relative change against the
# tolerance; counters that must not grow are compared exactly.
RELATIVE = [
    ("frame busy avg (us)", "busy_avg_us", False),
    ("frame busy max (us)", "busy_max_us", False),
    ("utilisation avg (%)", "util_avg_pct", False),
    ("SID2 wait avg (us)", "sid_wait_avg_us", False),
    ("audio write avg (us)", "audio_write_avg_us", False),
]
COUNTERS = [
    ("missed vblanks", "missed_vblank"),
    ("over-budget frames", "over_budget_frames"),
    ("audio buffer empty frames", "audio_fill_zero"),
    ("silent audio packets", "audio_silent_packets"),
]


def compare(s, base, tol):
    rows = []
    status = 0
    if base.get("core") != s["core"]:
        rows.append(("NOTE", "core", "%s -> %s (cross-core comparison)" % (base.get("core"), s["core"])))
    if base.get("nominal_us") != s["nominal_us"] or base.get("pi") != s["pi"]:
        rows.append(("WARN", "setup", "board or frame period differs from baseline; "
                     "comparison is not like-for-like"))
    bnet, cnet = (base.get("network") or {}), s.get("network") or {}
    if bnet and cnet and bnet.get("state") != cnet.get("state"):
        rows.append(("WARN", "network", "state differs from baseline (%s -> %s); frame-time tails may "
                     "differ for that reason alone" % (bnet.get("state"), cnet.get("state"))))
    for label, key, _ in RELATIVE:
        b, c = base.get(key), s[key]
        if not b:
            continue
        delta = 100.0 * (c - b) / b
        flag = "REGRESSION" if delta > tol else ("improved" if delta < -tol else "ok")
        if flag == "REGRESSION":
            status = 1
        rows.append((flag, label, "%s -> %s (%+.1f%%)" % (b, c, delta)))
    for label, key in COUNTERS:
        b, c = base.get(key, 0), s[key]
        flag = "REGRESSION" if c > b else "ok"
        if flag == "REGRESSION":
            status = 1
        rows.append((flag, label, "%s -> %s" % (b, c)))
    return status, rows


def print_summary(s, label):
    if label:
        print("== %s" % label)
    print("core %s | bmc64 %s | Pi model %s | %d window(s), %d frames | %s MHz | max %.1f C"
          % (s["core"], s["bmc64"], s["pi"], s["windows"], s["frames"],
             "/".join(map(str, s["arm_mhz"])), s["temp_max_c"]))
    print("frame period %d us   busy avg %d us (%.1f%%)   busy max %d us (%.1f%%)"
          % (s["nominal_us"], s["busy_avg_us"], s["util_avg_pct"],
             s["busy_max_us"], s["util_max_pct"]))
    st = s["stage_avg_us"]
    print("  per frame avg us   emulate %d | post/OSD %d | present %d | vblank wait %d | tail %d"
          % (st["emu"], st["post"], st["ready"], st["swap"], st["tail"]))
    edges = s["hist_pct_edges"]
    names = ["<%d%%" % edges[0]] + ["<%d%%" % e for e in edges[1:]] + [">=%d%%" % edges[-1]]
    print("  busy histogram     " + "  ".join("%s:%d" % (n, c) for n, c in zip(names, s["hist"])))
    print("missed vblank %d   period max %d us   over-budget frames %d"
          % (s["missed_vblank"], s["period_max_us"], s["over_budget_frames"]))
    print("audio fill min/avg %s/%s of %s samples   empty %d   silent pkts %d   write avg/max %d/%d us"
          % (s["audio_fill_min"], s["audio_fill_avg"], s["audio_fill_cap"],
             s["audio_fill_zero"], s["audio_silent_packets"],
             s["audio_write_avg_us"], s["audio_write_max_us"]))
    net = s.get("network")
    if net:
        print("network: %s" % net["text"])
        if net["events_in_windows"]:
            print("  %d network event(s) fell inside the measured windows (e.g. %s); a network that is "
                  "retrying can add ms to a frame, see the slow-frame lines"
                  % (net["events_in_windows"], net["first_events"][0]))
    if s.get("slow_windows"):
        d = s["slow_detail"]
        print("slow frames (did not fit a frame period) in %d of %d windows; %d followed a firmware env query"
              % (s["slow_windows"], s["windows"], s["slow_after_env_query"]))
        stages = sorted({x["stage"] for x in d})
        ats = [x["at_ms"] for x in d]
        print("  slow stage %s; began %d..%d ms into the window (a spread means not tied to the report)"
              % ("/".join(stages), min(ats), max(ats)))
        for x in d[:6]:
            print("    win %-3d +%5d ms  %6d us, %s %d us%s"
                  % (x["win"], x["at_ms"], x["busy_us"], x["stage"], x["stage_us"],
                     "  (after env query)" if x["after_env_query"] else ""))
    if s["sid_wait_max_us"]:
        print("SID2 helper core wait avg/max %d/%d us" % (s["sid_wait_avg_us"], s["sid_wait_max_us"]))
    if s["windows_dropped"]:
        print("note: %d window(s) not reported (log/serial backlog)" % s["windows_dropped"])


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog=__doc__.split("\n\n", 1)[1])
    ap.add_argument("source", help="log file, serial device, or - for stdin")
    ap.add_argument("--skip", type=int, default=1,
                    help="leading windows to ignore while the workload starts (default 1)")
    ap.add_argument("--windows", type=int, default=0,
                    help="windows to keep after --skip; stop reading as soon as a boot has collected that "
                         "many (0 = read all)")
    ap.add_argument("--boot", type=int, default=0,
                    help="which boot to analyse when the log holds several (1 = first with reports; "
                         "default: the last one with more than --skip windows)")
    ap.add_argument("--timeout", type=int, default=0,
                    help="stop reading after this many seconds (for serial devices)")
    ap.add_argument("--label", default="", help="free text shown in the summary")
    ap.add_argument("--json", metavar="FILE", help="write the summary as JSON (usable as a baseline)")
    ap.add_argument("--baseline", metavar="FILE", help="compare against a saved summary")
    ap.add_argument("--tolerance", type=float, default=5.0,
                    help="allowed relative increase vs baseline, percent (default 5)")
    ap.add_argument("--allow-miss", action="store_true",
                    help="do not fail on missed vblanks (for deliberately over-budget workloads)")
    ap.add_argument("--ignore-throttle", action="store_true",
                    help="do not invalidate the run on firmware throttle bits")
    args = ap.parse_args()

    want = (args.skip + args.windows) if args.windows else 0
    try:
        boots, bad = read_boots(args.source, want, args.timeout)
    except OSError as e:
        print("cannot read %s: %s" % (args.source, e), file=sys.stderr)
        return 2
    if bad:
        print("note: ignored %d malformed [perf] line(s)" % bad, file=sys.stderr)

    boot, boot_notes = choose_boot(boots, args.skip, args.boot)
    if boot is None:
        for n in boot_notes:
            print(n, file=sys.stderr)
        print("no usable [perf] windows found", file=sys.stderr)
        return 2
    windows = boot.windows

    s, notes = summarise(windows, args.skip, args)
    if s is None:
        print("no usable [perf] windows found (%d read, %d skipped)" % (len(windows), args.skip),
              file=sys.stderr)
        return 2
    s["network"] = analyse_network(boot, args.skip)
    notes = boot_notes + notes

    print_summary(s, args.label)
    for n in notes:
        print(n)

    status, findings = verdict(s, args)
    for f in findings:
        print(f)

    if args.baseline and status != 3:
        try:
            with open(args.baseline) as bf:
                base = json.load(bf)
        except (OSError, ValueError) as e:
            print("cannot read baseline: %s" % e, file=sys.stderr)
            return 2
        cstatus, rows = compare(s, base, args.tolerance)
        print("-- vs baseline %s (tolerance %.1f%%)" % (args.baseline, args.tolerance))
        for flag, name, detail in rows:
            print("  %-10s %-26s %s" % (flag, name, detail))
        status = max(status, cstatus)

    if args.json:
        with open(args.json, "w") as jf:
            json.dump(s, jf, indent=2, sort_keys=True)
            jf.write("\n")

    print("RESULT: %s" % {0: "PASS", 1: "FAIL", 3: "INVALID"}[status])
    return status


if __name__ == "__main__":
    sys.exit(main())
