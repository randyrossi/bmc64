#!/bin/bash

BASE_DIR="$HOME/Library/Application Support/plus4emu"
MAKECFG="`dirname \"$0\"`/plus4emu.app/Contents/MacOS/p4makecfg"

if ( ! [ -e "$BASE_DIR/plus4cfg.dat" ] ) ; then
  "$MAKECFG" -f "$BASE_DIR" ;
else
  "$MAKECFG" "$BASE_DIR" ;
fi

if ( ! [ -e "$BASE_DIR/roms/dos15412.rom" ] ) ; then
  SAVED_CWD="`pwd`"
  mkdir -p "$BASE_DIR/roms" || exit -1
  cd "$BASE_DIR/roms" || exit -1
  curl -o plus4emu_roms.zip http://www.sharemation.com/IstvanV/roms/plus4emu_roms.zip
  unzip -o plus4emu_roms.zip
  rm -f plus4emu_roms.zip
  cd "$SAVED_CWD" ;
fi

