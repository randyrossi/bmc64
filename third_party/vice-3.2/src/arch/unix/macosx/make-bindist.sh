#!/bin/bash

#
# make-bindist.sh - make binary distribution for the Mac OSX port
#
# Written by
#  Christian Vogelgsang <chris@vogelgsang.org>
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
# Usage: make-bindist.sh <top_srcdir> <strip> <vice-version> <--enable-arch> <zip|nozip> <x64sc-included> <ui_type> [bin_format]
#                         $1           $2      $3             $4              $5          $6               $7        $8
#

RUN_PATH=`dirname $0`

echo "Generating Mac OSX binary distribution."

TOP_DIR=$1
STRIP=$2
VICE_VERSION=$3
ENABLEARCH=$4
ZIP=$5
X64SC=$6
UI_TYPE=$7
BIN_FORMAT=$8

# ui type
if [ "x$UI_TYPE" = "x" ]; then
  UI_TYPE="x11"
fi
echo "  ui type: $UI_TYPE"

# check binary type
TEST_BIN=src/x64
if [ ! -x $TEST_BIN ]; then
  echo "error missing binary $TEST_BIN"
  exit 1
fi
if [ x"$BIN_FORMAT" = "x" ]; then
  BIN_TYPE=`file $TEST_BIN | grep "$TEST_BIN:" | sed -e 's/executable//g' -e 's/Mach-O//g' -e 's/64-bit//g' | awk '{print $2}'`
  if [ x"$BIN_TYPE" = "xi386" ]; then
    BIN_FORMAT=i386
  elif [ x"$BIN_TYPE" = "xx86_64" ]; then
    BIN_FORMAT=x86_64
  elif [ x"$BIN_TYPE" = "xppc" ]; then
    BIN_FORMAT=ppc
  else
    echo "fatal: unknown bin type '$BIN_TYPE'"
    exit 1
  fi
fi
echo "  binary format: $BIN_FORMAT"

# setup BUILD dir
if [ x"$SDK_TAG" != "x" ]; then
  BUILD_DIR=vice-macosx-$UI_TYPE-$BIN_FORMAT-$SDK_TAG-$VICE_VERSION
else
  BUILD_DIR=vice-macosx-$UI_TYPE-$BIN_FORMAT-$VICE_VERSION
fi
if [ -d $BUILD_DIR ]; then
  rm -rf $BUILD_DIR
fi
mkdir $BUILD_DIR
if [ ! -d $BUILD_DIR ]; then
  echo "error creating directory $BUILD_DIR"
  exit 1
fi

# make tools dir
TOOL_DIR=$BUILD_DIR/tools
if [ ! -d $TOOL_DIR ]; then
  mkdir $TOOL_DIR
fi

if test x"$X64SC" = "xyes"; then
  SCFILE="x64sc"
else
  SCFILE=""
fi

# define emulators and command line tools
EMULATORS="x64 xscpu64 x64dtv $SCFILE x128 xcbm2 xcbm5x0 xpet xplus4 xvic"
TOOLS="c1541 petcat cartconv"

# define data files for emulators
ROM_COMMON="DRIVES PRINTER"
ROM_x64=C64
ROM_xscpu64=SCPU64
ROM_x64sc=C64
ROM_x64dtv=C64DTV
ROM_x128=C128
ROM_xcbm2=CBM-II
ROM_xcbm5x0=CBM-II
ROM_xpet=PET
ROM_xplus4=PLUS4
ROM_xvic=VIC20
# files to remove from ROM directory
ROM_REMOVE="{beos,amiga,dos,win,RO}*.vkm"
DOC_REMOVE="Makefile.* *.c *.mak *.sh *.tex *.texi *.pl *.chm *.guide *.hlp *.inf building readmes"
# define droppable file types
DROP_TYPES="x64|p64|g64|d64|d71|d81|t64|tap|prg|p00|crt|reu"
DROP_FORMATS="x64 p64 g64 d64 d71 d81 t64 tap prg p00 crt reu"

# launcher script
LAUNCHER=x11-launcher.sh

# multi apps
MULTI_APPS=0
if [ "$UI_TYPE" = "cocoa" -o "$UI_TYPE" = "sdl" ]; then
  MULTI_APPS=1
fi

# use platypus or launcher directly
PLATYPUS_PATH="`which platypus`"
PLATYPUS=0
if [ $MULTI_APPS -eq 0 ]; then
  if [ -e "$PLATYPUS_PATH" -a "$NO_PLATYPUS" = "" ]; then
    PLATYPUS_VERSION=`$PLATYPUS_PATH -v | cut -f 3 -d ' '`
    echo "  using platypus: $PLATYPUS_PATH version $PLATYPUS_VERSION"
    PLATYPUS=1
  else
    echo "  using launcher only"
  fi
fi

# make sure icon is available
if [ ! -e $RUN_PATH/Resources/VICE.icns ]; then
  echo "ERROR: missing icon: $RUNPATH/Resources/VICE.icns"
  exit 1
fi

# make sure Info.plist is available
if [ ! -e $RUN_PATH/Info.plist ]; then
  echo "ERROR: missing: $RUN_PATH/Info.plist"
  exit 1
fi

# --- create bundles ---

if [ $MULTI_APPS -eq 1 ]; then
  # create a bundle for each emulator
  BUNDLES="$EMULATORS"
else
  BUNDLES="VICE"
fi

copy_tree () {
  (cd "$1" && tar --exclude 'Makefile*' --exclude .svn -c -f - .) | (cd "$2" && tar xf -)
}

ALL_ICONS="VICEFile floppy525 tape cartridge"

create_info_plist () {
  SRC="$1"
  TGT="$2"

  # add filetypes to Info.plist
  if [ "$UI_TYPE" = "cocoa" ]; then
    ADDON="  <key>CFBundleDocumentTypes</key><array>"
    for type in $DROP_FORMATS ; do
      # default icon
      ICON="VICEFile"
      case "$type" in
      [xgd][678][41])
        ICON="floppy525"
        ;;
      tap|t64)
        ICON="tape"
        ;;
      crt)
        ICON="cartridge"
        ;;
      esac
      ADDLINE="<dict><key>CFBundleTypeExtensions</key><array><string>$type</string></array>"
      ADDLINE="$ADDLINE <key>CFBundleTypeIconFile</key><string>$ICON</string>"
      ADDLINE="$ADDLINE <key>CFBundleTypeName</key><string>$type VICE File</string>"
      ADDLINE="$ADDLINE <key>CFBundleTypeRole</key><string>Editor</string></dict>"
      ADDON="$ADDON $ADDLINE"
    done
    ADDON="$ADDON </array></dict>"
    sed -e "s/XVERSIONX/$VICE_VERSION/g" \
        -e "s/XNAMEX/$bundle/g" \
        -e "s,</dict>,$ADDON," \
        < "$SRC" > "$TGT"
  else
    sed -e "s/XVERSIONX/$VICE_VERSION/g" \
        -e "s/XNAMEX/$bundle/g" \
        < "$SRC" > "$TGT"
  fi
}

for bundle in $BUNDLES ; do
  
  APP_NAME=$BUILD_DIR/$bundle.app
  APP_CONTENTS=$APP_NAME/Contents
  APP_MACOS=$APP_CONTENTS/MacOS
  APP_FRAMEWORKS=$APP_CONTENTS/Frameworks
  APP_RESOURCES=$APP_CONTENTS/Resources
  APP_ROMS=$APP_RESOURCES/ROM
  APP_DOCS=$APP_RESOURCES/doc
  
  if [ $MULTI_APPS -eq 1 ]; then
    APP_BIN=$APP_MACOS
    APP_LIB=$APP_MACOS
  else
    APP_BIN=$APP_RESOURCES/bin
    APP_LIB=$APP_RESOURCES/lib
    APP_ETC=$APP_RESOURCES/etc
  fi

  echo "  bundling $bundle.app: "
  echo -n "    "
  
  if [ "$PLATYPUS" = "1" ]; then
    # --- use platypus for bundling ---
    echo -n "[platypus] "
    $PLATYPUS_PATH \
        -a VICE \
        -o None \
        -i $RUN_PATH/Resources/VICE.icns \
        -V "$VICE_VERSION" \
        -u "The VICE Team" \
        -I "org.viceteam.VICE" \
        -c $RUN_PATH/$LAUNCHER \
        $APP_NAME
    PLATYPUS_STATUS=$?
    if [ $PLATYPUS_STATUS -ne 0 ]; then
      echo "ERROR: platypus failed with $PLATYPUS_STATUS"
      exit $PLATYPUS_STATUS
    fi

    # where is the launcher script
    LAUNCHER_SCRIPT_REL="Resources/script"
    # make launcher executable
    chmod 755 $APP_RESOURCES/script
  else
    # --- bundling without platypus ---
    # create directory structure
    echo -n "[app dirs] "
    mkdir -p $APP_CONTENTS
    mkdir -p $APP_MACOS
    mkdir -p $APP_FRAMEWORKS
    mkdir -p $APP_RESOURCES

    # copy icons
    ICON="$RUN_PATH/Resources/$bundle.icns"
    if [ ! -e "$ICON" ]; then
      ICON="$RUN_PATH/Resources/VICE.icns"
    fi
    ICON_BASE="`basename \"$ICON\"`"
    echo -n "[icon=$ICON_BASE] "
    cp $ICON $APP_RESOURCES/

    # add VICE.icns
    if [ "$bundle" != "VICE" ]; then
      ICON="$RUN_PATH/Resources/VICE.icns"
      cp $ICON $APP_RESOURCES/
      echo -n "[icon=VICE] "
    fi

    # setup Info.plist
    echo -n "[Info.plist] "
    create_info_plist "$RUN_PATH/Info.plist" "$APP_CONTENTS/Info.plist"
    
    # copy extra icons
    echo -n "[FTIcons:"
    for icon in $ALL_ICONS ; do
      ICON_FILE="$RUN_PATH/Resources/$icon.icns"
      if [ -e "$ICON_FILE" ]; then
        echo -n "$icon "
        cp $ICON_FILE $APP_RESOURCES/
      fi
    done
    echo "] "
    echo -n "    "

    # copy launcher for non-cocoa
    if [ $MULTI_APPS -eq 0 ]; then
      echo -n "[launcher] "
      if [ ! -e $RUN_PATH/$LAUNCHER ]; then
        echo "ERROR: missing launcher script: $RUNPATH/$LAUNCHER"
        exit 1
      fi
      MAIN_PROG=$APP_MACOS/VICE
      cp $RUN_PATH/$LAUNCHER $MAIN_PROG
      chmod 755 $MAIN_PROG

      # where is the launcher script
      LAUNCHER_SCRIPT_REL="MacOS/VICE"
    elif [ "$UI_TYPE" = "cocoa" ]; then
      # embed resources for cocoa
      LOC_RESOURCES="$RUN_PATH/Resources"
      
      # copy extra files from Resources
      EXTRA_RES_FILES="Credits.html"
      for f in $EXTRA_RES_FILES ; do
        echo -n "[$f] "
        cp "$LOC_RESOURCES/$f" "$APP_RESOURCES/"
      done
      
      # rename emu nib
      RES_LANGUAGES="English"
      for lang in $RES_LANGUAGES ; do
        echo -n "[lang:$lang"

        RES_DIR="$APP_RESOURCES/${lang}.lproj"
        mkdir -p "$RES_DIR"
        copy_tree "$LOC_RESOURCES/${lang}.lproj" "$RES_DIR"
        
        # make emu nib the MainMenu.nib
        EMU_NIB="$RES_DIR/$bundle.nib"
        if [ -e "$EMU_NIB" ]; then
          echo -n " nib"
          MAIN_NIB="$RES_DIR/MainMenu.nib"
          mv "$EMU_NIB" "$MAIN_NIB"
        else 
          echo -n " **MISSING:nib"
        fi
        # remove unwanted emu nibs
        find -d "$RES_DIR" -name "x*.nib" -exec rm -rf {} \;
                
        echo -n "]"
      done
      
      # clean up nibs and remove developer files
      echo -n "[clean nib] "
      find "$APP_RESOURCES" \( -name "info.nib" -o -name "classes.nib" -o -name "designable.nib" \) -exec rm {} \;
      
    fi
  fi

  echo -n "[dirs] "
  mkdir -p $APP_ROMS
  mkdir -p $APP_DOCS
  mkdir -p $APP_BIN

  # copy roms and data into bundle
  echo -n "[common ROMs] "
  for rom in $ROM_COMMON ; do
    if [ ! -d $TOP_DIR/data/$rom ]; then
      echo "ERROR: missing ROM: $TOP_DIR/data/$rom"
      exit 1
    fi
    if [ ! -d "$APP_ROMS/$rom" ]; then
        mkdir "$APP_ROMS/$rom"
    fi
    copy_tree "$TOP_DIR/data/$rom" "$APP_ROMS/$rom"
    (cd "$APP_ROMS/$rom" && eval "rm -f $ROM_REMOVE")
  done

  # copy html docs into bundle
  echo -n "[docs] "
  copy_tree "$TOP_DIR/doc/html" "$APP_DOCS"
  (cd $APP_DOCS && eval "rm -rf $DOC_REMOVE")

  # embed c1541
  echo -n "[c1541] "
  if [ ! -e src/c1541 ]; then
    echo "ERROR: missing binary: src/c1541"
    exit 1
  fi
  cp src/c1541 $APP_BIN/

  # strip embedded c1541 binary
  if [ x"$STRIP" = "xstrip" ]; then
    echo -n "[strip c1541] "
    /usr/bin/strip $APP_BIN/c1541
  fi

  # any dylibs required?
  if [ -d lib ]; then
    mkdir -p $APP_LIB
    DYLIBS=`find lib -name *.dylib`
    NUMDYLIBS=`echo $DYLIBS | wc -w`
    echo -n "[dylibs"
    for lib in $DYLIBS ; do
      echo -n "."
      cp $lib $APP_LIB
    done
    echo -n "] "
  fi

  # any config files from /etc?
  if [ -d etc ]; then
    mkdir -p $APP_ETC
    echo -n "[etc"
    (cd etc && tar cf - *) | (cd "$APP_ETC" && tar xf -)
    echo -n "] "
  fi

  # ready with bundle
  echo  

  # --- embed binaries ---
  if [ "$bundle" = "VICE" ]; then
    BINARIES="$EMULATORS"
  else
    BINARIES="$bundle"
  fi
  for emu in $BINARIES ; do
    echo -n "     embedding $emu: "

    # copy binary
    echo -n "[binary] "
    if [ ! -e src/$emu ]; then
      echo "ERROR: missing binary: src/$emu"
      exit 1
    fi
    cp src/$emu $APP_BIN/$emu

    # strip binary
    if [ x"$STRIP" = "xstrip" ]; then
      echo -n "[strip] "
      /usr/bin/strip $APP_BIN/$emu
    fi

    # copy any needed "local" libs
    LOCAL_LIBS=`otool -L $APP_BIN/$emu | egrep '^\s+/(opt|usr)/local/'  | awk '{print $1}'`
    for lib in $LOCAL_LIBS; do
        cp $lib $APP_FRAMEWORKS
        lib_base=`basename $lib`
        LOCAL_LIBS_LIBS=`otool -L $APP_FRAMEWORKS/$lib_base | egrep '^\s+/(opt|usr)/local/' | grep -v $lib_base | awk '{print $1}'`
        for lib_lib in $LOCAL_LIBS_LIBS; do
            cp $lib_lib $APP_FRAMEWORKS
            lib_lib_base=`basename $lib_lib`
            chmod 644 $APP_FRAMEWORKS/$lib_base
            install_name_tool -change $lib_lib @executable_path/../Frameworks/$lib_lib_base $APP_FRAMEWORKS/$lib_base
        done
        install_name_tool -change $lib @executable_path/../Frameworks/$lib_base $APP_BIN/$emu
        install_name_tool -change $lib @executable_path/../Frameworks/$lib_base $APP_BIN/c1541
    done

    # copy emulator ROM
    eval "ROM=\${ROM_$emu}"
    echo -n "[ROM=$ROM] "
    if [ ! -d $TOP_DIR/data/$ROM ]; then
      echo "ERROR: missing ROM: $TOP_DIR/data/$ROM"
      exit 1
    fi
    if [ ! -d "$APP_ROMS/$ROM" ]; then
        mkdir "$APP_ROMS/$ROM"
    fi
    copy_tree "$TOP_DIR/data/$ROM" "$APP_ROMS/$ROM"
    (cd $APP_ROMS/$ROM && eval "rm -f $ROM_REMOVE")

    # ready
    echo
  done

  # print bundle size
  echo -n "    => " ; du -sh "$APP_NAME" | awk '{ print $1 }'

done

# --- copy tools ---
for tool in $TOOLS ; do
  echo -n "  copying tool $tool: "

  # copy binary
  echo -n "[binary] "
  if [ ! -e src/$tool ]; then
    echo "ERROR: missing binary: src/$tool"
    exit 1
  fi
  cp src/$tool $TOOL_DIR/
  
  # strip binary
  if [ x"$STRIP" = "xstrip" ]; then
    echo -n "[strip] "
    /usr/bin/strip $TOOL_DIR/$tool
  fi

  # ready
  echo
done

# --- copy command line launcher ---
echo "  copying command line launcher"
if [ ! -e "$RUN_PATH/vice-launcher.sh" ]; then
  echo "Error: '$RUN_PATH/vice-launcher.sh' is missing!"
  exit 1
fi
cp $RUN_PATH/vice-launcher.sh $TOOL_DIR
chmod 755 $TOOL_DIR/vice-launcher.sh
for emu in $EMULATORS ; do
  (cd $TOOL_DIR && ln -sf vice-launcher.sh $emu)
done

# --- copy docs ---
echo "  copying documents"
cp $TOP_DIR/FEEDBACK $BUILD_DIR/FEEDBACK.txt
cp $TOP_DIR/README $BUILD_DIR/README.txt
mkdir "$BUILD_DIR/doc"
copy_tree "$TOP_DIR/doc" "$BUILD_DIR/doc"
mv $BUILD_DIR/doc/readmes/Readme-MacOSX.txt $BUILD_DIR/
mv $BUILD_DIR/doc/building/MacOSX-Howto.txt $BUILD_DIR/doc/
(cd $BUILD_DIR/doc && eval "rm -rf $DOC_REMOVE")

# --- copy fonts ---
FONTS="CBM.ttf"
if [ "$UI_TYPE" = "cocoa" ]; then
  echo "  copying fonts"
  mkdir "$BUILD_DIR/fonts"
  for FONT in $FONTS ; do
    cp "$TOP_DIR/data/fonts/$FONT" "$BUILD_DIR/fonts/"
  done
fi

# --- make dmg? ---
if [ x"$ZIP" = "xnozip" ]; then
  echo "ready. created dist directory: $BUILD_DIR"
  du -sh $BUILD_DIR
else
  # image name
  BUILD_IMG=$BUILD_DIR.dmg
  BUILD_TMP_IMG=$BUILD_DIR.tmp.dmg
  
  # Create the image and format it
  echo "  creating DMG"
  hdiutil create -srcfolder $BUILD_DIR $BUILD_TMP_IMG -volname $BUILD_DIR -ov -quiet 

  # Compress the image
  echo "  compressing DMG"
  hdiutil convert $BUILD_TMP_IMG -format UDZO -o $BUILD_IMG -ov -quiet
  rm -f $BUILD_TMP_IMG

  echo "ready. created dist file: $BUILD_IMG"
  du -sh $BUILD_IMG
  md5 -q $BUILD_IMG
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
