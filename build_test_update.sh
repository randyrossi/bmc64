#!/bin/bash
#
# build_test_update.sh
#
# BMC64 - A bare metal Commodore emulator for Raspberry Pi
# Copyright (C) 2026 aminch <https://github.com/aminch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# Makes build/bmc64-update.zip from the card staged by build_sdcard.sh, to
# test the updater. Copy it to the top folder of an SD card that runs a build
# with the updater, then boot.
#
#   ./build_test_update.sh              label it one version above this source
#   ./build_test_update.sh v5.1.10      label it with this version (lower =
#                                       downgrade, same = repair)
#
# The zip holds exactly what is in build/sdcard; the version is only a label.
# release/manifest_history.txt is not changed.
#

set -e

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
STAGING_DIR="$SRC_DIR/build/sdcard"
OUT="$SRC_DIR/build/bmc64-update.zip"

if [ ! -d "$STAGING_DIR" ]
then
    echo "No staged card in $STAGING_DIR. Run ./build_sdcard.sh first." >&2
    exit 1
fi

VERSION="$1"
if [ -z "$VERSION" ]
then
    SOURCE_VERSION=$(sed -n 's/^#define VERSION_STRING "\(.*\)"$/\1/p' \
        "$SRC_DIR/third_party/common/menu.c")
    if [[ ! "$SOURCE_VERSION" =~ ^([0-9]+\.[0-9]+\.)([0-9]+)$ ]]
    then
        echo "Can't work out the next version from '$SOURCE_VERSION'; give one, e.g. v5.1.12" >&2
        exit 1
    fi
    VERSION="v${BASH_REMATCH[1]}$((BASH_REMATCH[2] + 1))"
fi

python3 "$SRC_DIR/tools/update/gen_update_manifest.py" package \
    --stage "$STAGING_DIR" --version "$VERSION" --out "$OUT"
python3 "$SRC_DIR/tools/update/gen_update_manifest.py" check "$OUT"

echo "Copy $OUT to the top folder of the SD card (keep the name"
echo "bmc64-update.zip), then boot. The Web UI's Update page also takes it,"
echo "after warning that it doesn't match a release on GitHub."
