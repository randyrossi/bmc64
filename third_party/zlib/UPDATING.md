# zlib in BMC64

This folder holds only zlib's decompression files, copied unchanged from the
official release. The BMC64 updater (`src/update/`) uses them to unpack
release zips. The files are built with `Z_SOLO` (no `gz*` file functions).

| | |
| --- | --- |
| Version | 1.3.2 |
| Source | <https://github.com/madler/zlib> |
| Release | <https://github.com/madler/zlib/releases/tag/v1.3.2> |
| Archive | <https://github.com/madler/zlib/releases/download/v1.3.2/zlib-1.3.2.tar.gz> |
| SHA-256 | `bb329a0a2cd0274d05519d61c667c062e06990d72e125ee2dfa8de64f0119d16` |
| Licence | zlib licence, see [LICENSE](LICENSE) |

Files: `adler32.c`, `crc32.c`, `crc32.h`, `inffast.c`, `inffast.h`, `inffixed.h`, `inflate.c`, `inflate.h`, `inftrees.c`, `inftrees.h`, `zutil.c`, `zutil.h`, `zlib.h`, `zconf.h`, `LICENSE`, `README`

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
