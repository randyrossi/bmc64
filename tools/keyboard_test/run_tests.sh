#!/bin/bash
# Builds the keyboard tests with the PC's C compiler and runs them.
# Used by make_all.sh (before the long build) and by the Makefile.

cd "$(dirname "$0")/../.." || exit 1

CC="${KEYBOARD_TEST_CC:-cc}"
BIN=build/.keyboard_router_test

if ! command -v "$CC" >/dev/null 2>&1
then
       echo "A host C compiler is required for the keyboard tests" >&2
       exit 1
fi

mkdir -p build
"$CC" -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter \
       -ffunction-sections -fdata-sections -DBMC64_KEYBOARD_FEATURE_H \
       -DBMC64_NEW_KEYBOARD_INPUT=1 -Ithird_party/common \
       -Ithird_party/circle-stdlib/include \
       -Ithird_party/circle-stdlib/libs/circle/include \
       -Wl,--gc-sections tools/keyboard_test/keyboard_router_test.c \
       src/keyboard/keyboard_router.c third_party/common/kbd.c \
       src/keyboard/keyboard_layout.c src/keyboard/layout_*.c -o "$BIN" || exit 1
"./$BIN"
