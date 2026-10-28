//
// webui_profiles.h
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
// The Profiles page's files in one request, so the page doesn't fetch
// every profile's files one by one.
//

#ifndef _webui_profiles_h
#define _webui_profiles_h

class CSocket;

// GET /api/profiles/files  -> JSON with the text of every profile file in
// /profiles (docs/PROFILES.md, Profile files): active.txt, Main's
// main/<machine>.txt files, and each profile folder's profile.txt,
// vice.ini and settings files.
//
//   {"vol":"SD","folder":true,"ids":["geos",...],"truncated":false,
//    "files":{"/profiles/active.txt":"...","/profiles/geos/vice.ini":"...",...}}
//
// "folder" is false when there is no /profiles. "ids" are the folders
// with a valid profile id (other than main), whether or not they hold a
// profile.txt. Paths keep the case the card has. A file larger than 64 KB
// is left out, as if it were missing.
void WebUiProfileFiles(CSocket *socket);

#endif
