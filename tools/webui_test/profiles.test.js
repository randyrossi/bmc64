// Tests for reading the profile files in the web UI
// (src/webui/assets/js/profiles_data.js), against the same rules as the
// device's parser (src/profiles, tools/profiles_test).
//
// A plain ES module with no dependencies. run_tests.mjs runs it under Node.js;
// runTests() returns { passed, failed, log }.

import {
  activeWithout, cleanName, groupProfiles, idValid, machineDetail, machineLabel,
  parseActive, parseKv, parseMainFile, parseProfile, PROFILE_FILES, settingsFile,
  withName,
} from "../../src/webui/assets/js/profiles_data.js";

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

  test("key=value lines", () => {
    const kv = parseKv("  name =  My Game  \n# name=comment\nnot a pair\n=x\n" +
                       "machine=C64\r\nname=Last\n");
    eq(kv.get("name"), "Last", "last repeated key wins");
    eq(kv.get("machine"), "C64", "CRLF");
    eq(kv.has("# name"), false, "comment");
    eq(kv.size, 2);
  });

  test("ids", () => {
    eq(idValid("geos-2"), true);
    eq(idValid("GEOS"), false);
    eq(idValid("my game"), false);
    eq(idValid(""), false);
    eq(idValid("a".repeat(32)), true);
    eq(idValid("a".repeat(33)), false);
    eq(idValid(undefined), false);
  });

  test("profile.txt", () => {
    const p = parseProfile("elite", "name=Elite\nmachine=C64/PAL/HDMI\n" +
      "category=Games\nstart=once\ndisk_9=SD:/games/save.d64\n" +
      "disk_08=SD:/x.d64\ndisk_12=SD:/y.d64\nautostart=SD:/games/elite.d64\n");
    eq(p.name, "Elite");
    eq(p.machine, "C64/PAL/HDMI");
    eq(p.category, "Games");
    eq(p.start, "once");
    eq(p.disks, ["", "SD:/games/save.d64", "", ""], "only disk_8..disk_11");
    eq(p.autostart, "SD:/games/elite.d64");
    eq(parseProfile("x", "name=x\nmachine=C64\nstart=Once\n").start, "switch");
  });

  test("invalid profiles", () => {
    eq(parseProfile("a", "machine=C64\n"), null, "no name");
    eq(parseProfile("a", "name=A\n"), null, "no machine");
    eq(parseProfile("UPPER", "name=A\nmachine=C64\n"), null, "bad id");
    eq(parseProfile("main", "name=A\nmachine=C64\n"), null, "main isn't a profile");
    eq(parseProfile("a", "name=" + "n".repeat(40) + "\nmachine=C64\n").name.length,
       32, "names cut to 32");
  });

  test("active.txt", () => {
    eq(parseActive("profile=geos\nonce=elite\nmain_machine=C64/PAL/HDMI\n"),
       { profile: "geos", once: "elite", mainMachine: "C64/PAL/HDMI" });
    eq(parseActive(""), { profile: "main", once: "", mainMachine: "" }, "empty");
    eq(parseActive("profile=GEOS\nonce=Elite!\n"),
       { profile: "main", once: "", mainMachine: "" }, "typos are ignored");
  });

  test("Main's file", () => {
    eq(parseMainFile("disk_8=SD:/a.d64\nautostart=SD:/b.prg\nname=Sneaky\n"),
       { disks: ["SD:/a.d64", "", "", ""], autostart: "SD:/b.prg" });
  });

  test("machine labels", () => {
    eq(machineLabel("C64/PAL/HDMI/VICE 720p@50Hz"), "C64");
    eq(machineLabel("vic20"), "VIC-20");
    eq(machineLabel("Plus4Emu/PAL"), "Plus/4");
    eq(machineLabel("Pet/NTSC"), "PET");
    eq(machineLabel(""), "");
    eq(machineDetail("C64 / PAL / HDMI / VICE 720p@50Hz"), "PAL · HDMI · VICE 720p@50Hz");
    eq(machineDetail("C64/PAL/DPI/Gert 666/RGB"), "PAL · DPI · Gert 666/RGB");
    eq(machineDetail("C64"), "");
  });

  test("grouping", () => {
    const p = (name, category) => ({ name, category });
    const groups = groupProfiles([p("zork", "Games"), p("Elite", "Games"),
                                  p("Jiffy", ""), p("basic", ""), p("GEOS", "Apps")]);
    // Categories in the order their first profile sorts, as in the menu.
    eq(groups.map((g) => g.category), ["", "Games", "Apps"]);
    eq(groups[0].profiles.map((x) => x.name), ["basic", "Jiffy"]);
    eq(groups[1].profiles.map((x) => x.name), ["Elite", "zork"]);
    eq(groupProfiles([]), []);
  });

  test("names", () => {
    eq(cleanName("  My GEOS  "), "My GEOS");
    eq(cleanName("   "), "");
    eq(cleanName(null), "");
    eq(cleanName("a\nb"), "a b", "one line");
    eq(cleanName("n".repeat(40)).length, 32, "cut to 32");
    eq(cleanName("x".repeat(31) + "  y"), "x".repeat(31), "trimmed after cutting");
  });

  test("rename keeps the rest of profile.txt", () => {
    eq(withName("# mine\nname=Old\nmachine=C64\nname=Older\n", "New"),
       "# mine\nname=New\nmachine=C64\n", "repeats removed");
    eq(withName("  name = Old\r\nmachine=C64\r\n", "New"),
       "name=New\r\nmachine=C64\r\n", "CRLF kept");
    eq(withName("machine=C64", "New"), "name=New\nmachine=C64\n", "no name yet");
    const back = parseProfile("x", withName("name=Old\nmachine=C64\n", "New"));
    eq(back.name, "New");
  });

  test("active.txt after a delete", () => {
    eq(activeWithout("profile=geos\nonce=geos\nmain_machine=C64\n", "geos"),
       "profile=main\nmain_machine=C64\n");
    eq(activeWithout("profile=elite\nonce=geos\n", "geos"), "profile=elite\n");
    eq(activeWithout("profile=elite\n", "geos"), null, "not mentioned");
    eq(activeWithout("profile=geos-2\n", "geos"), null, "only that id");
    eq(activeWithout("profile = geos \r\n", "geos"), "profile=main\r\n");
  });

  test("settings files", () => {
    eq(settingsFile("C64/PAL/HDMI"), "settings-c64.txt");
    eq(settingsFile("Plus4Emu"), "settings-plus4emu.txt");
    eq(settingsFile("VIC20 / PAL"), "settings-vic20.txt");
    eq(settingsFile(""), "settings.txt");
    for (const f of ["settings.txt", "settings-plus4.txt", "settings-pet.txt.bak",
                     "vice.in~", "profile.txt.bak"]) {
      eq(PROFILE_FILES.includes(f), true, f);
    }
    eq(PROFILE_FILES.includes("my-notes.txt"), false);
  });

  return { passed, failed, log };
}
