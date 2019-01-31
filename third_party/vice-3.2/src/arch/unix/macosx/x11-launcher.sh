#!/bin/bash

#
# x11-launcher.sh - X11 Application Launcher for the VICE Project
#
# Written by
#  Christian Vogelgsang <chris@vogelgsang.org>
#
# inspired by Gimp.app of Aaron Voisine <aaron@voisine.org>
#
# This file is part of VICE, the Versatile Commodore Emulator.
# See README for copyright notice.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#  02111-1307  USA.
#

# --- debug echo ---
# only available if DEBUG_VICE_LAUNCHER is defined
dbgecho () {
  [ "$DEBUG_VICE_LAUNCHER" != "" ] && echo "$@"
  [ "$LOG_VICE_LAUNCHER" != "" ] && echo "$@" >> $HOME/vice_launcher.log
}

# --- find bundle name and resource dir ---
SCRIPT_DIR="`dirname \"$0\"`"
RESOURCES_DIR="`cd \"$SCRIPT_DIR/../Resources\" && pwd`"
BUNDLE_DIR="`cd \"$SCRIPT_DIR/../..\" && pwd`"
BUNDLE_NAME="`basename \"$BUNDLE_DIR\" .app`"
dbgecho "BUNDLE_DIR=$BUNDLE_DIR"
dbgecho "BUNDLE=$BUNDLE_NAME"
dbgecho "ARGS=""$@"

# --- determine launch environment ---
LAUNCH=cmdline
# finder always appends a -psn_ switch
echo "$1" | grep -e -psn_ > /dev/null
if [ "$?" == "0" ]; then
  LAUNCH=finder
fi
# platypus passes the bundle as in first arg
if [ "$1" = "$BUNDLE_DIR" ]; then
  LAUNCH=platypus
  shift
fi
dbgecho "LAUNCH=$LAUNCH"

# only do the following if no X11 env (i.e. DISPLAY) is defined
# e.g. not required for Mac OS X 10.5 Leopard
CREATED_XINITRC=0
if [ "x$DISPLAY" = "x" ]; then

# --- create a temporary .xinitc if X11 is not running and user has none ---
# check for X11
ps -wx -ocommand | grep X11.app > /dev/null | grep -v grep > /dev/null
if [ "$?" != "0" ]; then
  # if user has no config then create one
  if [ ! -f "$HOME/.xinitrc" ]; then
    DEFAULT_XINITRC="/usr/X11R6/lib/X11/xinit/xinitrc"
    if [ -f "$DEFAULT_XINITRC" ]; then
      # create a one time xinitrc for gimp without an xterm
      sed 's/xterm/# xterm/' "$DEFAULT_XINITRC" > "$HOME/.xinitrc"
      CREATED_XINITRC=1
      dbgecho "created user's .xinitrc"
    fi
  fi
fi

# --- launch x11 if not already here and find out our DISPLAY ---
# create temp 
TMP_DIR="/var/tmp/$UID-$$"
mkdir -p "$TMP_DIR" || exit 1
DISPLAY_RUN="$TMP_DIR/display.run"
DISPLAY_RESULT="$TMP_DIR/display.result"
rm -f "$DISPLAY_RESULT"
# make display emitter command
echo "#!/bin/sh" > "$DISPLAY_RUN"
echo "echo \"\$DISPLAY\" > \"$DISPLAY_RESULT\"" >> "$DISPLAY_RUN"
chmod 755 "$DISPLAY_RUN"
# launch x11 and run display emitter
/usr/bin/open-x11 "$DISPLAY_RUN"
# wait for command
while [ "$?" == "0" -a ! -f "$DISPLAY_RESULT" ]; do sleep 1; done
# fetch display
DISPLAY="`cat \"$DISPLAY_RESULT\"`"
if [ "$DISPLAY" = "" ]; then
  DISPLAY=":0"
fi
dbgecho "DISPLAY=$DISPLAY"
export DISPLAY
# clean up
rm -f "$DISPLAY_RUN" "$DISPLAY_RESULT"
rmdir "$TMP_DIR"

fi # xDISPLAY=x

# --- prepare platypus dropped file args for VICE ---
if [ "$LAUNCH" = "platypus" ]; then
  if [ "$1" != "" ]; then
    LAUNCH_FILE="$1"
  fi
fi

# --- setup environment ---
# setup dylib path
LIB_DIR="$RESOURCES_DIR/lib"
if [ -d "$LIB_DIR" ]; then
  PROGRAM_PREFIX="env DYLD_LIBRARY_PATH=\"$LIB_DIR\""
fi
dbgecho "PROGRAM_PREFIX=$PROGRAM_PREFIX"
# setup path
BIN_DIR="$RESOURCES_DIR/bin"
if [ ! -d "$BIN_DIR" ]; then
  dbgecho "Directory $BIN_DIR not found!"
  exit 1
fi
export PATH="$BIN_DIR:/usr/X11R6/bin:$PATH"

# GTK: setup fontconfig
ETC_DIR="$RESOURCES_DIR/etc"
if [ -d "$ETC_DIR/fonts" ]; then
  export "FONTCONFIG_PATH=$ETC_DIR/fonts"
fi

# --- find VICE binary ---
# derive emu name from bundle name
if [ "x$PROGRAM" = "x" ]; then
  EMUS="x128,x64,x64dtv,x64sc,xcbm2,xcbm5x0,xpet,xplus4,xvic"
  case "$BUNDLE_NAME" in
  x128*)
    PROGRAM=x128
    ;;
  x64*)
    PROGRAM=x64
    ;;
  x64dtv*)
    PROGRAM=x64dtv
    ;;
  x64sc*)
    PROGRAM=x64sc
    ;;
  xcbm2*)
    PROGRAM=xcbm2
    ;;
  xcbm5x0*)
    PROGRAM=xcbm5x0
    ;;
  xpet*)
    PROGRAM=xpet
    ;;
  xplus*)
    PROGRAM=xplus
    ;;
  xvic*)
    PROGRAM=xvic
    ;;
  VICE*)
    # pick emu name in dialog
    PROGRAM=`xmessage -nearmouse -print -buttons "$EMUS" "Please select an Emulator to run:"`
    ;;
  *)
    # invalid bundle name
    xmessage -nearmouse -buttons "Abort" -default "Abort" "Invalid Bundle Name! (use: VICE,$EMUS)"
    PROGRAM=""
    ;;
  esac
fi
dbgecho "PROGRAM=$PROGRAM"
PROGRAM_PATH="$BIN_DIR/$PROGRAM"
dbgecho "PROGRAM_PATH=$PROGRAM_PATH"

# --- now launch the VICE emulator ---
if [ "$LAUNCH" = "cmdline" ]; then
  # launch in cmd line without xterm
  dbgecho "CMDLINE ARGS=""$@"
  $PROGRAM_PREFIX "$PROGRAM_PATH" "$@"
else
  # use xterm as console
  XTERM_BIN=`which xterm`
  if [ "x$XTERM_BIN" != "x" ]; then
    dbgecho "xterm not found!"
    exit 1
  fi
  dbgecho "XTERM LAUNCH_FILE=" "$LAUNCH_FILE"
  if [ "$LAUNCH_FILE" != "" ]; then
    "$XTERM_BIN" \
      -sb -title "VICE $PROGRAM Console" \
      -e $PROGRAM_PREFIX "$PROGRAM_PATH" -autostart "$LAUNCH_FILE"
  else
    "$XTERM_BIN" \
      -sb -title "VICE $PROGRAM Console" \
      -e $PROGRAM_PREFIX "$PROGRAM_PATH"
  fi
fi

# --- clean up ---
# remove temporary .xinitc
if [ $CREATED_XINITRC = 1 ]; then
  rm "$HOME/.xinitrc"
  dbgecho "removed user's .xinitrc"
fi

exit 0

