// Overlay text editor for the device's config files (vice.ini, settings*.txt,
// cmdline.txt, config.txt, machines.txt, wpa_supplicant.conf). Which files
// may be edited is decided by the device: the file list marks them "edit".

import { $ } from "./util.js";
import * as api from "./api.js";
import { rebootNow } from "./dashboard.js";

// Files the emulator's own menus also write; a menu save overwrites edits
// made here until the next reboot.
const MENU_WRITTEN = /^(settings.*\.txt|vice\.ini|wpa_supplicant\.conf)$/i;

// Keyboard mapping files (*.vkm) are editable in any folder.
const KEYMAP = /\.vkm$/i;

// The open editor, or null. Holds everything about the file being edited.
let session = null;

const ta = () => $("ed-text");

function isDirty() {
  return session.loaded && ta().value !== session.original;
}

function setMsg(text, isErr) {
  $("ed-msg").className = "msg ed-msg" + (isErr ? " err" : "");
  $("ed-msg").textContent = text;
}

function refreshButtons() {
  const canSave = session.loaded && !session.busy && isDirty();
  $("ed-save").disabled = !canSave;
  $("ed-save-reboot").disabled = !canSave;
  $("ed-cancel").disabled = session.busy;
  $("ed-close").disabled = session.busy;
}

function hintFor(name) {
  if (KEYMAP.test(name)) {
    return "Keyboard mapping. Changes take effect after a reboot. A mistake " +
      "can make keys type the wrong character or stop working; the previous " +
      "version is kept as " + name + ".bak.";
  }
  let text = "Changes take effect after a reboot. A mistake in a system " +
    "file can stop BMC64 starting; the previous version is kept as " +
    name + ".bak.";
  if (MENU_WRITTEN.test(name)) {
    text += " Saving from the emulator's menus before you reboot " +
      "overwrites your edits.";
  }
  return text;
}

async function load(s) {
  let bytes;
  try {
    bytes = await api.readFile(s.vol, s.path);
  } catch (e) {
    if (session === s) setMsg("Could not open " + s.name + " — " + e.message, true);
    return;
  }
  if (session !== s) return; // closed while loading

  let text;
  try {
    text = new TextDecoder("utf-8", { fatal: true }).decode(bytes);
    if (text.includes("\u0000")) throw new Error("binary data");
  } catch (e) {
    setMsg(s.name + " is not plain UTF-8 text, so it can't be edited here.", true);
    return;
  }

  // A textarea holds LF line endings; remember if the file used CRLF so
  // saving doesn't silently convert it.
  s.crlf = /\r\n/.test(text) && !/(^|[^\r])\n/.test(text);
  ta().value = text;
  s.original = ta().value; // as the textarea normalised it
  ta().disabled = false;
  s.loaded = true;
  setMsg("");
  refreshButtons();
  // Assigning .value leaves the caret at the end, and focusing would scroll
  // to it; put the caret at the start first and open at the top of the file.
  ta().setSelectionRange(0, 0);
  ta().focus({ preventScroll: true });
  ta().scrollTop = 0;
}

function close(result) {
  const s = session;
  session = null;
  $("editor").hidden = true;
  document.body.classList.remove("modal-open");
  ta().value = "";
  if (s.opener && s.opener.focus) s.opener.focus();
  s.resolve(result);
}

function requestCancel() {
  if (session.busy) return;
  if (isDirty() && !confirm("Discard your changes to " + session.name + "?")) return;
  close({ saved: false, rebooting: false });
}

async function save(andReboot) {
  const s = session;
  if (s.busy || !s.loaded || !isDirty()) return;
  if (andReboot && !confirm(
      "Save " + s.name + " and reboot BMC64 now?\n\n" +
      "Any unsaved emulator state will be lost.")) return;

  s.busy = true;
  ta().readOnly = true;
  refreshButtons();
  setMsg("Saving…");

  const text = s.crlf ? ta().value.replace(/\n/g, "\r\n") : ta().value;
  try {
    await api.saveFile(s.vol, s.path, text);
  } catch (e) {
    s.busy = false;
    ta().readOnly = false;
    refreshButtons();
    setMsg("Save failed — " + e.message, true);
    return;
  }

  if (andReboot) {
    setMsg("Saved. Rebooting…");
    await rebootNow();
  }
  close({ saved: true, rebooting: andReboot });
}

// Keep Tab / Shift+Tab inside the dialog while it is open.
function trapTab(ev) {
  const items = Array.from($("editor").querySelectorAll("button, textarea"))
    .filter((el) => !el.disabled);
  if (!items.length) return;
  const first = items[0];
  const last = items[items.length - 1];
  if (ev.shiftKey && document.activeElement === first) {
    ev.preventDefault();
    last.focus();
  } else if (!ev.shiftKey && document.activeElement === last) {
    ev.preventDefault();
    first.focus();
  }
}

// Resolves when the editor closes with { saved, rebooting }.
export function openEditor({ vol, path, name }) {
  if (session) return Promise.resolve({ saved: false, rebooting: false });

  return new Promise((resolve) => {
    const s = session = {
      vol, path, name, resolve,
      original: "", crlf: false, loaded: false, busy: false,
      opener: document.activeElement,
    };

    $("ed-title").textContent = name;
    $("ed-path").textContent = vol + ":" + path;
    $("ed-hint").textContent = hintFor(name);
    ta().value = "";
    ta().disabled = true;
    ta().readOnly = false;
    setMsg("Loading…");
    $("editor").hidden = false;
    document.body.classList.add("modal-open");
    refreshButtons();
    $("ed-cancel").focus();
    load(s);
  });
}

export function initEditor() {
  $("ed-cancel").addEventListener("click", requestCancel);
  $("ed-close").addEventListener("click", requestCancel);
  $("ed-save").addEventListener("click", () => save(false));
  $("ed-save-reboot").addEventListener("click", () => save(true));
  ta().addEventListener("input", refreshButtons);

  $("editor").addEventListener("keydown", (ev) => {
    if (!session) return;
    if (ev.key === "Escape") {
      ev.preventDefault();
      requestCancel();
    } else if ((ev.ctrlKey || ev.metaKey) && ev.key.toLowerCase() === "s") {
      ev.preventDefault();
      save(false);
    } else if (ev.key === "Tab") {
      trapTab(ev);
    }
  });

  window.addEventListener("beforeunload", (ev) => {
    if (session && isDirty()) {
      ev.preventDefault();
      ev.returnValue = "";
    }
  });
}
