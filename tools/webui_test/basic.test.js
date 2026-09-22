// Tests for the web UI's C64 BASIC tokeniser (src/webui/assets/js/basic.js).
//
// A plain ES module with no dependencies. run_tests.mjs runs it under Node.js;
// runTests() returns { passed, failed, log }.

import {
  BasicError, buildPrg, isExact, listing, parsePrg, tokenise,
} from "../../src/webui/assets/js/basic.js";
import { GOLDEN, ROM_KEYWORDS } from "./basic_vectors.js";

const hex = (u) => Array.from(u, (b) => b.toString(16).padStart(2, "0")).join("");
const bytesOf = (text) => tokenise(text).bytes;

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
    if (got !== want) {
      throw new Error((what ? what + ": " : "") + "got  " + got + "\nwant " + want);
    }
  }
  function throwsBasicError(fn, wantText) {
    try {
      fn();
    } catch (e) {
      if (!(e instanceof BasicError)) throw e;
      if (wantText && !e.message.includes(wantText)) {
        throw new Error("message “" + e.message + "” lacks “" + wantText + "”");
      }
      return;
    }
    throw new Error("no BasicError thrown");
  }

  // ---- reference data ----

  for (const g of GOLDEN) {
    test("matches petcat: " + g.name, () => {
      eq(hex(bytesOf(g.listing)), g.hex);
    });
    test("upper case text is the same: " + g.name, () => {
      eq(hex(bytesOf(g.listing.toUpperCase())), g.hex);
    });
    test("PRG -> listing -> PRG is exact: " + g.name, () => {
      const parsed = parsePrg(Uint8Array.from(g.hex.match(/../g), (h) => parseInt(h, 16)));
      eq(hex(bytesOf(listing(parsed.lines))), g.hex);
      eq(isExact(parsed), true, "isExact");
    });
  }

  test("ROM has 76 keywords", () => eq(ROM_KEYWORDS.length, 76));
  ROM_KEYWORDS.forEach((kw, i) => {
    const token = 0x80 + i;
    test("token $" + token.toString(16) + " is " + kw, () => {
      eq(hex(parsePrg(bytesOf("10 " + kw)).lines[0].data), hex([token]), "tokenise");
      const line = { num: 10, data: Uint8Array.of(token) };
      eq(listing([line]), "10 " + kw + "\n", "listing");
    });
  });

  // ---- format ----

  test("10 PRINT \"HI\" is the expected 15 bytes", () => {
    eq(hex(bytesOf('10 PRINT "HI"')), "01080c080a00992022484922000000");
  });

  test("an empty listing is a load address and an end link", () => {
    eq(hex(bytesOf("")), "01080000");
    eq(hex(bytesOf("\n \n")), "01080000");
  });

  test("buildPrg links each line to the next", () => {
    const prg = buildPrg([
      { num: 1, data: Uint8Array.of(0x80) },
      { num: 2, data: Uint8Array.of(0x80, 0x80) },
    ]);
    eq(hex(prg), "0108" + "0708" + "0100" + "80" + "00" + "0e08" + "0200" + "8080" + "00" + "0000");
  });

  // ---- tokenising rules ----

  test("? is stored as PRINT", () => {
    eq(hex(parsePrg(bytesOf('10 ?"X":?#1')).lines[0].data), "99225822" + "3a992331");
  });

  test("keywords match in token order, so GOTO wins over GO and TOTAL is TO+TAL", () => {
    eq(hex(parsePrg(bytesOf("10 goto")).lines[0].data), "89");
    eq(hex(parsePrg(bytesOf("10 go to")).lines[0].data), "cb20a4");
    eq(hex(parsePrg(bytesOf("10 total")).lines[0].data), "a454414c");
  });

  test("nothing is tokenised in quotes, after REM, or in a DATA item", () => {
    eq(hex(parsePrg(bytesOf('10 print "goto"')).lines[0].data), "992022" + "474f544f" + "22");
    eq(hex(parsePrg(bytesOf("10 rem goto")).lines[0].data), "8f20474f544f");
    eq(hex(parsePrg(bytesOf("10 data goto:goto")).lines[0].data), "8320474f544f3a89");
  });

  test("a colon inside quotes does not end a DATA item", () => {
    eq(hex(parsePrg(bytesOf('10 data "a:goto":goto')).lines[0].data), "832022" + "41" + "3a474f544f" + "22" + "3a89");
  });

  test("lines are sorted, a repeated number replaces, a bare number deletes", () => {
    const lines = parsePrg(bytesOf("30 end\n10 end\n20 end\n10 stop\n30\n")).lines;
    eq(lines.map((l) => l.num + ":" + hex(l.data)).join(" "), "10:90 20:80");
  });

  test("spaces after the line number are dropped, others kept", () => {
    eq(hex(parsePrg(bytesOf("10    print  1")).lines[0].data), "99202031");
  });

  // ---- {codes} ----

  test("{names}, {$xx} and repeats", () => {
    eq(hex(parsePrg(bytesOf('10 print "{clr}{home}{red}{$c1}{down*3}{right*2}"')).lines[0].data),
       "992022" + "93" + "13" + "1c" + "c1" + "111111" + "1d1d" + "22");
  });

  test("names are case-insensitive and other tools' spellings work", () => {
    eq(hex(parsePrg(bytesOf('10 ?"{CLR}{Rvs On}{white}{CYAN}{rvs off}"')).lines[0].data),
       "99" + "22" + "93" + "12" + "05" + "9f" + "92" + "22");
  });

  test("three or more of a code are listed as {code*N}, two are not", () => {
    const line = { num: 10, data: Uint8Array.of(0x22, 0x11, 0x11, 0x11, 0x1d, 0x1d, 0x7b, 0x7b, 0x7b, 0x22) };
    eq(listing([line]), '10 "{down*3}{rght}{rght}{$7b*3}"\n');
  });

  test("every byte in a string survives listing and back", () => {
    const data = [0x99, 0x22];
    for (let b = 1; b < 256; b++) if (b !== 0x22) data.push(b);
    data.push(0x22);
    const prg = buildPrg([{ num: 10, data: Uint8Array.from(data) }]);
    eq(hex(bytesOf(listing(parsePrg(prg).lines))), hex(prg));
  });

  test("£ ↑ ← π and the ASCII equivalents", () => {
    eq(hex(parsePrg(bytesOf('10 print "£↑←"')).lines[0].data), "992022" + "5c5e5f" + "22");
    eq(hex(parsePrg(bytesOf("10 x=π")).lines[0].data), "58b2ff");
  });

  // ---- errors ----

  test("errors", () => {
    throwsBasicError(() => tokenise("print 1"), "no BASIC line number");
    throwsBasicError(() => tokenise("10 print {nope}"), "{nope}");
    throwsBasicError(() => tokenise('10 print "{down*0}"'), "{down*0}");
    throwsBasicError(() => tokenise('10 print "{down*256}"'), "{down*256}");
    throwsBasicError(() => tokenise('10 print "{down*x}"'), "{down*x}");
    throwsBasicError(() => tokenise("10 print |"), "|");
    throwsBasicError(() => tokenise("64000 end"), "too big");
    eq(hex(parsePrg(bytesOf("63999 end")).lines[0].data), "80");
  });

  test("a program too big for BASIC memory is refused", () => {
    const rows = [];
    for (let i = 0; i < 2000; i++) rows.push((i + 1) + ' print "' + "x".repeat(30) + '"');
    throwsBasicError(() => tokenise(rows.join("\n")), "too big");
  });

  // ---- reading PRGs ----

  test("parsePrg: not BASIC", () => {
    eq(parsePrg(Uint8Array.of(0x00, 0xc0, 1, 2, 3)), null, "wrong load address");
    eq(parsePrg(Uint8Array.of(0x01, 0x08)), null, "too short");
    eq(parsePrg(Uint8Array.of(0x01, 0x08, 0x09, 0x08, 0x0a, 0x00, 0x80)), null, "no line end");
    eq(parsePrg(Uint8Array.of(0x01, 0x08, 0x09, 0x08, 0x0a, 0x00, 0x80, 0x00)), null, "no end link");
  });

  test("parsePrg: bytes after the program are the tail", () => {
    const p = parsePrg(Uint8Array.of(1, 8, 0x0c, 8, 10, 0, 0x9e, 0x32, 0x30, 0x36, 0x34, 0, 0, 0, 0xea, 0x60));
    eq(hex(p.tail), "ea60");
    eq(listing(p.lines), "10 SYS2064\n");
  });

  test("parsePrg ignores wrong next-line links, as the C64 does after LOAD", () => {
    const p = parsePrg(Uint8Array.of(1, 8, 0xff, 0xff, 10, 0, 0x80, 0, 0, 0));
    eq(listing(p.lines), "10 END\n");
    eq(isExact(p), true);
  });

  test("isExact is false when the listing would not give the same program", () => {
    // "PRINT" spelled out in letters instead of the token
    const spelled = parsePrg(Uint8Array.of(1, 8, 0x0a, 8, 10, 0, 0x50, 0x52, 0x49, 0x4e, 0x54, 0, 0, 0));
    eq(isExact(spelled), false, "spelled-out keyword");
    // out of order lines
    const unsorted = parsePrg(buildPrg([
      { num: 20, data: Uint8Array.of(0x80) }, { num: 10, data: Uint8Array.of(0x80) }]));
    eq(isExact(unsorted), false, "unsorted lines");
    // an empty line would be deleted
    eq(isExact(parsePrg(buildPrg([{ num: 10, data: new Uint8Array(0) }]))), false, "empty line");
  });

  return { passed, failed, log };
}
