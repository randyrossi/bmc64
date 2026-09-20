// Files view: browse the SD card, upload, download, delete, autostart, and
// open the config-file editor.

import { $, fmtBytes, normPath, parentPath } from "./util.js";
import * as api from "./api.js";
import { openEditor } from "./editor.js";

// current location (set by loadDir)
let fbVol = "SD";
let fbPath = "/";
let fbNames = new Set(); // file names in the current folder (for overwrite checks)

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

export function filesHash(path) {
  path = normPath(path);
  if (path === "/") return "#/files";
  return "#/files" + path.split("/").map(encodeURIComponent).join("/");
}

export function pathFromHash() {
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

// A file name that acts as a link running `action` when clicked.
function nameLink(name, title, action) {
  const a = document.createElement("a");
  a.href = "#";
  a.title = title;
  a.textContent = name;
  a.addEventListener("click", (ev) => {
    ev.preventDefault();
    action();
  });
  return a;
}

function actionButton(cls, label, title, action) {
  const b = document.createElement("button");
  b.className = cls;
  b.textContent = label;
  if (title) b.title = title;
  b.addEventListener("click", action);
  return b;
}

export async function loadDir(path) {
  path = normPath(path);
  $("fb-up").disabled = path === "/";
  $("fb-status").className = "msg";
  $("fb-status").textContent = "Loading…";
  $("fb-rows").textContent = "";

  let data;
  try {
    data = await api.listDir(path);
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
      wrap.appendChild(nameLink(e.name, "Run " + e.name,
                                () => runFile(childPath, e.name)));
    } else if (e.edit) {
      wrap.appendChild(nameLink(e.name, "Edit " + e.name,
                                () => editFile(childPath, e.name)));
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
        tdAct.appendChild(actionButton("run", "Run", "Autostart on the emulator",
                                       () => runFile(childPath, e.name)));
      }
      if (e.edit) {
        tdAct.appendChild(actionButton("edit", "Edit", "Edit this file here",
                                       () => editFile(childPath, e.name)));
      }

      const dl = document.createElement("a");
      dl.className = "dl";
      dl.href = api.downloadUrl(vol, childPath);
      dl.textContent = "Download";
      tdAct.appendChild(dl);

      tdAct.appendChild(actionButton("del", "Delete", "",
                                     () => deleteEntry(childPath, e.name)));
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
    await api.autostart(fbVol, path);
  } catch (e) {
    $("fb-status").className = "msg err";
    $("fb-status").textContent = "Could not start " + name + " — " + e.message;
    return;
  }
  $("fb-status").textContent = "Started " + name + ".";
}

// ---- edit ----

async function editFile(path, name) {
  const result = await openEditor({ vol: fbVol, path, name });
  if (!result.saved) return;
  if (result.rebooting) {
    $("fb-status").className = "msg";
    $("fb-status").textContent = "Saved " + name + ". BMC64 is rebooting.";
    return;
  }
  await loadDir(fbPath);
  $("fb-status").textContent =
    "Saved " + name + ". Changes take effect after the next reboot.";
}

// ---- delete ----

async function deleteEntry(path, name) {
  if (!confirm("Delete “" + name + "”?\n\nThis cannot be undone.")) return;
  $("fb-status").className = "msg";
  $("fb-status").textContent = "Deleting " + name + "…";
  try {
    await api.deleteFile(fbVol, path);
  } catch (e) {
    $("fb-status").className = "msg err";
    $("fb-status").textContent = "Could not delete " + name + " — " + e.message;
    return;
  }
  loadDir(fbPath);
}

// ---- upload ----

function uploadOne(file, overwrite) {
  return api.upload(fbVol, normPath(fbPath + "/" + file.name), file, {
    overwrite,
    onProgress: (frac) => {
      $("fb-status").textContent =
        "Uploading " + file.name + " — " + Math.round(frac * 100) + "%";
    },
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

export function initFiles() {
  $("fb-up").addEventListener("click", () => {
    location.hash = filesHash(parentPath(pathFromHash()));
  });
  $("fb-refresh").addEventListener("click", () => loadDir(pathFromHash()));
  $("fb-upload").addEventListener("change", (e) => uploadFiles(e.target.files));
}
