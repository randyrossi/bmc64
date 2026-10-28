//
// profile_summary.test.js
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
// Tests for the Profiles page's at-a-glance summary
// (src/webui/assets/js/profile_summary.js).
//
// A plain ES module with no dependencies. run_tests.mjs runs it under Node.js;
// runTests() returns { passed, failed, log }.
//

import { machineKey, parseViceIni, summarise }
  from "../../src/webui/assets/js/profile_summary.js";

export function runTests() {
  const log = [];
  let passed = 0;
  let failed = 0;

  function test(name, fn) {
    try {
      fn();
      passed++;
    } catch (e) {
      failed++;
      log.push("FAIL " + name + "\n     " + String(e.message).split("\n").join("\n     "));
    }
  }
  function eq(got, want, what) {
    const g = JSON.stringify(got);
    const w = JSON.stringify(want);
    if (g !== w) throw new Error((what ? what + ": " : "") + "got  " + g + "\nwant " + w);
  }

  test("machine key", () => {
    eq(machineKey("C128/PAL/HDMI/VICE 720p@50Hz"), "c128");
    eq(machineKey("VIC20"), "vic20");
    eq(machineKey(""), "");
  });

  test("vice.ini: only the machine's section, keys lower case, unquoted", () => {
    const ini = "[C128]\nREU=0\n[C64]\nREU=1\r\nKernalName=\"JiffyDOS.bin\"\n" +
                "[VIC20]\nREU=0\n";
    const v = parseViceIni(ini, "C64/PAL");
    eq(v.get("reu"), "1");
    eq(v.get("kernalname"), "JiffyDOS.bin");
    eq(parseViceIni(ini, "Plus4Emu").size, 0, "no vice.ini section");
  });

  test("drives: settings first, vice.ini next, then defaults", () => {
    const s = summarise("C64", "[C64]\nDrive9Type=1581\nDrive10Type=4844\n",
                        "drive_type_8=1542\ndrive_type_10=2000\n",
                        ["SD:/disks/C64/game.d64", "", "", ""]);
    eq(s.drives.map((d) => d.type), ["1541II", "1581", "FD2000", ""]);
    eq(s.drives[0].disk, "SD:/disks/C64/game.d64");
    eq(summarise("C64", null, null).drives.map((d) => d.type), ["1541", "", "", ""]);
    eq(summarise("Pet", null, null).drives[0].type, "2031");
    eq(summarise("Plus4Emu", null, null).drives.length, 1);
  });

  test("drives: IEC file system on C64, not on VIC-20", () => {
    const ini = "[C64]\nIECDevice11=1\nFSDevice11Dir=\"/disks/c64/\"\n" +
                "[VIC20]\nIECDevice11=1\n";
    const d = summarise("C64", ini, "drive_type_11=0\n").drives[3];
    eq([d.type, d.iec, d.iecDir], ["", true, "/disks/c64/"]);
    eq(summarise("VIC20", ini, null).drives[3].iec, false);
  });

  test("ports: the menu's choice names, userport only when used", () => {
    const s = summarise("C64", null, "port_1=10\nport_2=1\nport_3=0\nport_4=3\n");
    eq(s.ports.map((p) => [p.port, p.device, p.none]), [
      [1, "USB Mouse (MicroMys)", false], [2, "USB Gamepad 1", false],
      [4, "GPIO Bank 1", false],
    ]);
    eq(summarise("VIC20", null, "port_1=0\nport_2=1\n").ports.map((p) => [p.port, p.none]),
       [[1, true]]);
    eq(summarise("Pet", null, "port_1=1\n").ports, []);
  });

  test("extras: REU, cartridge, kernal, SID", () => {
    const ini = "[C64]\nREU=1\nREUsize=16384\nCartridgeFile=\"SD:/carts/C64/ide.crt\"\n" +
                "KernalName=\"JiffyDOS_C64.bin\"\nSidModel=1\nSidStereo=1\n";
    eq(summarise("C64", ini, null).extras.map((x) => [x.key, x.value]), [
      ["REU", "16 MB"], ["Cart", "ide.crt"], ["Kernal", "JiffyDOS_C64"],
      ["SID", "8580 ×2"],
    ]);
    eq(summarise("C64", "[C64]\nREU=1\nKernalName=\"kernal\"\n", null).extras
         .map((x) => [x.key, x.value]), [["REU", "512 KB"]], "defaults left out");
  });

  test("extras: C128 40/80 key", () => {
    const screen = (ini) => summarise("C128", ini, null).extras
      .filter((x) => x.key === "Screen").map((x) => x.value);
    eq(screen("[C128]\nC128ColumnKey=0\n"), ["80 columns"], "key down");
    eq(screen("[C128]\nC128ColumnKey=1\n"), ["40 columns"], "key up");
    eq(screen(""), ["40 columns"], "VICE default");
    eq(summarise("C64", "[C64]\nC128ColumnKey=0\n", null).extras, [], "C64");
  });

  test("extras: VIC-20 memory, PET screen", () => {
    eq(summarise("VIC20", "[VIC20]\nRAMBlock0=1\nRAMBlock1=1\nRAMBlock2=1\n", null)
         .extras.map((x) => x.value), ["+19 KB"]);
    eq(summarise("Pet", "[PET]\nVideoSize=80\n", null).extras.map((x) => x.value),
       ["80 columns"]);
  });

  test("known only when a file was read", () => {
    eq(summarise("C64", null, null).known, false);
    eq(summarise("C64", "", null).known, true);
  });

  return { passed, failed, log };
}
