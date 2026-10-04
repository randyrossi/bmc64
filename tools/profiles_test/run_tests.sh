#!/bin/bash
# Builds the profiles tests with the PC's C compiler and runs them.
# Used by make_all.sh (before the long build) and by the Makefile.

cd "$(dirname "$0")/../.." || exit 1

CC="${PROFILES_TEST_CC:-cc}"
BIN=build/.profiles_test

if ! command -v "$CC" >/dev/null 2>&1
then
       echo "A host C compiler is required for the profiles tests" >&2
       exit 1
fi

mkdir -p build
"$CC" -std=c11 -Wall -Wextra -Werror -Isrc/profiles \
       tools/profiles_test/profiles_test.c src/profiles/*.c -o "$BIN" || exit 1
"./$BIN"
