// Commodore 64 BASIC V2 <-> PRG conversion, used by the BASIC editor.
//
//   tokenise(text)   listing text -> { bytes } (a PRG), or throws BasicError
//   parsePrg(bytes)  PRG -> { lines, tail }, or null if it is not BASIC
//   buildPrg(lines)  lines -> PRG bytes (the reverse of parsePrg)
//   listing(lines)   lines -> listing text
//
// The tokeniser follows the C64 ROM's own crunch routine: keywords are
// matched in token order at every position outside quotes (so GOTO wins over
// GO, and TOTAL becomes TO + TAL exactly as it does on a real machine), and
// the text after REM, and inside a DATA item, is left alone.
//
// Text is written as the default C64 screen shows it: letters of either case
// give the unshifted PETSCII letters ($41-$5A), and any other byte is written
// {name} or {$xx} (the names are the ones VICE's petcat uses).

export class BasicError extends Error {}

export const LOAD_ADDRESS = 0x0801;
const MAX_LINE_NUMBER = 63999;
const BASIC_END = 0xa000; // BASIC RAM ends here

// Tokens $80..$CB, in token order (matching is first-hit in this order).
const KEYWORDS = [
  "END", "FOR", "NEXT", "DATA", "INPUT#", "INPUT", "DIM", "READ", "LET",
  "GOTO", "RUN", "IF", "RESTORE", "GOSUB", "RETURN", "REM", "STOP", "ON",
  "WAIT", "LOAD", "SAVE", "VERIFY", "DEF", "POKE", "PRINT#", "PRINT", "CONT",
  "LIST", "CLR", "CMD", "SYS", "OPEN", "CLOSE", "GET", "NEW", "TAB(", "TO",
  "FN", "SPC(", "THEN", "NOT", "STEP", "+", "-", "*", "/", "^", "AND", "OR",
  ">", "=", "<", "SGN", "INT", "ABS", "USR", "FRE", "POS", "SQR", "RND",
  "LOG", "EXP", "COS", "SIN", "TAN", "ATN", "PEEK", "LEN", "STR$", "VAL",
  "ASC", "CHR$", "LEFT$", "RIGHT$", "MID$", "GO",
];

const TOKEN_FIRST = 0x80;
const TOKEN_LAST = TOKEN_FIRST + KEYWORDS.length - 1; // $CB
const T_DATA = 0x83;
const T_REM = 0x8f;
const T_PRINT = 0x99;
const PI = 0xff;

// Control characters that have a name. Everything else outside $20-$5F is
// written {$xx}.
const CONTROL_NAMES = {
  0x03: "stop", 0x05: "wht", 0x08: "dish", 0x09: "ensh", 0x0d: "ret",
  0x0e: "swlc", 0x11: "down", 0x12: "rvon", 0x13: "home", 0x14: "del",
  0x1b: "esc", 0x1c: "red", 0x1d: "rght", 0x1e: "grn", 0x1f: "blu",
  0x81: "orng", 0x85: "f1", 0x86: "f3", 0x87: "f5", 0x88: "f7", 0x89: "f2",
  0x8a: "f4", 0x8b: "f6", 0x8c: "f8", 0x8d: "sret", 0x8e: "swuc", 0x90: "blk",
  0x91: "up", 0x92: "rvof", 0x93: "clr", 0x94: "inst", 0x95: "brn",
  0x96: "lred", 0x97: "gry1", 0x98: "gry2", 0x99: "lgrn", 0x9a: "lblu",
  0x9b: "gry3", 0x9c: "pur", 0x9d: "left", 0x9e: "yel", 0x9f: "cyn",
};

// Names accepted when reading, in addition to the ones above (the spellings
// other C64 tools use).
const NAME_ALIASES = {
  cls: 0x93, clear: 0x93, cr: 0x0d, return: 0x0d, right: 0x1d, space: 0x20,
  "rvs on": 0x12, "rvs off": 0x92, "reverse on": 0x12, "reverse off": 0x92,
  white: 0x05, red: 0x1c, green: 0x1e, blue: 0x1f, black: 0x90, orange: 0x81,
  brown: 0x95, cyan: 0x9f, purple: 0x9c, yellow: 0x9e,
};

const MAX_REPEAT = 255;

const NAME_TO_BYTE = new Map(
  Object.entries(CONTROL_NAMES).map(([b, n]) => [n, Number(b)]));
for (const [n, b] of Object.entries(NAME_ALIASES)) NAME_TO_BYTE.set(n, b);

const hex2 = (b) => b.toString(16).padStart(2, "0");

// ---- text -> PRG ----

// One character of listing text as its PETSCII byte, or -1 if it has none.
function charByte(ch) {
  const c = ch.charCodeAt(0);
  if (c >= 0x61 && c <= 0x7a) return c - 0x20;   // a-z -> A-Z
  if (c >= 0x20 && c <= 0x5f) return c;
  switch (ch) {
    case "\t": return 0x20;
    case "£": return 0x5c;
    case "↑": return 0x5e;
    case "←": return 0x5f;
    case "π": return PI;
    default: return -1;
  }
}

// Reads a {...} code starting at text[i] === "{": a name or $xx, optionally
// repeated as {name*N}. Returns [byte, count, next index], or null if it is
// not a valid code.
function readBraceCode(text, i) {
  const end = text.indexOf("}", i);
  if (end < 0) return null;
  let name = text.slice(i + 1, end).trim().toLowerCase();
  let count = 1;
  const rep = /^(.*?)\s*\*\s*(\d+)$/.exec(name);
  if (rep) {
    name = rep[1];
    count = parseInt(rep[2], 10);
    if (count < 1 || count > MAX_REPEAT) return null;
  }
  let b;
  if (/^\$[0-9a-f]{1,2}$/.test(name)) b = parseInt(name.slice(1), 16);
  else if (NAME_TO_BYTE.has(name)) b = NAME_TO_BYTE.get(name);
  else return null;
  return [b, count, end + 1];
}

// Crunches the text of one line (after its number) into token bytes.
function crunch(text, lineNo) {
  const out = [];
  const fail = (msg) => {
    throw new BasicError("BASIC line " + lineNo + ": " + msg);
  };
  let i = 0;
  let inQuote = false;
  let mode = null; // "rem" | "data" while copying text verbatim

  const literal = (ch) => {
    const b = charByte(ch);
    if (b < 0) {
      fail("no C64 character for “" + ch + "”. Use {$xx} for its byte value.");
    }
    out.push(b);
  };

  while (i < text.length) {
    const ch = text[i];

    if (ch === "{") {
      const code = readBraceCode(text, i);
      if (!code) {
      const end = text.indexOf("}", i);
      fail("unknown code “" + text.slice(i, end < 0 ? i + 12 : end + 1) +
           "”. Use a name like {clr} or {down*3}, or {$xx}.");
    }
      for (let n = 0; n < code[1]; n++) out.push(code[0]);
      i = code[2];
      continue;
    }

    if (ch === '"') {
      inQuote = !inQuote;
      out.push(0x22);
      i++;
      continue;
    }

    if (inQuote || mode === "rem" || (mode === "data" && ch !== ":")) {
      literal(ch);
      i++;
      continue;
    }
    mode = null; // a ':' ends a DATA item

    if (ch === "?") {
      out.push(T_PRINT);
      i++;
      continue;
    }

    // Keyword: the first one in token order that matches here.
    let matched = -1;
    for (let k = 0; k < KEYWORDS.length && matched < 0; k++) {
      const kw = KEYWORDS[k];
      if (text.substr(i, kw.length).toUpperCase() === kw) matched = k;
    }
    if (matched >= 0) {
      const tok = TOKEN_FIRST + matched;
      out.push(tok);
      i += KEYWORDS[matched].length;
      if (tok === T_REM) mode = "rem";
      else if (tok === T_DATA) mode = "data";
      continue;
    }

    literal(ch);
    i++;
  }
  return out;
}

// Turns a whole listing into a PRG. Blank lines are ignored, a line that is
// only a number deletes that line, and lines are stored in number order (a
// repeated number replaces the earlier line), as on a real C64.
export function tokenise(text) {
  const byNumber = new Map();
  const rows = text.split(/\r\n|\r|\n/);
  for (let n = 0; n < rows.length; n++) {
    const row = rows[n].replace(/\s+$/, "");
    const m = /^\s*(\d+)\s*(.*)$/.exec(row);
    if (!row.trim()) continue;
    if (!m) {
      throw new BasicError("Line " + (n + 1) + " of the text has no BASIC line number.");
    }
    const num = parseInt(m[1], 10);
    if (num > MAX_LINE_NUMBER) {
      throw new BasicError("Line number " + num + " is too big (the maximum is " +
                           MAX_LINE_NUMBER + ").");
    }
    if (m[2] === "") {
      byNumber.delete(num);
    } else {
      byNumber.set(num, Uint8Array.from(crunch(m[2], num)));
    }
  }

  const lines = Array.from(byNumber.keys()).sort((a, b) => a - b)
    .map((num) => ({ num, data: byNumber.get(num) }));
  const bytes = buildPrg(lines);
  if (LOAD_ADDRESS + bytes.length - 2 > BASIC_END) {
    throw new BasicError("The program is too big for C64 BASIC memory (" +
                         (BASIC_END - LOAD_ADDRESS) + " bytes).");
  }
  return { bytes };
}

// The PRG for `lines` ({ num, data }, data being the tokenised text without
// its terminating 0), with correct next-line links.
export function buildPrg(lines) {
  let size = 2 + 2; // load address, final 00 00 link
  for (const l of lines) size += 4 + l.data.length + 1;
  const out = new Uint8Array(size);
  out[0] = LOAD_ADDRESS & 0xff;
  out[1] = LOAD_ADDRESS >> 8;
  let pos = 2;
  for (const l of lines) {
    const next = LOAD_ADDRESS + (pos - 2) + 4 + l.data.length + 1;
    out[pos++] = next & 0xff;
    out[pos++] = next >> 8;
    out[pos++] = l.num & 0xff;
    out[pos++] = l.num >> 8;
    out.set(l.data, pos);
    pos += l.data.length;
    out[pos++] = 0;
  }
  return out; // the last two bytes are already 00 00
}

// ---- PRG -> text ----

// Splits a PRG into lines, or returns null if it is not BASIC loading at
// $0801. The stored next-line links are not trusted (the C64 recomputes
// them after LOAD): a line runs to its 0 byte, and the program ends at a
// link whose high byte is 0. Bytes after that end are returned as `tail`
// (often machine code that the BASIC line SYSes to).
export function parsePrg(bytes) {
  if (bytes.length < 4 || bytes[0] !== (LOAD_ADDRESS & 0xff) ||
      bytes[1] !== (LOAD_ADDRESS >> 8)) return null;
  const lines = [];
  let pos = 2;
  for (;;) {
    if (pos + 1 >= bytes.length) return null;  // no end-of-program link
    if (bytes[pos + 1] === 0) {                // end of program
      return { lines, tail: bytes.subarray(pos + 2) };
    }
    if (pos + 4 >= bytes.length) return null;
    const num = bytes[pos + 2] | (bytes[pos + 3] << 8);
    let end = pos + 4;
    while (end < bytes.length && bytes[end] !== 0) end++;
    if (end >= bytes.length) return null;      // line without a terminator
    lines.push({ num, data: bytes.subarray(pos + 4, end) });
    pos = end + 1;
  }
}

function byteText(b) {
  if (b >= 0x20 && b <= 0x5f) {
    return b === 0x5c ? "£" : String.fromCharCode(b);
  }
  if (b === PI) return "π";
  const name = CONTROL_NAMES[b];
  return "{" + (name || "$" + hex2(b)) + "}";
}

// The listing text of one line's tokenised data.
function detokenise(data) {
  let out = "";
  let inQuote = false;
  let mode = null;
  for (const b of data) {
    if (b === 0x22) {
      inQuote = !inQuote;
      out += '"';
    } else if (inQuote || mode === "rem" || (mode === "data" && b !== 0x3a)) {
      out += byteText(b);
    } else if (b >= TOKEN_FIRST && b <= TOKEN_LAST) {
      out += KEYWORDS[b - TOKEN_FIRST];
      if (b === T_REM) mode = "rem";
      else if (b === T_DATA) mode = "data";
    } else {
      mode = null;
      out += byteText(b);
    }
  }
  return out;
}

// Three or more of the same {code} in a row are written {code*N}. A literal
// "{" is never in the text (byte $7B is written {$7b}), so this only ever
// sees codes.
function collapseRuns(text) {
  return text.replace(/(\{[^{}*]+\})\1{2,}/g,
    (run, one) => one.slice(0, -1) + "*" + run.length / one.length + "}");
}

export function listing(lines) {
  return lines.map((l) => l.num + " " + collapseRuns(detokenise(l.data)))
    .join("\n") + (lines.length ? "\n" : "");
}

// True if turning `parsed` (from parsePrg) into a listing and back gives the
// same program, so editing it can't lose anything but the bytes in `tail`.
// Programs that were not made by a normal tokeniser (keywords spelled out in
// letters, unsorted or empty lines) come out false.
export function isExact(parsed) {
  try {
    const again = tokenise(listing(parsed.lines)).bytes;
    const orig = buildPrg(parsed.lines);
    return again.length === orig.length && again.every((b, i) => b === orig[i]);
  } catch (e) {
    return false;
  }
}
