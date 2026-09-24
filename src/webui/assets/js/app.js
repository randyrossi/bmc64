// Entry point: wires the modules together and switches between views.
//
//   api.js        every /api/* call
//   util.js       DOM lookup, formatting, path helpers
//   dashboard.js  status polling, hardware / storage meters, reboot & reset
//   files.js      SD card file browser
//   menu.js       pop-up menu for a file row's Actions button
//   editor.js     overlay editor for config files and BASIC listings
//   basic.js      C64 BASIC V2 tokeniser / detokeniser (listing <-> PRG)
//   ../update/    the Update page (release check, upload of bmc64-update.zip)

import { $ } from "./util.js";
import { initDashboard, refreshVolumes, startStatusPolling } from "./dashboard.js";
import { initFiles, loadDir, filesHash, pathFromHash } from "./files.js";
import { initEditor } from "./editor.js";
import { initUpdate, showUpdate } from "../update/update.js";

function route() {
  const hash = location.hash || "#/dashboard";
  const view = hash.startsWith("#/files") ? "files"
             : hash.startsWith("#/update") ? "update" : "dashboard";
  $("view-dashboard").hidden = view !== "dashboard";
  $("view-files").hidden = view !== "files";
  $("view-update").hidden = view !== "update";

  document.querySelectorAll(".nav-item[data-view]").forEach((el) => {
    el.classList.toggle("active", el.dataset.view === view);
  });

  if (view === "files") {
    loadDir(pathFromHash());
  } else if (view === "update") {
    showUpdate();
  } else {
    refreshVolumes();
  }
}

initDashboard();
initFiles();
initEditor();
initUpdate($("view-update"));

$("qa-files").addEventListener("click", () => { location.hash = filesHash("/"); });
window.addEventListener("hashchange", route);

route();
startStatusPolling();
