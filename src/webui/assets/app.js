"use strict";

const $ = (id) => document.getElementById(id);

// ---- status polling (polite: back off on failure, pause when hidden) ----

const POLL_OK_MS = 5000;
const POLL_FAIL_MS = 12000;
let pollTimer = null;
let rebooting = false;

// current file-browser location (set by loadDir)
let fbVol = "SD";
let fbPath = "/";
let fbNames = new Set(); // file names in the current folder (for overwrite checks)

function fmtUptime(secs) {
  secs = Math.floor(secs || 0);
  const d = Math.floor(secs / 86400);
  const h = Math.floor((secs % 86400) / 3600);
  const m = Math.floor((secs % 3600) / 60);
  const s = secs % 60;
  const parts = [];
  if (d) parts.push(d + "d");
  if (d || h) parts.push(h + "h");
  parts.push(m + "m");
  if (!d) parts.push(s + "s");
  return parts.join(" ");
}

function fmtKB(kb) {
  kb = Number(kb) || 0;
  if (kb < 1024) return kb + " KB";
  const mb = kb / 1024;
  if (mb < 1024) return mb.toFixed(mb < 10 ? 1 : 0) + " MB";
  const gb = mb / 1024;
  return gb.toFixed(gb < 10 ? 2 : 1) + " GB";
}

function fmtBytes(n) {
  return fmtKB(Math.ceil((Number(n) || 0) / 1024));
}

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
    const r = await fetch("/api/status", { cache: "no-store" });
    if (r.status === 401) {
      setConn(false, "Locked");
      $("action-msg").className = "msg err";
      $("action-msg").textContent =
        "Authentication required. Reload the page and enter the Web UI PIN.";
      clearTimeout(pollTimer);
      pollTimer = null;
      return;
    }
    if (!r.ok) throw new Error("HTTP " + r.status);
    const s = await r.json();
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
    /* keep last known values */
  }
  setConn(ok);
  scheduleStatus(ok ? POLL_OK_MS : POLL_FAIL_MS);
}

async function refreshVolumes() {
  try {
    const r = await fetch("/api/volumes", { cache: "no-store" });
    if (!r.ok) throw new Error();
    const v = (await r.json()).volumes || [];
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

// ---- reboot ----

async function doReboot() {
  if (!confirm("Reboot BMC64 now? Any unsaved emulator state will be lost.")) return;
  rebooting = true;
  $("qa-reboot").disabled = true;
  $("nav-reboot").disabled = true;
  $("action-msg").className = "msg";
  $("action-msg").textContent = "Sending reboot command…";
  try {
    const r = await fetch("/api/reboot", { method: "POST" });
    if (!r.ok) throw new Error("HTTP " + r.status);
  } catch (e) {
    /* the connection usually drops as it reboots; that is expected */
  }
  $("action-msg").textContent = "BMC64 is rebooting. This page will reconnect automatically.";
  scheduleStatus(POLL_FAIL_MS);
}

async function doHardReset() {
  if (!confirm(
    "Hard reset the emulated machine now?\n\n")) return;
  $("qa-reset").disabled = true;
  $("action-msg").className = "msg";
  $("action-msg").textContent = "Sending hard reset…";
  try {
    const r = await fetch("/api/reset", { method: "POST" });
    if (!r.ok) throw new Error("HTTP " + r.status);
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
    const r = await fetch("/api/webui/disable", { method: "POST" });
    if (!r.ok) throw new Error("HTTP " + r.status);
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

// ---- file browser ----

const FILE_ICONS = {
  d64: "▣", d71: "▣", d81: "▣", d82: "▣", g64: "▣", t64: "▣", tap: "▤",
  prg: "▶", crt: "▦", sid: "♪", txt: "≣", nfo: "≣", zip: "▤",
};

// Types the emulator can autostart (keep in sync with webui_fs.cpp).
const RUNNABLE = new Set(
  ["d64", "d71", "d81", "d82", "g64", "x64", "t64", "tap", "prg", "p00"]);

function isRunnable(name) {
  return RUNNABLE.has((name.split(".").pop() || "").toLowerCase());
}

function fileIcon(name, isDir) {
  if (isDir) return "▸";
  const ext = (name.split(".").pop() || "").toLowerCase();
  return FILE_ICONS[ext] || "•";
}

function normPath(p) {
  if (!p || p[0] !== "/") p = "/" + (p || "");
  const parts = p.split("/").filter((s) => s && s !== ".");
  return "/" + parts.join("/");
}

function parentPath(p) {
  p = normPath(p);
  if (p === "/") return "/";
  return normPath(p.slice(0, p.lastIndexOf("/")) || "/");
}

function filesHash(path) {
  path = normPath(path);
  if (path === "/") return "#/files";
  return "#/files" + path.split("/").map(encodeURIComponent).join("/");
}

function pathFromHash() {
  const raw = (location.hash || "").slice("#/files".length);
  if (!raw) return "/";
  try {
    return normPath(raw.split("/").map(decodeURIComponent).join("/"));
  } catch (e) {
    return "/";
  }
}

function renderCrumbs(vol, path) {
  const box = $("fb-crumbs");
  box.textContent = "";
  const root = document.createElement("a");
  root.href = filesHash("/");
  root.textContent = vol || "SD";
  box.appendChild(root);

  const parts = normPath(path).split("/").filter(Boolean);
  let acc = "";
  parts.forEach((seg, i) => {
    acc += "/" + seg;
    const sep = document.createElement("span");
    sep.className = "sep";
    sep.textContent = "/";
    box.appendChild(sep);
    if (i === parts.length - 1) {
      const cur = document.createElement("span");
      cur.className = "cur";
      cur.textContent = seg;
      box.appendChild(cur);
    } else {
      const a = document.createElement("a");
      a.href = filesHash(acc);
      a.textContent = seg;
      box.appendChild(a);
    }
  });
}

async function loadDir(path) {
  path = normPath(path);
  $("fb-up").disabled = path === "/";
  $("fb-status").className = "msg";
  $("fb-status").textContent = "Loading…";
  $("fb-rows").textContent = "";

  let data;
  try {
    const r = await fetch("/api/fs/list?path=" + encodeURIComponent(path), { cache: "no-store" });
    if (r.status === 401) throw new Error("authentication required — reload the page and enter the PIN");
    if (!r.ok) throw new Error("HTTP " + r.status + " " + (await r.text()).trim());
    data = await r.json();
  } catch (e) {
    $("fb-status").className = "msg err";
    $("fb-status").textContent = "Could not list " + path + " — " + e.message;
    return;
  }

  const vol = data.vol || "SD";
  fbVol = vol;
  fbPath = data.path || path;
  renderCrumbs(vol, fbPath);

  const entries = (data.entries || []).slice().sort((a, b) => {
    if (!!a.dir !== !!b.dir) return a.dir ? -1 : 1;
    return a.name.toLowerCase().localeCompare(b.name.toLowerCase());
  });
  fbNames = new Set(entries.filter((e) => !e.dir).map((e) => e.name));

  const tbody = $("fb-rows");
  const frag = document.createDocumentFragment();
  for (const e of entries) {
    const childPath = normPath(path + "/" + e.name);
    const tr = document.createElement("tr");

    const tdName = document.createElement("td");
    const wrap = document.createElement("span");
    wrap.className = "fname";
    const ic = document.createElement("span");
    ic.className = "ic " + (e.dir ? "dir" : "file");
    ic.textContent = fileIcon(e.name, e.dir);
    wrap.appendChild(ic);
    if (e.dir) {
      const a = document.createElement("a");
      a.href = filesHash(childPath);
      a.textContent = e.name;
      wrap.appendChild(a);
    } else if (isRunnable(e.name)) {
      const a = document.createElement("a");
      a.href = "#";
      a.title = "Run " + e.name;
      a.textContent = e.name;
      a.addEventListener("click", (ev) => {
        ev.preventDefault();
        runFile(childPath, e.name);
      });
      wrap.appendChild(a);
    } else {
      wrap.appendChild(document.createTextNode(e.name));
    }
    tdName.appendChild(wrap);

    const tdSize = document.createElement("td");
    tdSize.className = "col-size";
    tdSize.textContent = e.dir ? "—" : fmtBytes(e.size);

    const tdMod = document.createElement("td");
    tdMod.className = "col-mod";
    tdMod.textContent = e.mtime ? e.mtime.replace("T", " ") : "";

    const tdAct = document.createElement("td");
    tdAct.className = "col-act";
    if (!e.dir) {
      if (isRunnable(e.name)) {
        const run = document.createElement("button");
        run.className = "run";
        run.textContent = "Run";
        run.title = "Autostart on the emulator";
        run.addEventListener("click", () => runFile(childPath, e.name));
        tdAct.appendChild(run);
      }

      const dl = document.createElement("a");
      dl.className = "dl";
      dl.href = "/api/fs/download?vol=" + encodeURIComponent(vol) +
                "&path=" + encodeURIComponent(childPath);
      dl.textContent = "Download";
      tdAct.appendChild(dl);

      const del = document.createElement("button");
      del.className = "del";
      del.textContent = "Delete";
      del.addEventListener("click", () => deleteEntry(childPath, e.name));
      tdAct.appendChild(del);
    }

    tr.append(tdName, tdSize, tdMod, tdAct);
    frag.appendChild(tr);
  }
  tbody.appendChild(frag);

  if (!entries.length) {
    $("fb-status").textContent = "Empty folder.";
  } else if (data.truncated) {
    $("fb-status").textContent = entries.length + " entries (list truncated).";
  } else {
    $("fb-status").textContent = entries.length + " item" + (entries.length === 1 ? "" : "s") + ".";
  }
}

// ---- run (autostart) ----

async function runFile(path, name) {
  $("fb-status").className = "msg";
  $("fb-status").textContent = "Starting " + name + "…";
  try {
    const r = await fetch(
      "/api/fs/autostart?vol=" + encodeURIComponent(fbVol) +
      "&path=" + encodeURIComponent(path), { method: "POST" });
    if (!r.ok) throw new Error("HTTP " + r.status + " " + (await r.text()).trim());
  } catch (e) {
    $("fb-status").className = "msg err";
    $("fb-status").textContent = "Could not start " + name + " — " + e.message;
    return;
  }
  $("fb-status").textContent = "Started " + name + ".";
}

// ---- delete ----

async function deleteEntry(path, name) {
  if (!confirm("Delete “" + name + "”?\n\nThis cannot be undone.")) return;
  $("fb-status").className = "msg";
  $("fb-status").textContent = "Deleting " + name + "…";
  try {
    const r = await fetch(
      "/api/fs/delete?vol=" + encodeURIComponent(fbVol) +
      "&path=" + encodeURIComponent(path), { method: "POST" });
    if (!r.ok) throw new Error("HTTP " + r.status + " " + (await r.text()).trim());
  } catch (e) {
    $("fb-status").className = "msg err";
    $("fb-status").textContent = "Could not delete " + name + " — " + e.message;
    return;
  }
  loadDir(fbPath);
}

// ---- upload ----

function uploadOne(file, overwrite) {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    const url = "/api/fs/upload?vol=" + encodeURIComponent(fbVol) +
      "&path=" + encodeURIComponent(normPath(fbPath + "/" + file.name)) +
      (overwrite ? "&overwrite=1" : "");
    xhr.open("POST", url);
    xhr.upload.onprogress = (e) => {
      if (e.lengthComputable) {
        const pct = Math.round((e.loaded / e.total) * 100);
        $("fb-status").textContent = "Uploading " + file.name + " — " + pct + "%";
      }
    };
    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 300) { resolve(); return; }
      const bodyText = (xhr.responseText || "").trim();
      const err = new Error("HTTP " + xhr.status + " " + bodyText);
      if (xhr.status === 409 && /exist/i.test(bodyText)) err.conflict = true;
      reject(err);
    };
    xhr.onerror = () => reject(new Error("network error"));
    xhr.send(file);
  });
}

// Returns "ok" or "skipped"; throws on real failure.
async function uploadWithRetry(file, overwrite) {
  try {
    await uploadOne(file, overwrite);
    return "ok";
  } catch (e) {
    if (e.conflict && !overwrite) {
      if (!confirm("“" + file.name + "” already exists here. Overwrite it?")) {
        return "skipped";
      }
      await uploadOne(file, true);
      return "ok";
    }
    throw e;
  }
}

async function uploadFiles(fileList) {
  const files = Array.from(fileList || []);
  if (!files.length) return;

  const clashes = files.filter((f) => fbNames.has(f.name));
  if (clashes.length && !confirm(
      "These file" + (clashes.length === 1 ? "" : "s") +
      " already exist here and will be overwritten:\n\n" +
      clashes.map((f) => f.name).join("\n") + "\n\nContinue?")) {
    $("fb-upload").value = "";
    return;
  }
  const overwriteNames = new Set(clashes.map((f) => f.name));

  $("fb-upload").disabled = true;
  $("fb-upload-btn").classList.add("disabled");
  $("fb-status").className = "msg";
  let done = 0;
  let skipped = 0;
  for (const f of files) {
    try {
      const result = await uploadWithRetry(f, overwriteNames.has(f.name));
      if (result === "skipped") skipped++;
      else done++;
    } catch (e) {
      $("fb-status").className = "msg err";
      $("fb-status").textContent = "Upload of " + f.name + " failed — " + e.message;
      break;
    }
  }
  $("fb-upload").disabled = false;
  $("fb-upload-btn").classList.remove("disabled");
  $("fb-upload").value = "";
  if (done + skipped === files.length) {
    let msg = "Uploaded " + done + " file" + (done === 1 ? "" : "s") + ".";
    if (skipped) msg += " Skipped " + skipped + " (not overwritten).";
    $("fb-status").textContent = msg;
  }
  loadDir(fbPath);
}

// ---- router ----

function route() {
  const hash = location.hash || "#/dashboard";
  const isFiles = hash.startsWith("#/files");
  $("view-dashboard").hidden = isFiles;
  $("view-files").hidden = !isFiles;

  document.querySelectorAll(".nav-item[data-view]").forEach((el) => {
    el.classList.toggle("active", el.dataset.view === (isFiles ? "files" : "dashboard"));
  });

  if (isFiles) {
    loadDir(pathFromHash());
  } else {
    refreshVolumes();
  }
}

// ---- wire up ----

$("qa-reboot").addEventListener("click", doReboot);
$("nav-reboot").addEventListener("click", doReboot);
$("qa-reset").addEventListener("click", doHardReset);
$("qa-disable").addEventListener("click", disableWebUi);
$("qa-files").addEventListener("click", () => { location.hash = filesHash("/"); });
$("fb-up").addEventListener("click", () => {
  location.hash = filesHash(parentPath(pathFromHash()));
});
$("fb-refresh").addEventListener("click", route);
$("fb-upload").addEventListener("change", (e) => uploadFiles(e.target.files));

window.addEventListener("hashchange", route);
document.addEventListener("visibilitychange", () => {
  if (document.hidden) { clearTimeout(pollTimer); pollTimer = null; }
  else pollStatus();
});

route();
pollStatus();
