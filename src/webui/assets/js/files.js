// Files view: browse the SD card, upload, download, rename, delete, create
// folders, autostart, and open the config-file and BASIC editors. Each row has one
// "Actions" button that opens a menu of what applies to that entry;
// clicking the name does the most common one (open a folder, run, edit).

import { $, fmtBytes, normPath, parentPath } from "./util.js";
import * as api from "./api.js";
import { openEditor, openBasicEditor } from "./editor.js";
import { LOAD_ADDRESS, parsePrg } from "./basic.js";
import { toggleMenu } from "./menu.js";

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
  ["d64", "d71", "d81", "d82", "g64", "x64", "t64", "tap", "prg", "p00", "crt"]);

const extOf = (name) => (name.split(".").pop() || "").toLowerCase();

function isRunnable(name) {
  return RUNNABLE.has(extOf(name));
}

// Disks, tapes and programs are autostarted, like the menu's Autostart item;
// a cartridge image is attached (which resets the machine).
const runVerb = (name) =>
  extOf(name) === "crt" ? "Attach cartridge" : "Autostart";

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

// The entries of a row's Actions menu; empty when nothing applies (the
// device marks its own config files and /firmware as protected).
function rowActions(e, path) {
  const items = [];
  if (!e.dir && isRunnable(e.name)) {
    items.push({ label: runVerb(e.name), run: () => runFile(path, e.name) });
  }
  if (!e.dir && e.edit) {
    items.push({ label: "Edit…", run: () => editFile(path, e.name) });
  }
  // Whether a .prg is BASIC can only be told by reading it, which is left to
  // the click rather than done for every file in a folder listing.
  if (!e.dir && extOf(e.name) === "prg") {
    items.push({ label: "Edit listing…", run: () => editListing(path, e) });
  }
  if (!e.dir) {
    items.push({ label: "Download", href: api.downloadUrl(fbVol, path) });
  }
  if (!e.protected) {
    items.push({ label: "Rename…", run: () => renameEntry(path, e.name) });
    items.push({ label: "Delete", danger: true,
                 run: () => deleteEntry(path, e.name, e.dir) });
  }
  return items;
}

function actionsButton(e, path) {
  const items = rowActions(e, path);
  if (!items.length) return null;
  const b = document.createElement("button");
  b.className = "btn act-btn";
  b.textContent = "Actions ▾";
  b.title = "Actions for " + e.name;
  b.setAttribute("aria-haspopup", "menu");
  b.setAttribute("aria-expanded", "false");
  b.addEventListener("click", () => toggleMenu(b, items));
  return b;
}

// "YYYY-MM-DDTHH:MM:SS" -> "YYYY-MM-DD HH:MM" (the phone layout has no
// room for the seconds).
const shortTime = (mtime) => mtime.slice(0, 16).replace("T", " ");

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
    let nm;
    if (e.dir) {
      nm = document.createElement("a");
      nm.href = filesHash(childPath);
      nm.textContent = e.name;
    } else if (isRunnable(e.name)) {
      nm = nameLink(e.name, runVerb(e.name) + " " + e.name,
                    () => runFile(childPath, e.name));
    } else if (e.edit) {
      nm = nameLink(e.name, "Edit " + e.name,
                    () => editFile(childPath, e.name));
    } else {
      nm = document.createElement("span");
      nm.textContent = e.name;
    }
    nm.classList.add("nm");
    wrap.appendChild(nm);
    tdName.appendChild(wrap);

    // Size and date sit under the name on a phone, where their own columns
    // are hidden.
    const meta = [e.dir ? "" : fmtBytes(e.size),
                  e.mtime ? shortTime(e.mtime) : ""].filter(Boolean).join(" · ");
    if (meta) {
      const sub = document.createElement("div");
      sub.className = "fmeta";
      sub.textContent = meta;
      tdName.appendChild(sub);
    }

    const tdSize = document.createElement("td");
    tdSize.className = "col-size";
    tdSize.textContent = e.dir ? "—" : fmtBytes(e.size);

    const tdMod = document.createElement("td");
    tdMod.className = "col-mod";
    tdMod.textContent = e.mtime ? e.mtime.replace("T", " ") : "";

    const tdAct = document.createElement("td");
    tdAct.className = "col-act";
    const actions = actionsButton(e, childPath);
    if (actions) tdAct.appendChild(actions);

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

// ---- BASIC programs ----

// A BASIC program can't be bigger than the memory from its load address to
// the end of BASIC RAM ($A000), plus the two load address bytes.
const BASIC_FILE_MAX = 0xa000 - LOAD_ADDRESS + 2;

async function newBasic() {
  const result = await openBasicEditor({ vol: fbVol, dir: fbPath });
  await savedBasic(result);
}

async function editListing(path, e) {
  if (e.size > BASIC_FILE_MAX) {
    showError(e.name + " is too big to be a BASIC program.");
    return;
  }
  $("fb-status").className = "msg";
  $("fb-status").textContent = "Reading " + e.name + "…";
  let parsed;
  try {
    parsed = parsePrg(new Uint8Array(await api.readFile(fbVol, path)));
  } catch (err) {
    showError("Could not read " + e.name + " — " + err.message);
    return;
  }
  if (!parsed) {
    showError(e.name + " is not a C64 BASIC program, so it has no listing to edit.");
    return;
  }
  $("fb-status").textContent = "";
  const result = await openBasicEditor({
    vol: fbVol, dir: fbPath, path, name: e.name, parsed,
  });
  await savedBasic(result);
}

async function savedBasic(result) {
  if (!result.saved) return;
  await loadDir(fbPath);
  $("fb-status").textContent =
    "Saved " + result.name + " (" + result.size + " bytes).";
}

// ---- delete ----

async function deleteEntry(path, name, isDir) {
  const question = isDir
    ? "Delete the folder “" + name + "” and everything in it?\n\n" +
      "Detach any disk image inside it first. This cannot be undone."
    : "Delete “" + name + "”?\n\nThis cannot be undone.";
  if (!confirm(question)) return;
  $("fb-status").className = "msg";
  $("fb-status").textContent = "Deleting " + name + "…";
  try {
    await api.deleteFile(fbVol, path, { recursive: isDir });
  } catch (e) {
    $("fb-status").className = "msg err";
    $("fb-status").textContent = "Could not delete " + name + " — " + e.message;
    return;
  }
  loadDir(fbPath);
}

// ---- new folder / rename ----

// A folder or file name is one path segment; the device checks the rest.
const hasSlash = (name) => /[\/\\]/.test(name);

function showError(text) {
  $("fb-status").className = "msg err";
  $("fb-status").textContent = text;
}

async function makeFolder() {
  const input = prompt("New folder name:");
  if (input === null) return;
  const name = input.trim();
  if (!name) return;
  if (hasSlash(name) || /^\.+$/.test(name)) {
    showError("“" + name + "” is not a valid folder name.");
    return;
  }
  $("fb-status").className = "msg";
  $("fb-status").textContent = "Creating " + name + "…";
  try {
    await api.makeDir(fbVol, normPath(fbPath + "/" + name));
  } catch (e) {
    showError("Could not create " + name + " — " + e.message);
    return;
  }
  await loadDir(fbPath);
  $("fb-status").textContent = "Created folder " + name + ".";
}

async function renameEntry(path, name) {
  const input = prompt("Rename “" + name + "” to:", name);
  if (input === null) return;
  const newName = input.trim();
  if (!newName || newName === name) return;
  if (hasSlash(newName) || /^\.+$/.test(newName)) {
    showError("“" + newName + "” is not a valid name.");
    return;
  }
  $("fb-status").className = "msg";
  $("fb-status").textContent = "Renaming " + name + "…";
  try {
    await api.renameEntry(fbVol, path, newName);
  } catch (e) {
    showError("Could not rename " + name + " — " + e.message);
    return;
  }
  await loadDir(fbPath);
  $("fb-status").textContent = "Renamed " + name + " to " + newName + ".";
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
  $("fb-mkdir").addEventListener("click", makeFolder);
  $("fb-newbasic").addEventListener("click", newBasic);
  $("fb-upload").addEventListener("change", (e) => uploadFiles(e.target.files));
}
