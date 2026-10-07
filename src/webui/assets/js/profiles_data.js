// Reading the profile files in /profiles (docs/PROFILES.md, Profile files)
// the same way the device does (src/profiles). Plain functions with no DOM,
// so tools/webui_test can run them under Node.js.

export const MAIN_ID = "main";
export const FIRST_DRIVE = 8;
export const NUM_DRIVES = 4;

const MAX_ID = 32;
const MAX_NAME = 32;
const MAX_CATEGORY = 32;
const MAX_DESCRIPTION = 64;
const MAX_MACHINE = 128;
const MAX_PATH = 256;

// key=value lines: '#' starts a comment line, spaces around keys and values
// are ignored, Windows line endings are accepted, and the last of a repeated
// key wins.
export function parseKv(text) {
  const values = new Map();
  for (const raw of String(text).split("\n")) {
    const line = raw.replace(/^[ \t\r]+|[ \t\r]+$/g, "");
    const equals = line.indexOf("=");
    if (line.startsWith("#") || equals < 0) continue;
    const key = line.slice(0, equals).replace(/[ \t\r]+$/, "");
    if (!key) continue;
    values.set(key, line.slice(equals + 1).replace(/^[ \t\r]+/, ""));
  }
  return values;
}

// 1 to 32 lowercase letters, digits and '-'.
export function idValid(id) {
  return typeof id === "string" && id.length > 0 && id.length <= MAX_ID &&
         /^[a-z0-9-]+$/.test(id);
}

const cut = (value, max) => (value || "").slice(0, max);

// The startup disks (disk_8 … disk_11) and autostart of a profile.txt or of
// Main's main/<machine>.txt.
function startupActions(values) {
  const disks = [];
  for (let i = 0; i < NUM_DRIVES; i++) {
    disks.push(cut(values.get("disk_" + (FIRST_DRIVE + i)), MAX_PATH));
  }
  return { disks, autostart: cut(values.get("autostart"), MAX_PATH) };
}

// A profile from its id (folder name) and profile.txt text, or null if it
// isn't a valid profile (it needs a name and a machine).
export function parseProfile(id, text) {
  const values = parseKv(text);
  const name = cut(values.get("name"), MAX_NAME);
  const machine = cut(values.get("machine"), MAX_MACHINE);
  if (!idValid(id) || id === MAIN_ID || !name || !machine) return null;
  return {
    id,
    name,
    machine,
    category: cut(values.get("category"), MAX_CATEGORY),
    description: cut(values.get("description"), MAX_DESCRIPTION),
    start: values.get("start") === "once" ? "once" : "switch",
    ...startupActions(values),
  };
}

// Main's startup actions from /profiles/main/<machine>.txt.
export function parseMainFile(text) {
  return startupActions(parseKv(text));
}

// active.txt: the power-on profile, a pending Start once, and the machine
// Main last ran on. Values that aren't profile ids are ignored, as at boot.
export function parseActive(text) {
  const values = parseKv(text);
  const profile = values.get("profile");
  const once = values.get("once");
  return {
    profile: idValid(profile) ? profile : MAIN_ID,
    once: idValid(once) ? once : "",
    mainMachine: cut(values.get("main_machine"), MAX_MACHINE),
  };
}

const MACHINE_LABELS = {
  vic20: "VIC-20",
  plus4: "Plus/4",
  plus4emu: "Plus/4",
  pet: "PET",
};

// The parts of a machine value: machine / standard / output / description
// (the description may hold '/' itself).
function machineParts(machine) {
  const parts = String(machine || "").split("/");
  const head = parts.slice(0, 3);
  if (parts.length > 3) head.push(parts.slice(3).join("/"));
  return head.map((p) => p.trim()).filter((p, i) => p || i === 0);
}

// Short machine name for display, e.g. "VIC20/PAL/HDMI" gives "VIC-20".
export function machineLabel(machine) {
  const name = machineParts(machine)[0] || "";
  return MACHINE_LABELS[name.toLowerCase()] || name;
}

// The rest of a machine value for display, e.g. "PAL · HDMI".
export function machineDetail(machine) {
  return machineParts(machine).slice(1).join(" · ");
}

// Profiles sorted by name as the menu lists them, grouped by category:
// those without a category first, then each category in the order its
// first profile appears.
export function groupProfiles(profiles) {
  const lower = (s) => s.replace(/[A-Z]/g, (c) => c.toLowerCase());
  const sorted = profiles.slice().sort((a, b) => {
    const x = lower(a.name);
    const y = lower(b.name);
    return x < y ? -1 : x > y ? 1 : 0;
  });
  const groups = [{ category: "", profiles: [] }];
  for (const p of sorted) {
    let group = groups.find((g) => g.category === p.category);
    if (!group) {
      group = { category: p.category, profiles: [] };
      groups.push(group);
    }
    group.profiles.push(p);
  }
  return groups.filter((g) => g.profiles.length);
}

export function baseName(path) {
  const s = String(path || "");
  return s.slice(s.lastIndexOf("/") + 1);
}

// ---- Rename and delete, as the menu does them ----

// A name as the menu stores it: spaces trimmed, at most 32 characters, on
// one line. "" if nothing is left.
export function cleanName(input) {
  const name = String(input || "").replace(/^ +/, "").slice(0, MAX_NAME)
    .replace(/[\r\n]/g, " ");
  return name.replace(/ +$/, "");
}

// The key of a key=value line, or "" for comments and other lines.
function lineKey(line) {
  const trimmed = line.replace(/^[ \t\r]+/, "");
  const equals = trimmed.indexOf("=");
  if (trimmed.startsWith("#") || equals < 0) return "";
  return trimmed.slice(0, equals).replace(/[ \t\r]+$/, "");
}

function lineValue(line) {
  return line.slice(line.indexOf("=") + 1).replace(/^[ \t\r]+|[ \t\r]+$/g, "");
}

// Rewrites a file's lines with edit(line, key), which returns the new line
// or null to drop it. Keeps the file's line endings.
function editLines(text, edit) {
  const eol = /\r\n/.test(text) ? "\r\n" : "\n";
  const lines = String(text).split(/\r?\n/);
  if (lines[lines.length - 1] === "") lines.pop();
  const out = [];
  for (const line of lines) {
    const changed = edit(line, lineKey(line));
    if (changed !== null) out.push(changed);
  }
  return out.length ? out.join(eol) + eol : "";
}

// profile.txt text with its name changed; everything else is kept.
export function withName(text, name) {
  let named = false;
  const result = editLines(text, (line, key) => {
    if (key !== "name") return line;
    if (named) return null;
    named = true;
    return "name=" + name;
  });
  return named ? result : "name=" + name + (/\r\n/.test(text) ? "\r\n" : "\n") + result;
}

// active.txt text once profile id is deleted: Main starts at power-on
// instead of it, and a pending Start once of it is dropped. null if active.txt
// doesn't mention it.
export function activeWithout(text, id) {
  let changed = false;
  const result = editLines(text, (line, key) => {
    if (lineValue(line) !== id) return line;
    if (key === "profile") {
      changed = true;
      return "profile=" + MAIN_ID;
    }
    if (key === "once") {
      changed = true;
      return null;
    }
    return line;
  });
  return changed ? result : null;
}

// The files the menu's Delete removes from a profile's folder (and the
// backups the web UI's editor makes). Anything else in the folder is kept,
// and then so is the folder.
export const PROFILE_FILES = [
  "profile.txt", "vice.ini", "vice.in~", "settings.txt",
  "profile.txt.bak", "vice.ini.bak", "settings.txt.bak",
];
