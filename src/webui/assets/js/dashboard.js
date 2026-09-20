// Dashboard view and the Pi/system controls: status polling, hardware and
// storage meters, and the reboot / hard reset / disable-web-UI actions.

import { $, fmtUptime, fmtKB } from "./util.js";
import * as api from "./api.js";

// ---- status polling (polite: back off on failure, pause when hidden) ----

const POLL_OK_MS = 5000;
const POLL_FAIL_MS = 12000;
let pollTimer = null;
let rebooting = false;

function setConn(ok, text) {
  $("sys-dot").className = "dot " + (ok ? "ok" : "bad");
  $("sys-state").textContent = text || (ok ? "System Online" : "Offline");
}

function renderTemp(t) {
  const wrap = $("d-tempmeter");
  if (t == null || isNaN(Number(t))) { wrap.hidden = true; return; }
  t = Number(t);
  const cls = t >= 80 ? "v-bad" : t >= 70 ? "v-warn" : "v-ok";
  const val = $("d-temp");
  val.textContent = t.toFixed(1) + " °C";
  val.className = cls;
  const bar = $("d-temp-bar");
  bar.style.width = Math.max(0, Math.min(100, ((t - 30) / 60) * 100)) + "%";
  bar.style.background =
    t >= 80 ? "var(--red)" : t >= 70 ? "var(--amber)" : "var(--green)";
  wrap.hidden = false;
}

// Raspberry Pi get_throttled bits. Bits 0-3 are the current state; bits
// 16-19 latch "has happened since boot" and only clear on reboot, so
// each is shown on its own row rather than mixed into the live value.
const BIT_UNDERVOLT_NOW = 1 << 0;
const BIT_FREQ_CAP_NOW = 1 << 1;
const BIT_THROTTLED_NOW = 1 << 2;
const BIT_SOFT_TEMP_NOW = 1 << 3;
const BIT_UNDERVOLT_EVER = 1 << 16;
const BIT_FREQ_CAP_EVER = 1 << 17;
const BIT_THROTTLED_EVER = 1 << 18;
const BIT_SOFT_TEMP_EVER = 1 << 19;

// el gets "Yes (<detail>)" / "No"; bad rows use `cls`, "No" is always ok.
function yesNo(id, bad, cls, hex, detail) {
  const el = $(id);
  el.textContent = bad ? ("Yes" + (detail ? " — " + detail : "")) : "No";
  el.className = bad ? cls : "v-ok";
  el.title = "get_throttled: 0x" + hex;
}

function renderThermalFlags(raw) {
  const ids = ["d-uv", "d-uv-boot", "d-thr", "d-thr-boot"];
  if (raw == null) {
    ids.forEach((id) => { const e = $(id); e.textContent = "—"; e.className = ""; e.title = ""; });
    return;
  }
  const b = Number(raw) >>> 0;
  const hex = b.toString(16);

  yesNo("d-uv", b & BIT_UNDERVOLT_NOW, "v-bad", hex);
  yesNo("d-uv-boot", b & BIT_UNDERVOLT_EVER, "v-warn", hex);

  let detail = "";
  if (!(b & BIT_THROTTLED_NOW) && (b & BIT_FREQ_CAP_NOW)) detail = "turbo capped";
  else if (!(b & BIT_THROTTLED_NOW) && (b & BIT_SOFT_TEMP_NOW)) detail = "1.2 GHz, warm";
  yesNo("d-thr", b & (BIT_THROTTLED_NOW | BIT_FREQ_CAP_NOW | BIT_SOFT_TEMP_NOW),
        (b & BIT_THROTTLED_NOW) ? "v-bad" : "v-warn", hex, detail);

  yesNo("d-thr-boot",
        b & (BIT_THROTTLED_EVER | BIT_FREQ_CAP_EVER | BIT_SOFT_TEMP_EVER),
        "v-warn", hex);
}

function scheduleStatus(ms) {
  clearTimeout(pollTimer);
  pollTimer = null;
  if (document.hidden) return;
  pollTimer = setTimeout(pollStatus, ms);
}

async function pollStatus() {
  let ok = false;
  try {
    const s = await api.getStatus();
    ok = true;

    $("sc-uptime").textContent = fmtUptime(s.uptime_secs);
    $("sc-ip").textContent = s.ip || "—";
    $("sc-ver").textContent = s.version ? "v" + s.version : "—";

    renderTemp(s.soc_temp_c);
    renderThermalFlags(s.throttled);

    $("i-host").textContent = s.hostname || "—";
    $("i-ip").textContent = s.ip || "—";
    $("i-machine").textContent = s.machine || "—";
    $("i-model").textContent = s.model || "—";
    $("i-net").textContent = s.net_text || ("status " + s.net_status);
    $("i-ver").textContent = s.version ? "BMC64 v" + s.version : "—";
    $("i-uptime").textContent = fmtUptime(s.uptime_secs);

    if (rebooting) {
      rebooting = false;
      $("action-msg").className = "msg";
      $("action-msg").textContent = "BMC64 is back online.";
      $("qa-reboot").disabled = false;
      $("nav-reboot").disabled = false;
    }
  } catch (e) {
    if (e.status === 401) {
      setConn(false, "Locked");
      $("action-msg").className = "msg err";
      $("action-msg").textContent =
        "Authentication required. Reload the page and enter the Web UI PIN.";
      clearTimeout(pollTimer);
      pollTimer = null;
      return;
    }
    /* otherwise keep last known values */
  }
  setConn(ok);
  scheduleStatus(ok ? POLL_OK_MS : POLL_FAIL_MS);
}

export function startStatusPolling() {
  pollStatus();
}

export async function refreshVolumes() {
  try {
    const v = (await api.getVolumes()).volumes || [];
    const sd = v[0];
    if (!sd || !sd.total_kb) { $("d-storage").hidden = true; return; }
    const usedKb = sd.total_kb - sd.free_kb;
    const pct = Math.min(100, Math.round((usedKb / sd.total_kb) * 100));
    $("d-storage-bar").style.width = pct + "%";
    $("d-storage-txt").textContent =
      fmtKB(sd.free_kb) + " free of " + fmtKB(sd.total_kb);
    $("d-storage").hidden = false;
  } catch (e) {
    $("d-storage").hidden = true;
  }
}

// ---- power / web UI actions ----

// Send the reboot command without asking; the caller has already confirmed.
export async function rebootNow() {
  rebooting = true;
  $("qa-reboot").disabled = true;
  $("nav-reboot").disabled = true;
  $("action-msg").className = "msg";
  $("action-msg").textContent = "Sending reboot command…";
  try {
    await api.reboot();
  } catch (e) {
    /* the connection usually drops as it reboots; that is expected */
  }
  $("action-msg").textContent = "BMC64 is rebooting. This page will reconnect automatically.";
  scheduleStatus(POLL_FAIL_MS);
}

async function doReboot() {
  if (!confirm("Reboot BMC64 now? Any unsaved emulator state will be lost.")) return;
  await rebootNow();
}

async function doHardReset() {
  if (!confirm(
    "Hard reset the emulated machine now?\n\n")) return;
  $("qa-reset").disabled = true;
  $("action-msg").className = "msg";
  $("action-msg").textContent = "Sending hard reset…";
  try {
    await api.hardReset();
    $("action-msg").textContent = "Machine reset.";
  } catch (e) {
    $("action-msg").className = "msg err";
    $("action-msg").textContent = "Hard reset failed — " + e.message;
  }
  $("qa-reset").disabled = false;
}

async function disableWebUi() {
  if (!confirm(
    "Disable the Web UI now?\n\n" +
    "The server stops immediately. It starts again after the next reboot " +
    "unless you also turn it off in the Network menu.")) return;
  $("qa-disable").disabled = true;
  $("action-msg").className = "msg";
  $("action-msg").textContent = "Stopping the Web UI…";
  try {
    await api.disableWebUi();
  } catch (e) {
    $("action-msg").className = "msg err";
    $("action-msg").textContent = "Could not stop the Web UI — " + e.message;
    $("qa-disable").disabled = false;
    return;
  }
  clearTimeout(pollTimer);
  pollTimer = null;
  setConn(false, "Stopped");
  $("action-msg").textContent =
    "Web UI stopped. This page is now offline; it returns after the next " +
    "reboot unless disabled in the Network menu.";
}

export function initDashboard() {
  $("qa-reboot").addEventListener("click", doReboot);
  $("nav-reboot").addEventListener("click", doReboot);
  $("qa-reset").addEventListener("click", doHardReset);
  $("qa-disable").addEventListener("click", disableWebUi);

  document.addEventListener("visibilitychange", () => {
    if (document.hidden) { clearTimeout(pollTimer); pollTimer = null; }
    else pollStatus();
  });
}
