// Every call to the device's /api/* endpoints lives here, so the other
// modules deal only in plain values and ApiError.

export class ApiError extends Error {
  constructor(status, detail) {
    super(status === 401
      ? "authentication required — reload the page and enter the PIN"
      : "HTTP " + status + (detail ? " " + detail : ""));
    this.status = status;
  }
}

async function check(r) {
  if (!r.ok) throw new ApiError(r.status, (await r.text()).trim());
  return r;
}

async function getJson(url) {
  return (await check(await fetch(url, { cache: "no-store" }))).json();
}

async function post(url, init) {
  await check(await fetch(url, { method: "POST", ...init }));
}

const fsQuery = (vol, path) =>
  "vol=" + encodeURIComponent(vol) + "&path=" + encodeURIComponent(path);

// Required by the device on every call that changes the card's config or
// layout (see webui.cpp); it keeps other sites from triggering them.
const WEB_HEADER = { "X-BMC64-Web": "1" };

// ---- status ----

export const getStatus = () => getJson("/api/status");
export const getVolumes = () => getJson("/api/volumes");

export const reboot = () => post("/api/reboot");
export const hardReset = () => post("/api/reset");
export const disableWebUi = () => post("/api/webui/disable");

// ---- files ----

// No vol: the first listing tells us which volume the device uses.
export const listDir = (path) =>
  getJson("/api/fs/list?path=" + encodeURIComponent(path));

export const downloadUrl = (vol, path) =>
  "/api/fs/download?" + fsQuery(vol, path);

// The file's bytes, as an ArrayBuffer.
export async function readFile(vol, path) {
  const r = await check(await fetch(downloadUrl(vol, path), { cache: "no-store" }));
  return r.arrayBuffer();
}

// With `recursive`, a folder is deleted together with everything in it.
export const deleteFile = (vol, path, { recursive = false } = {}) =>
  post("/api/fs/delete?" + fsQuery(vol, path) + (recursive ? "&recursive=1" : ""),
       recursive ? { headers: WEB_HEADER } : undefined);

export const autostart = (vol, path) =>
  post("/api/fs/autostart?" + fsQuery(vol, path));

// Replace an editable config file with `text`.
export const saveFile = (vol, path, text) =>
  post("/api/fs/save?" + fsQuery(vol, path), {
    headers: WEB_HEADER,
    body: text,
  });

// Create the folder `path` (its parent must exist).
export const makeDir = (vol, path) =>
  post("/api/fs/mkdir?" + fsQuery(vol, path), { headers: WEB_HEADER });

// Rename the file or folder at `path` to `newName`, in the same folder.
export const renameEntry = (vol, path, newName) =>
  post("/api/fs/rename?" + fsQuery(vol, path) +
       "&to=" + encodeURIComponent(newName), { headers: WEB_HEADER });

// The file's original modified time as local "YYYY-MM-DDTHH:MM:SS" (FAT
// stores no time zone, and the file list shows the stored value as-is).
function localMtime(file) {
  if (!file.lastModified) return "";
  const d = new Date(file.lastModified);
  const p = (n) => String(n).padStart(2, "0");
  return d.getFullYear() + "-" + p(d.getMonth() + 1) + "-" + p(d.getDate()) +
    "T" + p(d.getHours()) + ":" + p(d.getMinutes()) + ":" + p(d.getSeconds());
}

// Upload `file` to `path`. XHR rather than fetch so we get progress events.
// A rejected promise carries `.conflict = true` when the file already exists.
export function upload(vol, path, file, { overwrite = false, onProgress } = {}) {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    const mtime = localMtime(file);
    xhr.open("POST", "/api/fs/upload?" + fsQuery(vol, path) +
      (mtime ? "&mtime=" + encodeURIComponent(mtime) : "") +
      (overwrite ? "&overwrite=1" : ""));
    xhr.upload.onprogress = (e) => {
      if (e.lengthComputable && onProgress) onProgress(e.loaded / e.total);
    };
    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 300) { resolve(); return; }
      const bodyText = (xhr.responseText || "").trim();
      const err = new ApiError(xhr.status, bodyText);
      if (xhr.status === 409 && /exist/i.test(bodyText)) err.conflict = true;
      reject(err);
    };
    xhr.onerror = () => reject(new Error("network error"));
    xhr.send(file);
  });
}
