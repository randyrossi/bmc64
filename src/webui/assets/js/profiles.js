// The Profiles page: the profiles on the SD card (docs/PROFILES.md), which
// one is running, which starts at power-on and which starts next. It only
// reads /profiles; each profile's files open in the usual editor.

import { $ } from "./util.js";
import * as api from "./api.js";
import { openEditor } from "./editor.js";
import { filesHash } from "./files.js";
import { toggleMenu } from "./menu.js";
import {
  FIRST_DRIVE, MAIN_ID, PROFILE_FILES, activeWithout, baseName, cleanName,
  groupProfiles, idValid, machineDetail, machineLabel, parseActive,
  parseMainFile, parseProfile, withName,
} from "./profiles_data.js";

// Main's usual settings files on the running machine (the web UI runs on
// the C64 and C128 only).
const MAIN_SETTINGS = { C64: "/settings.txt", C128: "/settings-c128.txt" };

let pfVol = "SD";
let loading = false;
// What the page last showed: running, power-on and Start-once ids.
let lastState = { running: "", powerOn: MAIN_ID, once: "" };

function setMsg(text, isErr) {
  $("pf-status").className = "msg" + (isErr ? " err" : "");
  $("pf-status").textContent = text;
}

// A file's text, or null if it can't be read (missing, for example).
async function readText(path) {
  try {
    return new TextDecoder("utf-8").decode(await api.readFile(pfVol, path));
  } catch (e) {
    if (e.status === 401) throw e;
    return null;
  }
}

async function editFile(path) {
  const result = await openEditor({ vol: pfVol, path, name: baseName(path) });
  if (result.saved && !result.rebooting) loadProfiles();
}

function el(tag, className, text) {
  const e = document.createElement(tag);
  if (className) e.className = className;
  if (text !== undefined) e.textContent = text;
  return e;
}

function badge(text, kind) {
  return el("span", "pf-badge " + kind, text);
}

const CHIP_CLASS = { c64: "", c128: "c128", vic20: "vic20", plus4: "plus4",
                     plus4emu: "plus4", pet: "pet" };

function machineChip(machine) {
  const name = (machine.split("/")[0] || "").trim().toLowerCase();
  const kind = name ? (CHIP_CLASS[name] || "") : "any";
  return el("span", "pf-chip " + kind, machineLabel(machine) || "Any machine");
}

// What the profile does at power-on: its autostart and startup disks.
function startsList(p) {
  const ul = el("ul", "pf-starts");
  const add = (key, path) => {
    const li = el("li");
    li.appendChild(el("span", "k", key));
    const v = el("span", "v", baseName(path));
    v.title = path;
    li.appendChild(v);
    ul.appendChild(li);
  };
  if (p.autostart) add("Autostart", p.autostart);
  p.disks.forEach((disk, i) => { if (disk) add("Drive " + (FIRST_DRIVE + i), disk); });
  if (!ul.childElementCount) {
    const li = el("li", "none", "No autostart or disks");
    ul.appendChild(li);
  }
  return ul;
}

function profileCard(p, state, actions) {
  const card = el("article", "pf-card");
  if (p.id === MAIN_ID) card.classList.add("pf-main");
  if (p.id === state.running) card.classList.add("running");

  const top = el("div", "pf-top");
  top.appendChild(machineChip(p.machine));
  const detail = machineDetail(p.machine);
  if (detail) top.appendChild(el("span", "pf-detail", detail));
  const badges = el("span", "pf-badges");
  if (p.id === state.running) badges.appendChild(badge("Running", "run"));
  if (p.id === state.powerOn) badges.appendChild(badge("Power-on", "power"));
  if (p.id === state.once) badges.appendChild(badge("Next start", "once"));
  top.appendChild(badges);
  card.appendChild(top);

  card.appendChild(el("div", "pf-name", p.name));
  if (p.description) card.appendChild(el("div", "pf-desc", p.description));
  card.appendChild(startsList(p));

  const bar = el("div", "pf-actions");
  for (const action of actions.buttons) {
    const b = el("button", "btn", action.label);
    b.type = "button";
    b.title = action.title || action.label;
    b.addEventListener("click", action.run);
    bar.appendChild(b);
  }
  if (actions.menu.length) {
    const b = el("button", "btn act-btn pf-more", "Actions ▾");
    b.type = "button";
    b.title = "Actions for " + p.name;
    b.setAttribute("aria-haspopup", "menu");
    b.setAttribute("aria-expanded", "false");
    b.addEventListener("click", () => toggleMenu(b, actions.menu));
    bar.appendChild(b);
  }
  card.appendChild(bar);
  return card;
}

// ---- rename / delete ----

// Like the menu's Rename: only the name changes, not the folder (id).
async function renameProfile(p) {
  const input = prompt("Rename “" + p.name + "” to:", p.name);
  if (input === null) return;
  const name = cleanName(input);
  if (!name || name === p.name) return;
  const path = "/profiles/" + p.id + "/profile.txt";
  setMsg("Renaming " + p.name + "…");
  try {
    const text = await readText(path);
    if (text === null) throw new Error("can't read profile.txt");
    await api.saveFile(pfVol, path, withName(text, name));
  } catch (e) {
    setMsg("Could not rename " + p.name + " — " + e.message, true);
    return;
  }
  await loadProfiles();
  setMsg("Renamed " + p.name + " to " + name + "." +
         (p.id === lastState.running
           ? " The BMC64 menu shows the new name after a restart." : ""));
}

// Like the menu's Delete: removes the profile's own files, then its folder
// if nothing else is in it. Also stops BMC64 starting it at power-on.
async function deleteProfile(p) {
  const powerOn = p.id === lastState.powerOn || p.id === lastState.once;
  if (!confirm("Delete the profile “" + p.name + "”?\n\n" +
               (powerOn ? "BMC64 will start Main at power-on instead.\n\n" : "") +
               "This cannot be undone.")) {
    return;
  }
  const dir = "/profiles/" + p.id;
  setMsg("Deleting " + p.name + "…");
  let keptFolder = false;
  try {
    const listing = await api.listDir(dir);
    const ours = new Set(PROFILE_FILES);
    for (const entry of listing.entries || []) {
      if (!entry.dir && ours.has(entry.name.toLowerCase())) {
        await api.deleteFile(pfVol, dir + "/" + entry.name);
      }
    }
    try {
      await api.deleteFile(pfVol, dir);
    } catch (e) {
      if (e.status === 401) throw e;
      keptFolder = true;  // the user's own files are still in it
    }
    const active = await readText("/profiles/active.txt");
    const changed = active === null ? null : activeWithout(active, p.id);
    if (changed !== null) await api.saveFile(pfVol, "/profiles/active.txt", changed);
  } catch (e) {
    setMsg("Could not delete " + p.name + " — " + e.message, true);
    loadProfiles();
    return;
  }
  await loadProfiles();
  setMsg("Deleted " + p.name + "." + (keptFolder
    ? " Its folder has other files in it, so the folder was kept." : ""));
}

// The card's file buttons, and the items of its Actions menu.
function profileActions(p) {
  const dir = "/profiles/" + p.id;
  return {
    buttons: [
      { label: "profile.txt", title: "Edit " + dir + "/profile.txt",
        run: () => editFile(dir + "/profile.txt") },
      { label: "settings.txt", title: "Edit " + dir + "/settings.txt",
        run: () => editFile(dir + "/settings.txt") },
      { label: "vice.ini", title: "Edit " + dir + "/vice.ini",
        run: () => editFile(dir + "/vice.ini") },
    ],
    menu: [
      { label: "Open folder", run: () => { location.hash = filesHash(dir); } },
      { label: "Rename…", run: () => renameProfile(p) },
      // Like the menu, the running profile can't be deleted.
      ...(p.id === lastState.running ? [] : [
        { label: "Delete", danger: true, run: () => deleteProfile(p) },
      ]),
    ],
  };
}

function mainActions(machine, hasMainFile) {
  const actions = [];
  const settings = MAIN_SETTINGS[machine];
  if (settings) {
    actions.push({ label: baseName(settings), title: "Edit " + settings,
                   run: () => editFile(settings) });
  }
  actions.push({ label: "vice.ini", title: "Edit /vice.ini",
                 run: () => editFile("/vice.ini") });
  if (hasMainFile) {
    const file = "/profiles/main/" + machine.toLowerCase() + ".txt";
    actions.push({ label: "main/" + baseName(file), title: "Edit " + file,
                   run: () => editFile(file) });
  }
  return { buttons: actions, menu: [] };
}

function tile(key, value, kind) {
  const t = el("div", "pf-tile " + kind);
  t.appendChild(el("div", "pf-tile-k", key));
  t.appendChild(el("div", "pf-tile-v", value));
  return t;
}

function renderSummary(state, names) {
  const name = (id) => names.get(id) || id;
  const box = $("pf-summary");
  if (state.running) box.appendChild(tile("Running", name(state.running), "run"));
  box.appendChild(tile("At power-on", name(state.powerOn), "power"));
  if (state.once) box.appendChild(tile("Next start", name(state.once), "once"));
}

function section(title) {
  const frag = document.createDocumentFragment();
  if (title) frag.appendChild(el("h3", "pf-section", title));
  const grid = el("div", "pf-grid");
  frag.appendChild(grid);
  return { frag, grid };
}

export async function loadProfiles() {
  if (loading) return;
  loading = true;
  setMsg("Loading…");
  $("pf-list").textContent = "";
  $("pf-summary").textContent = "";
  try {
    await render();
  } catch (e) {
    setMsg("Could not read the profiles — " + e.message, true);
  } finally {
    loading = false;
  }
}

async function render() {
  let status = null;
  try {
    status = await api.getStatus();
  } catch (e) {
    if (e.status === 401) throw e;
  }
  const machine = (status && status.machine) || "";

  let listing = null;
  try {
    listing = await api.listDir("/profiles");
  } catch (e) {
    if (e.status === 401) throw e;
  }
  if (!listing) {
    // No /profiles yet; the root listing still says which volume to use.
    try {
      pfVol = (await api.listDir("/")).vol || pfVol;
    } catch (e) {
      if (e.status === 401) throw e;
    }
  } else if (listing.vol) {
    pfVol = listing.vol;
  }

  // Profiles in folders with a valid id and a valid profile.txt, as the
  // menu lists them.
  const profiles = [];
  for (const entry of (listing && listing.entries) || []) {
    if (!entry.dir || !idValid(entry.name) || entry.name === MAIN_ID) continue;
    const text = await readText("/profiles/" + entry.name + "/profile.txt");
    const p = text === null ? null : parseProfile(entry.name, text);
    if (p) profiles.push(p);
  }

  const activeText = listing ? await readText("/profiles/active.txt") : null;
  const active = parseActive(activeText || "");
  const mainFile = listing && machine
    ? await readText("/profiles/main/" + machine.toLowerCase() + ".txt")
    : null;
  const main = {
    id: MAIN_ID,
    name: "Main",
    description: "Your usual settings, on whatever machine boots",
    machine: active.mainMachine || machine,
    ...parseMainFile(mainFile || ""),
  };

  const state = {
    // Only known from the device's status; "" if it doesn't say.
    running: (status && status.profile_id) || "",
    powerOn: active.profile,
    once: active.once,
  };
  lastState = state;
  const names = new Map([[MAIN_ID, "Main"], ...profiles.map((p) => [p.id, p.name])]);

  const list = $("pf-list");
  const groups = groupProfiles(profiles);
  // Main and the profiles without a category share the first grid.
  const first = section("");
  first.grid.appendChild(profileCard(main, state, mainActions(machine, mainFile !== null)));
  if (groups.length && !groups[0].category) {
    for (const p of groups.shift().profiles) {
      first.grid.appendChild(profileCard(p, state, profileActions(p)));
    }
  }
  list.appendChild(first.frag);
  for (const group of groups) {
    const sec = section(group.category);
    for (const p of group.profiles) {
      sec.grid.appendChild(profileCard(p, state, profileActions(p)));
    }
    list.appendChild(sec.frag);
  }
  renderSummary(state, names);

  if (!profiles.length) {
    setMsg("No profiles yet. Make one on the BMC64 menu: Profiles → New " +
           "profile from current settings.");
  } else {
    setMsg(profiles.length + (profiles.length === 1 ? " profile" : " profiles") + ".");
  }
}

export function initProfiles() {
  $("pf-refresh").addEventListener("click", loadProfiles);
}
