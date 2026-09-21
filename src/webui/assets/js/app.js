// Entry point: wires the modules together and switches between views.
//
//   api.js        every /api/* call
//   util.js       DOM lookup, formatting, path helpers
//   dashboard.js  status polling, hardware / storage meters, reboot & reset
//   files.js      SD card file browser
//   menu.js       pop-up menu for a file row's Actions button
//   editor.js     overlay editor for config files

import { $ } from "./util.js";
import { initDashboard, refreshVolumes, startStatusPolling } from "./dashboard.js";
import { initFiles, loadDir, filesHash, pathFromHash } from "./files.js";
import { initEditor } from "./editor.js";

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

initDashboard();
initFiles();
initEditor();

$("qa-files").addEventListener("click", () => { location.hash = filesHash("/"); });
window.addEventListener("hashchange", route);

route();
startStatusPolling();
