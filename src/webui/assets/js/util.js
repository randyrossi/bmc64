//
// util.js
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
// Small helpers shared by the other modules: DOM lookup, number
// formatting and file-path manipulation.
//

export const $ = (id) => document.getElementById(id);

export function fmtUptime(secs) {
  secs = Math.floor(secs || 0);
  const d = Math.floor(secs / 86400);
  const h = Math.floor((secs % 86400) / 3600);
  const m = Math.floor((secs % 3600) / 60);
  const s = secs % 60;
  const parts = [];
  if (d) parts.push(d + "d");
  if (d || h) parts.push(h + "h");
  parts.push(m + "m");
  if (!d) parts.push(s + "s");
  return parts.join(" ");
}

export function fmtKB(kb) {
  kb = Number(kb) || 0;
  if (kb < 1024) return kb + " KB";
  const mb = kb / 1024;
  if (mb < 1024) return mb.toFixed(mb < 10 ? 1 : 0) + " MB";
  const gb = mb / 1024;
  return gb.toFixed(gb < 10 ? 2 : 1) + " GB";
}

export function fmtBytes(n) {
  return fmtKB(Math.ceil((Number(n) || 0) / 1024));
}

export function normPath(p) {
  if (!p || p[0] !== "/") p = "/" + (p || "");
  const parts = p.split("/").filter((s) => s && s !== ".");
  return "/" + parts.join("/");
}

export function parentPath(p) {
  p = normPath(p);
  if (p === "/") return "/";
  return normPath(p.slice(0, p.lastIndexOf("/")) || "/");
}
