#!/usr/bin/env python3
"""Regenerate basic_vectors.js, the reference data for basic.test.js.

Only needed when adding test cases; the tests themselves need neither petcat
nor a ROM. It takes the expected bytes from VICE's petcat and the token list
from the C64 BASIC ROM's own keyword table ($A09E).

    python3 tools/webui_test/gen_basic_vectors.py [--rom /usr/share/vice/C64/basic]

petcat differs from a real C64 in a few ways, so each listing is first
normalised the way the C64 would store it: lines sorted, a repeated number
replaces the earlier line, and a bare line number deletes that line. petcat
also keeps ? as it is instead of storing PRINT, and has no {name*N}, so a case
can give an alternative text for petcat with those written out.
"""

import argparse
import json
import os
import random
import re
import subprocess
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))

CASES = [
    ("print and goto", '10 print "hello"\n20 goto 10\n', None),
    # petcat treats capitals as shifted letters; the editor does not, so
    # the reference for mixed case is petcat's lower case.
    ("mixed case text", '10 print "Hello World"\n', '10 print "hello world"\n'),
    ("operators and tokens after then", '10 x=total>=3:if a<>b then print"x":go to 10\n', None),
    ("data and rem are not tokenised", '10 data 1,2,print,"a:b":rem print here\n20 data print:print "x"\n', None),
    ("for next gosub", '10 for i=1 to 10 step 2:next i\n30 gosub 100:return\n', None),
    ("file commands", '10 open 1,8,15,"s0:x":print#1,"a":input#1,a$:close 1\n', None),
    ("poke sys peek and or", '10 poke 53280,0:sys 49152:x=peek(1)and 3 or 4\n', None),
    ("string functions", '10 a$=left$(b$,3)+mid$(c$,2,1)+right$(d$,1)+str$(1)+chr$(65)\n', None),
    ("tab spc fn on goto", '10 tab(5)spc(3)fn a(x)\n20 on x goto 1,2,3\n', None),
    ("rem and data edge cases", '10 rem\n20 rem :::\n30 dataa\n', None),
    ("unclosed and doubled quotes", '10 print "unclosed\n20 print "a"b"c"\n', None),
    ("keywords inside variable names", '10 orders=1:format=2:gotox=3:total=4\n', None),
    ("lines are sorted, a repeated number replaces", '30 print 3\n10 print 1\n20 print 2\n10 print 99\n', None),
    ("a bare line number deletes the line", '10 print 1\n20 print 2\n20\n', None),
    ("? is PRINT", '10 ?"x":?chr$(1)\n', '10 print"x":printchr$(1)\n'),
    # A repeat code in a string assigned to a variable (petcat has no
    # {name*N}, so its reference is the codes written out individually).
    ("a repeated code in a string variable", '10 x$="{home}{right*39}{down*24}"\n',
     '10 x$="{home}' + "{rght}" * 39 + "{down}" * 24 + '"\n'),
    # A line whose only content is a colon (an empty statement).
    ("a line that is only a colon", '10 print 1\n11 :\n12 print 2\n', None),
    ("a negative step in a for loop", '10 for i=7 to 0 step -1:next\n', None),
]

KW = ["end","for","next","data","input#","input","dim","read","let","goto","run","if","restore","gosub","return","rem","stop","on","wait","load","save","verify","def","poke","print#","print","cont","list","clr","cmd","sys","open","close","get","new","tab(","to","fn","spc(","then","not","step","and","or","sgn","int","abs","usr","fre","pos","sqr","rnd","log","exp","cos","sin","tan","atn","peek","len","str$","val","asc","chr$","left$","right$","mid$","go"]
OPS = list("+-*/^<>=(),;:#$%") + ["<>", ">=", "<="]


def rnd_line(rng):
    parts = []
    for _ in range(rng.randint(1, 8)):
        r = rng.random()
        if r < .35: parts.append(rng.choice(KW))
        elif r < .5: parts.append('"' + "".join(rng.choice("abcdefghijklmnopqrstuvwxyz 0123456789:,;+-*/<>=?!#$%&'()") for _ in range(rng.randint(0, 12))) + '"')
        elif r < .65: parts.append(rng.choice(OPS))
        elif r < .8: parts.append("".join(rng.choice("abcdefghijklmnopqrstuvwxyz") for _ in range(rng.randint(1, 6))))
        elif r < .9: parts.append(str(rng.randint(0, 9999)))
        else: parts.append(rng.choice([" ", "  ", ":"]))
    return "".join(parts)


def random_cases(count=30):
    rng = random.Random(7)
    for i in range(count):
        n = (1, 2, 5)[i % 3]
        text = "".join("%d %s\n" % (10 * (j + 1), rnd_line(rng)) for j in range(n))
        yield "random %d" % (i + 1), text, None


def normalise(text):
    """The program as a C64 would hold it, for petcat to tokenise."""
    lines = {}
    for row in text.splitlines():
        m = re.match(r"\s*(\d+)\s*(.*)$", row.rstrip())
        if not m:
            continue
        if m.group(2) == "":
            lines.pop(int(m.group(1)), None)
        else:
            lines[int(m.group(1))] = m.group(2)
    return "".join("%d %s\n" % (n, lines[n]) for n in sorted(lines))


def petcat(text, exe):
    with tempfile.TemporaryDirectory() as d:
        src, dst = os.path.join(d, "a.txt"), os.path.join(d, "a.prg")
        with open(src, "w") as f:
            f.write(text)
        subprocess.run([exe, "-w2", "-o", dst, "--", src], check=True, capture_output=True)
        with open(dst, "rb") as f:
            return f.read().hex()


def rom_keywords(path):
    with open(path, "rb") as f:
        rom = f.read()
    off, words, cur = 0x9E, [], ""
    while rom[off] != 0:
        cur += chr(rom[off] & 0x7F)
        if rom[off] & 0x80:
            words.append(cur)
            cur = ""
        off += 1
    return words


def find_rom():
    for d in ("/usr/share/vice", "/usr/lib/vice"):
        p = os.path.join(d, "C64", "basic")
        if os.path.exists(p):
            return p
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rom", default=find_rom(), help="C64 BASIC ROM image (8192 bytes)")
    ap.add_argument("--petcat", default="petcat")
    args = ap.parse_args()
    if not args.rom:
        ap.error("no C64 BASIC ROM found; pass --rom")

    golden = []
    for name, text, alt in list(CASES) + list(random_cases()):
        golden.append({"name": name, "listing": text,
                       "hex": petcat(normalise(alt or text), args.petcat)})
    words = rom_keywords(args.rom)
    assert len(words) == 76, words

    out = os.path.join(HERE, "basic_vectors.js")
    with open(out, "w") as f:
        f.write("// Reference data for basic.test.js. Generated by gen_basic_vectors.py; do not edit.\n\n")
        f.write("// Expected PRG bytes (hex) for each listing, from VICE's petcat (after the\n")
        f.write("// listing is put in the form a real C64 would store it).\n")
        f.write("export const GOLDEN = " + json.dumps(golden, indent=1) + ";\n\n")
        f.write("// The keywords of the C64 BASIC ROM, in token order from $80 (its own table at $A09E).\n")
        f.write("export const ROM_KEYWORDS = " + json.dumps(words) + ";\n")
    print("wrote", out, "-", len(golden), "programs,", len(words), "keywords")


if __name__ == "__main__":
    main()
