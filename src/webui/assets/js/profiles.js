//
// profiles.js
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// The Profiles page: the profiles on the SD card (docs/PROFILES.md), which
// one is running, which starts at power-on and which starts next. It only
// reads /profiles; each profile's files open in the usual editor.
//

import { $ } from "./util.js";
import * as api from "./api.js";
import { openEditor } from "./editor.js";
import { filesHash } from "./files.js";
import { toggleMenu } from "./menu.js";
import { setMachine } from "./machine.js";
import {
  MAIN_ID, PROFILE_FILES, activeWithout, baseName, cleanName,
  settingsFile, mainFilePath, mainSettingsPath,
  groupProfiles, idValid, machineDetail, machineLabel, parseActive,
  parseMainFile, parseProfile, withName,
} from "./profiles_data.js";
import { summarise } from "./profile_summary.js";

let pfVol = "SD";
let loading = false;
// Bumped on every render, so summaries still loading for an older render
// are dropped.
let generation = 0;
// What the page last showed: running, power-on and Start-once ids, and the
// machine running now ("C64", "C128", "VIC20", "Plus4", "Plus4Emu" or
// "Pet").
let lastState = { running: "", powerOn: MAIN_ID, once: "", machine: "" };

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

// Text that is cut in the middle when it doesn't fit: the start gives way
// first, so the end stays readable. The end is the last 14 characters of a
// file name (a disk's side and extension), or from `tailFrom` on.
function midCut(text, title, tailFrom) {
  const v = el("span", "v mid");
  const at = tailFrom === undefined ? text.length - 14 : tailFrom;
  if (at > 0) v.appendChild(el("span", "head", text.slice(0, at)));
  v.appendChild(el("span", "tail", at > 0 ? text.slice(at) : text));
  v.title = title || text;
  return v;
}

// A folder path cut in the middle, keeping its last folder whole:
// "/prg/games/" is "/prg" then "/games/".
function midCutPath(path) {
  return midCut(path, path, path.replace(/\/+$/, "").lastIndexOf("/"));
}

// What the profile runs at power-on.
function autostartLine(p) {
  const line = el("div", "pf-autostart" + (p.autostart ? "" : " none"));
  line.appendChild(el("span", "k", "▶"));
  line.appendChild(p.autostart ? midCut(baseName(p.autostart), p.autostart)
                               : el("span", "v", "No autostart"));
  return line;
}

// Placeholder for the summary until its files are read.
function specBox() {
  const box = el("div", "pf-spec loading");
  box.appendChild(el("div", "pf-drives pf-skel"));
  box.appendChild(el("div", "pf-skel short"));
  return box;
}

// One slot per drive, always shown, so cards line up for comparing. The
// slot only says what the drive is; its disk is listed below at full width.
function driveSlot(d) {
  const off = !d.type && !d.iec;
  const slot = el("div", "pf-drive" + (off ? " off" : "") +
                         (d.disk || d.iec ? " has-disk" : ""));
  slot.appendChild(el("span", "u", String(d.unit)));
  slot.appendChild(el("span", "t", d.type || (d.iec ? "IEC" : "—")));
  slot.title = "Drive " + d.unit + ": " + (off ? "off" : d.type || "IEC file system") +
    (d.disk ? "\nDisk: " + d.disk : "") +
    (d.iec ? "\nIEC folder: " + (d.iecDir || "(default)") : "");
  return slot;
}

// Label / value rows: the drives' disks and IEC folders, then the ports.
function specRows(s) {
  const rows = el("div", "pf-rows");
  const add = (key, value, kind) => {
    const row = el("span", "pf-row" + (kind ? " " + kind : ""));
    row.appendChild(el("span", "k", key));
    row.appendChild(value);
    rows.appendChild(row);
  };
  for (const d of s.drives) {
    if (d.disk) add("Disk " + d.unit, midCut(baseName(d.disk), d.disk), "disk");
    if (d.iec) {
      add("IEC " + d.unit, d.iecDir ? midCutPath(d.iecDir)
                                    : el("span", "v", "Default folder"), "iec");
    }
  }
  for (const port of s.ports) {
    add("Port " + port.port, el("span", "v", port.device), port.none ? "none" : "");
  }
  return rows;
}

function renderSpec(box, s) {
  box.className = "pf-spec";
  box.textContent = "";
  const drives = el("div", "pf-drives");
  drives.style.setProperty("--n", s.drives.length);
  s.drives.forEach((d) => drives.appendChild(driveSlot(d)));
  box.appendChild(drives);

  const rows = specRows(s);
  if (rows.childElementCount) box.appendChild(rows);

  const extras = el("div", "pf-extras");
  for (const x of s.extras) {
    const chip = el("span", "pf-x");
    chip.appendChild(el("span", "k", x.key));
    chip.appendChild(midCut(x.value, x.title));
    extras.appendChild(chip);
  }
  if (!s.extras.length) extras.appendChild(el("span", "pf-x none", "No expansions"));
  box.appendChild(extras);

  if (!s.known) {
    box.appendChild(el("div", "pf-spec-note",
                       "No settings saved yet; showing the defaults."));
  }
}

// The settings file of a profile folder; an older profile has settings.txt
// until it next starts.
async function readSettings(dir, machine) {
  const own = await readText(dir + "/" + settingsFile(machine));
  return own !== null ? own : readText(dir + "/settings.txt");
}

// Fills in the cards' summaries one profile at a time, after the cards are
// on screen, so the page doesn't wait on every profile's files.
async function fillSpecs(jobs, gen) {
  for (const job of jobs) {
    let texts;
    try {
      texts = await Promise.all([readText(job.vice), job.settings()]);
    } catch (e) {
      if (gen === generation) setMsg("Could not read the profiles — " + e.message, true);
      return;
    }
    if (gen !== generation) return;
    renderSpec(job.box, summarise(job.machine, texts[0], texts[1], job.disks));
  }
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
  card.appendChild(autostartLine(p));
  card.spec = specBox();
  card.appendChild(card.spec);

  const bar = el("div", "pf-actions");
  if (p.id !== state.running) bar.appendChild(startButtons(p));
  // The profile's files as quiet links, always one click away; the rarer
  // actions behind ⋯ at the end of the same line.
  const files = el("div", "pf-files");
  for (const file of actions.files) {
    const b = el("button", "btn pf-file", "✎ " + file.label);
    b.type = "button";
    b.title = file.title;
    b.addEventListener("click", file.run);
    files.appendChild(b);
  }
  if (actions.menu.length) {
    const b = el("button", "btn act-btn pf-more", "⋯");
    b.type = "button";
    b.title = "More for " + p.name;
    b.setAttribute("aria-haspopup", "menu");
    b.setAttribute("aria-expanded", "false");
    b.addEventListener("click", () => toggleMenu(b, actions.menu));
    files.appendChild(b);
  }
  bar.appendChild(files);
  card.appendChild(bar);
  return card;
}

// The profile's settings for its machine; an older profile has them in
// settings.txt until it next starts.
async function editSettings(dir, machine) {
  const own = dir + "/" + settingsFile(machine);
  const legacy = dir + "/settings.txt";
  const path = (await readText(own)) === null && (await readText(legacy)) !== null
    ? legacy : own;
  editFile(path);
}

// ---- Switch to / Start once ----

const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

// The profile's choice ("start" in profile.txt) is the main button.
// The profile's choice ("start" in profile.txt) is the main button.
function startButtons(p) {
  const row = el("div", "pf-start");
  const make = (once) => {
    const b = el("button", "btn", once ? "Start once" : "▶ Switch to");
    b.type = "button";
    b.title = once ? "Restart into " + p.name + " for this session only"
                   : "Restart into " + p.name + " and use it at every power-on";
    b.addEventListener("click", () => startProfile(p, once));
    return b;
  };
  const first = p.start === "once";
  const main = make(first);
  main.classList.add("btn-primary");
  row.appendChild(main);
  row.appendChild(make(!first));
  return row;
}

function showRestart(title, text, { spinning = true, closable = false } = {}) {
  $("pf-restart-title").textContent = title;
  $("pf-restart-msg").textContent = text;
  $("pf-restart-spin").hidden = !spinning;
  $("pf-restart-close").hidden = !closable;
  $("pf-restart").hidden = false;
  document.body.classList.add("modal-open");
  if (closable) $("pf-restart-close").focus();
}

function hideRestart() {
  $("pf-restart").hidden = true;
  document.body.classList.remove("modal-open");
}

async function startProfile(p, once) {
  const target = (p.machine.split("/")[0] || "").trim().toLowerCase();
  const label = machineLabel(p.machine);
  const current = lastState.machine.toLowerCase();
  const otherMachine = target && current && target !== current;
  let question = once
    ? "Start “" + p.name + "” once?\n\nBMC64 restarts into it for this " +
      "session only; your usual profile starts at the next power-on."
    : "Switch to “" + p.name + "”?\n\nBMC64 restarts into it, and it's " +
      "used at every power-on from now on.";
  if (otherMachine) {
    question += "\n\nIt's for the " + label + ", so BMC64 switches machine too.";
  }
  if (!confirm(question + "\n\nAnything not saved in the emulator is lost.")) return;

  let uptime = Infinity;
  try {
    uptime = (await api.getStatus()).uptime_secs;
  } catch (e) {
    /* checked again below */
  }
  try {
    await api.startProfile(p.id, once);
  } catch (e) {
    setMsg("Could not start " + p.name + " — " + e.message, true);
    return;
  }
  await waitForRestart(p.name, uptime);
}

// Waits for BMC64 to restart (it goes offline, or its uptime starts again),
// or for the reason it couldn't.
async function waitForRestart(name, uptimeBefore) {
  showRestart("Restarting into " + name + "…",
              "BMC64 is restarting. This page reconnects by itself.");
  let wentDown = false;
  const giveUp = Date.now() + 120000;
  while (Date.now() < giveUp) {
    await sleep(1500);
    let s = null;
    try {
      s = await api.getStatus();
    } catch (e) {
      if (e.status === 401) {
        hideRestart();
        setMsg("Authentication required — reload the page and enter the PIN.", true);
        return;
      }
    }
    if (!s) {
      wentDown = true;
      continue;
    }
    const restarted = wentDown || s.uptime_secs < uptimeBefore;
    if (!restarted && s.profile_error) {
      hideRestart();
      setMsg("Could not start " + name + " — " + s.profile_error, true);
      return;
    }
    if (restarted) {
      hideRestart();
      await loadProfiles();
      return;
    }
  }
  showRestart("Still waiting for BMC64",
              "It hasn't come back yet. Refresh this page once it has.",
              { spinning: false, closable: true });
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

// The items of the card's Actions menu: its files first.
function profileActions(p) {
  const dir = "/profiles/" + p.id;
  return {
    files: [
      { label: "profile", title: "Edit " + dir + "/profile.txt",
        run: () => editFile(dir + "/profile.txt") },
      { label: "settings", title: "Edit the profile's BMC64 settings for the " +
          (machineLabel(p.machine) || "machine"),
        run: () => editSettings(dir, p.machine) },
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

// Main's file buttons: its settings and startup file ("" when it has none)
// for the machine it last ran on, and /vice.ini.
function mainActions(settings, mainFile) {
  const files = [];
  if (settings) {
    files.push({ label: "settings", title: "Edit " + settings,
                 run: () => editFile(settings) });
  }
  files.push({ label: "vice.ini", title: "Edit /vice.ini",
               run: () => editFile("/vice.ini") });
  if (mainFile) {
    files.push({ label: "startup", title: "Edit " + mainFile + " (autostart and disks)",
                 run: () => editFile(mainFile) });
  }
  return { files, menu: [] };
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
  if (status) setMachine(status);
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
  // Main's files are those of the machine Main last ran on, which needn't
  // be the one running now.
  const mainMachine = active.mainMachine || machine;
  const mainPath = mainFilePath(mainMachine);
  const mainFile = listing && mainPath ? await readText(mainPath) : null;
  const main = {
    id: MAIN_ID,
    name: "Main",
    description: "Your usual settings, on whatever machine boots",
    machine: mainMachine,
    ...parseMainFile(mainFile || ""),
  };

  const state = {
    // Only known from the device's status; "" if it doesn't say.
    running: (status && status.profile_id) || "",
    powerOn: active.profile,
    once: active.once,
  };
  state.machine = machine;
  lastState = state;
  const names = new Map([[MAIN_ID, "Main"], ...profiles.map((p) => [p.id, p.name])]);

  const list = $("pf-list");
  const groups = groupProfiles(profiles);
  // Summaries to read once the cards are shown: Main's from the machine's
  // own vice.ini and settings, a profile's from its folder.
  const jobs = [];
  const card = (p, actions, vice, settings) => {
    const c = profileCard(p, state, actions);
    jobs.push({ box: c.spec, machine: p.machine, disks: p.disks, vice, settings });
    return c;
  };
  const mainSettings = mainSettingsPath(mainMachine);
  const profileCardFor = (p) => card(p, profileActions(p),
    "/profiles/" + p.id + "/vice.ini",
    () => readSettings("/profiles/" + p.id, p.machine));
  // Main and the profiles without a category share the first grid.
  const first = section("");
  first.grid.appendChild(card(main,
    mainActions(mainSettings, mainFile !== null ? mainPath : ""), "/vice.ini",
    () => (mainSettings ? readText(mainSettings) : Promise.resolve(null))));
  if (groups.length && !groups[0].category) {
    for (const p of groups.shift().profiles) first.grid.appendChild(profileCardFor(p));
  }
  list.appendChild(first.frag);
  for (const group of groups) {
    const sec = section(group.category);
    for (const p of group.profiles) sec.grid.appendChild(profileCardFor(p));
    list.appendChild(sec.frag);
  }
  renderSummary(state, names);
  fillSpecs(jobs, ++generation);

  if (!profiles.length) {
    setMsg("No profiles yet. Make one on the BMC64 menu: Profiles → New " +
           "profile from current settings.");
  } else {
    setMsg(profiles.length + (profiles.length === 1 ? " profile" : " profiles") + ".");
  }
}

export function initProfiles() {
  $("pf-refresh").addEventListener("click", loadProfiles);
  $("pf-restart-close").addEventListener("click", hideRestart);
}
