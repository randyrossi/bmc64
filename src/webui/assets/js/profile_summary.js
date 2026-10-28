//
// profile_summary.js
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
// An at-a-glance summary of a profile's setup for the Profiles page: its
// drives, control ports and expansions, read from the profile's vice.ini
// and BMC64 settings file. Plain functions with no DOM, so tools/webui_test
// can run them under Node.js.
//

import { FIRST_DRIVE, NUM_DRIVES, baseName, parseKv } from "./profiles_data.js";

// Drive model labels, as the BMC64 menu's Change Model shows them
// (DRIVE_TYPE_* in vice-3.3/src/drive/drive.h).
const DRIVE_TYPES = {
  1540: "1540", 1541: "1541", 1542: "1541II", 1551: "1551", 1570: "1570",
  1571: "1571", 1573: "1571CR", 1581: "1581", 2000: "FD2000", 4000: "FD4000",
  2031: "2031", 2040: "2040", 3040: "3040", 4040: "4040", 1001: "1001",
  8050: "8050", 8250: "8250", 4844: "CMDHD",
};

// What drive 8 is when nothing says otherwise.
const DEFAULT_DRIVE_8 = { pet: 2031 };

// The menu's Port choices; settings files keep the choice's position.
const PORT_CHOICES = [
  "None", "USB Gamepad 1", "USB Gamepad 2", "GPIO Bank 1", "GPIO Bank 2",
  "CURS + SPACE", "NUMPAD 64825", "NUMPAD 17930", "CURS + LCTRL",
  "USB Mouse (1351)", "USB Mouse (MicroMys)", "Custom Keyset 1",
  "Custom Keyset 2", "USB Gamepad 3", "USB Gamepad 4",
];

// The vice.ini section of each machine; Plus4Emu has no vice.ini.
const VICE_SECTIONS = { c64: "C64", c128: "C128", vic20: "VIC20",
                        plus4: "PLUS4", pet: "PET" };

// "C64/PAL/HDMI" gives "c64".
export function machineKey(machine) {
  return String(machine || "").split("/")[0].trim().toLowerCase();
}

// The machine's section of a vice.ini, with lower case keys (VICE ignores
// the case of resource names) and the quotes taken off string values.
export function parseViceIni(text, machine) {
  const want = VICE_SECTIONS[machineKey(machine)];
  const values = new Map();
  if (!want) return values;
  let inside = false;
  let section = "";
  for (const raw of String(text).split("\n")) {
    const line = raw.trim();
    const head = /^\[(.*)\]$/.exec(line);
    if (head) {
      inside = head[1].trim().toUpperCase() === want;
      section += inside ? line + "\n" : "";
      continue;
    }
    if (inside) section += line + "\n";
  }
  for (const [key, value] of parseKv(section)) {
    values.set(key.toLowerCase(), value.replace(/^"(.*)"$/, "$1"));
  }
  return values;
}

function int(value, fallback) {
  const n = parseInt(value, 10);
  return Number.isNaN(n) ? fallback : n;
}

const fileLabel = (path) => baseName(path).replace(/\.(bin|rom)$/i, "");

function sizeLabel(kb) {
  return kb >= 1024 && kb % 1024 === 0 ? kb / 1024 + " MB" : kb + " KB";
}

function drives(key, vice, settings, disks) {
  const count = key === "plus4emu" ? 1 : NUM_DRIVES;
  const iec = key !== "vic20" && key !== "pet" && key !== "plus4emu";
  const list = [];
  for (let i = 0; i < count; i++) {
    const unit = FIRST_DRIVE + i;
    const fallback = unit === FIRST_DRIVE ? DEFAULT_DRIVE_8[key] || 1541 : 0;
    const type = int(settings.get("drive_type_" + unit),
                     int(vice.get("drive" + unit + "type"), fallback));
    const iecOn = iec && int(vice.get("iecdevice" + unit), 0) !== 0;
    list.push({
      unit,
      type: type ? DRIVE_TYPES[type] || String(type) : "",
      disk: disks[i] || "",
      iec: iecOn,
      iecDir: iecOn ? vice.get("fsdevice" + unit + "dir") || "" : "",
    });
  }
  return list;
}

function ports(key, settings) {
  const machinePorts = { pet: 0, vic20: 1 }[key] ?? 2;
  const list = [];
  for (let port = 1; port <= 4; port++) {
    const choice = int(settings.get("port_" + port), 0);
    // Ports 3 and 4 are userport adapters: only shown when in use.
    if (port > machinePorts && (port <= 2 || !choice)) continue;
    list.push({ port, device: PORT_CHOICES[choice] || "Port choice " + choice,
                none: !choice });
  }
  return list;
}

// Expansions and other things that set a profile apart, each { key, value,
// title } for a chip on the card.
function extras(key, vice) {
  const list = [];
  const add = (k, value, title) => list.push({ key: k, value, title: title || "" });
  if (key === "c64" || key === "c128") {
    if (int(vice.get("reu"), 0)) {
      const image = vice.get("reufilename") || "";
      add("REU", sizeLabel(int(vice.get("reusize"), 512)),
          image ? "REU image " + image : "");
    }
    if (int(vice.get("georam"), 0)) {
      add("GeoRAM", sizeLabel(int(vice.get("georamsize"), 512)),
          vice.get("georamfilename") || "");
    }
  }
  if (key === "vic20") {
    const blocks = [3, 8, 8, 8, 0, 8];
    const kb = blocks.reduce(
      (sum, size, i) => sum + (int(vice.get("ramblock" + i), 0) ? size : 0), 0);
    if (kb) add("RAM", "+" + kb + " KB");
  }
  const cart = vice.get("cartridgefile") || "";
  if (cart) add("Cart", fileLabel(cart), cart);
  const kernal = vice.get("kernalname") || "";
  if (kernal && !/^kernal/i.test(kernal)) add("Kernal", fileLabel(kernal), kernal);
  if ((key === "c64" || key === "c128") && vice.has("sidmodel")) {
    const model = int(vice.get("sidmodel"), 0) ? "8580" : "6581";
    add("SID", int(vice.get("sidstereo"), 0) ? model + " ×2" : model);
  }
  if (key === "pet" && vice.has("videosize")) {
    add("Screen", vice.get("videosize") + " columns");
  }
  // The 40/80 key: down (0) starts in 80 columns, up (1, VICE's default) in 40.
  if (key === "c128") {
    add("Screen", int(vice.get("c128columnkey"), 1) ? "40 columns" : "80 columns");
  }
  return list;
}

// The summary of a profile on `machine` from its vice.ini and settings file
// texts (null when missing) and its startup disks (disk_8 … disk_11).
// `known` is false when neither file exists yet, so drives and ports are
// only guesses.
export function summarise(machine, viceText, settingsText, disks = []) {
  const key = machineKey(machine);
  const vice = parseViceIni(viceText || "", machine);
  const settings = parseKv(settingsText || "");
  return {
    known: viceText !== null || settingsText !== null,
    drives: drives(key, vice, settings, disks),
    ports: ports(key, settings),
    extras: extras(key, vice),
  };
}
