#!/usr/bin/env python3
"""Copy the parts of zlib that BMC64 uses into third_party/zlib/.

BMC64 uses only zlib's decompression (the updater, src/update/). This script
is the only way those files get into the tree: it downloads the official
release from https://github.com/madler/zlib, checks it against the SHA-256
below, and copies the listed files unchanged, with zlib's LICENSE and README.
It also writes third_party/zlib/UPDATING.md, which explains how to update.
"""

import hashlib
import os
import shutil
import sys
import tarfile
import tempfile
import urllib.request

VERSION = "1.3.2"
SHA256 = "bb329a0a2cd0274d05519d61c667c062e06990d72e125ee2dfa8de64f0119d16"
URL = ("https://github.com/madler/zlib/releases/download/v{0}/zlib-{0}.tar.gz"
       .format(VERSION))

# Decompression only (inflate), built with Z_SOLO (no gz* file functions).
FILES = [
    "adler32.c", "crc32.c", "crc32.h", "inffast.c", "inffast.h", "inffixed.h",
    "inflate.c", "inflate.h", "inftrees.c", "inftrees.h", "zutil.c", "zutil.h",
    "zlib.h", "zconf.h", "LICENSE", "README",
]

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEST = os.path.join(REPO, "third_party", "zlib")

NOTE = """# zlib in BMC64

This folder holds only zlib's decompression files, copied unchanged from the
official release. The BMC64 updater (`src/update/`) uses them to unpack
release zips. The files are built with `Z_SOLO` (no `gz*` file functions).

| | |
| --- | --- |
| Version | {version} |
| Source | <https://github.com/madler/zlib> |
| Release | <https://github.com/madler/zlib/releases/tag/v{version}> |
| Archive | <{url}> |
| SHA-256 | `{sha}` |
| Licence | zlib licence, see [LICENSE](LICENSE) |

Files: {files}

## Updating

Don't edit these files. They are copied by `tools/vendor_zlib.py`, which
downloads the release archive, checks its SHA-256 and replaces this folder.

1. In `tools/vendor_zlib.py`, set `VERSION` to the new release and `SHA256`
   to its archive's SHA-256. Take the hash from the release page
   (<https://github.com/madler/zlib/releases>) and check that the same file on
   <https://zlib.net> has the same hash.
2. Run `python3 tools/vendor_zlib.py`.
3. If the release adds or renames a decompression file, update `FILES` in the
   script and the zlib objects in the top-level `Makefile` and in
   `tools/update/test/run_tests.py`.
4. Run the updater tests (`python3 tools/update/test/run_tests.py`) and a
   build, then commit this folder and the script together.
5. Update the version in `docs/THIRD_PARTY.md`.
"""


def main():
    with tempfile.TemporaryDirectory() as tmp:
        archive = os.path.join(tmp, "zlib.tar.gz")
        print("downloading " + URL)
        with urllib.request.urlopen(URL) as r, open(archive, "wb") as f:
            shutil.copyfileobj(r, f)
        with open(archive, "rb") as f:
            digest = hashlib.sha256(f.read()).hexdigest()
        if digest != SHA256:
            sys.exit("SHA-256 mismatch: got %s, expected %s" % (digest, SHA256))

        prefix = "zlib-%s/" % VERSION
        with tarfile.open(archive) as tar:
            members = {m.name: m for m in tar.getmembers()}
            missing = [n for n in FILES if prefix + n not in members]
            if missing:
                sys.exit("not in this release: " + ", ".join(missing))
            shutil.rmtree(DEST, ignore_errors=True)
            os.makedirs(DEST)
            for name in FILES:
                data = tar.extractfile(members[prefix + name]).read()
                with open(os.path.join(DEST, name), "wb") as out:
                    out.write(data)

    with open(os.path.join(DEST, "UPDATING.md"), "w", newline="\n") as f:
        f.write(NOTE.format(version=VERSION, url=URL, sha=SHA256,
                            files=", ".join("`%s`" % n for n in FILES)))
    print("copied %d files to %s" % (len(FILES), os.path.relpath(DEST, REPO)))


if __name__ == "__main__":
    main()
