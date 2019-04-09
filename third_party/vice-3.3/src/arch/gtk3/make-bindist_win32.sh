#!/bin/sh

#
# make-bindist_win32.sh -- Make a binary distribution for the Windows GTK3 port.
#
# Written by
#  Marco van den Heuvel <blackystardust68@yahoo.com>
#  Greg King <gregdk@users.sf.net>
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
# Usage: make-bindist.sh <strip> <vice-version> <--enable-arch> <zip|nozip> <x64sc-included> <top-srcdir> <cpu> <abs-top-builddir> <cross> <objdump> <compiler>
#                         $1      $2             $3              $4          $5               $6           $7    $8                 $9      $10       $11
#

STRIP=$1
VICEVERSION=$2
ENABLEARCH=$3
ZIPKIND=$4
X64SCINC=$5
TOPSRCDIR=$6
CPU=$7
TOPBUILDDIR=$8
CROSS=$9

shift
OBJDUMP=$9

shift
COMPILER=$9


# Try to get the SVN revision
#echo "Trying to get SVN revision"
SVN_SUFFIX=""
svnrev_string=`svnversion $TOPSRCDIR`
if test "$?" != "0"; then
    #echo "No svnversion found"
    # nop:
    :
else
    # Choose the second number (usually higher) if it exists; drop letter suffixes.
    svnrev=`echo "$svnrev_string" | sed 's/^\([0-9]*:\)*\([0-9]*\)*.*/\2/'`
    #echo "svnrev string: $svnrev"
    # Only a number is extracted.
    if test -n "$svnrev"
        then SVN_SUFFIX="-r$svnrev"
    fi
fi


get_dll_deps()
{
  for j in `find $BUILDPATH -name "*.dll"`
  do
    dlls=`$OBJDUMP -p $j | sed 's| |\n|g' | grep -F ".dll"`
    for i in $dlls
    do
      if test -e $dlldir/$i; then
        cp $dlldir/$i $BUILDPATH
      fi
    done
  done
}


if test x"$CPU" = "xx86_64" -o x"$CPU" = "xamd64"; then
  WINXX="win64"
else
  WINXX="win32"
fi

if test x"$X64SCINC" = "xyes"; then
  SCFILE="x64sc"
else
  SCFILE=""
fi

EMULATORS="x64 xscpu64 x64dtv $SCFILE x128 xcbm2 xcbm5x0 xpet xplus4 xvic vsid"
CONSOLE_TOOLS="c1541 cartconv petcat"
EXECUTABLES="$EMULATORS $CONSOLE_TOOLS"
unset CONSOLE_TOOLS EMULATORS SCFILE X64SCINC CPU svnrev_string

for i in $EXECUTABLES; do
  if [ ! -x $TOPBUILDDIR/src/$i.exe ]; then
    echo 'Error: executable files not found; do a "make" first.'
    exit 1
  fi
done


GTK3NAME="GTK3VICE"
BUILDPATH="$TOPBUILDDIR/$GTK3NAME-$VICEVERSION-$WINXX$SVN_SUFFIX"
#echo "BUILDPATH = $BUILDPATH"


echo "Removing an old $BUILDPATH ..."
rm -r -f $BUILDPATH

echo "Generating a $WINXX GTK3 port binary distribution..."
mkdir $BUILDPATH

for i in $EXECUTABLES; do
  cp $TOPBUILDDIR/src/$i.exe $BUILDPATH
  $STRIP $BUILDPATH/$i.exe
done

if test x"$CROSS" != "xtrue"; then

# The following lines assume that this script is run by MSYS2.
  cp `ntldd -R $BUILDPATH/x64.exe|gawk '/\\\\bin\\\\/{print $3;}'|cygpath -f -` $BUILDPATH
  cp $MINGW_PREFIX/bin/lib{croco-0.6-3,lzma-5,rsvg-2-2,xml2-2}.dll $BUILDPATH
  cp $MINGW_PREFIX/bin/gspawn-win??-helper-console.exe $BUILDPATH
  cd $MINGW_PREFIX
  cp --parents lib/gdk-pixbuf-2.0/2.*/loaders.cache lib/gdk-pixbuf-2.0/2.*/loaders/libpixbufloader-{png,svg,xpm}.dll $BUILDPATH
  cp --parents share/glib-2.0/schemas/gschemas.compiled $BUILDPATH
  cp --parents -a share/icons/Adwaita $BUILDPATH
  cp --parents share/icons/hicolor/index.theme $BUILDPATH
  cd - >/dev/null
else

# The following lines assume a cross compiler,
# and the DLLs installed in the dll dir. of that toolchain.
  libm=`i686-w64-mingw32-gcc -print-file-name=libm.a`
  location=`dirname $libm`
  loc=`dirname $location`
  dlldir="$loc/dll"
  dlls=`$OBJDUMP -p src/x64.exe | sed 's| |\n|g' | grep -F ".dll"`
  for i in $dlls
  do
    if test -e $dlldir/$i; then
      cp $dlldir/$i $BUILDPATH
    fi
  done
  cp $dlldir/lib{bz2-1,freetype-6,gcc_s_dw2-1,croco-0.6-3,lzma-5,rsvg-2-2,xml2-2}.dll $BUILDPATH
  gccname=`$COMPILER -print-file-name=libgcc.a`
  gccdir=`dirname $gccname`
  cp $gccdir/libgcc*.dll $BUILDPATH
  get_dll_deps
  get_dll_deps
  current=`pwd`
  cd $loc
  cp --parents lib/gdk-pixbuf-2.0/2.*/loaders.cache lib/gdk-pixbuf-2.0/2.*/loaders/libpixbufloader-{png,svg,xpm}.dll $BUILDPATH
  cp --parents share/glib-2.0/schemas/gschemas.compiled $BUILDPATH
  cp --parents -a share/icons/Adwaita $BUILDPATH
  cp --parents share/icons/hicolor/index.theme $BUILDPATH
  cd $current
fi

cp -a $TOPSRCDIR/data/C128 $TOPSRCDIR/data/C64 $BUILDPATH
cp -a $TOPSRCDIR/data/C64DTV $TOPSRCDIR/data/CBM-II $BUILDPATH
cp -a $TOPSRCDIR/data/DRIVES $TOPSRCDIR/data/PET $BUILDPATH
cp -a $TOPSRCDIR/data/PLUS4 $TOPSRCDIR/data/PRINTER $BUILDPATH
cp -a $TOPSRCDIR/data/SCPU64 $TOPSRCDIR/data/VIC20 $BUILDPATH
rm -f `find $BUILDPATH -name "Makefile*"`
rm -f `find $BUILDPATH -name "sdl_*"`
mkdir $BUILDPATH/gui
cp $TOPBUILDDIR/src/arch/gtk3/data/vice.gresource $BUILDPATH/gui
cp $TOPSRCDIR/data/fonts/CBM.ttf $BUILDPATH/gui
cp -a $TOPSRCDIR/doc/html $BUILDPATH
cp -a -u $TOPBUILDDIR/doc/html $BUILDPATH
rm -f $BUILDPATH/html/Makefile* $BUILDPATH/html/checklinks.sh $BUILDPATH/html/texi2html
rm -f $BUILDPATH/html/robots.txt $BUILDPATH/html/sitemap.xml
rm -f $BUILDPATH/html/COPYING $BUILDPATH/html/NEWS
cp $TOPSRCDIR/COPYING $TOPSRCDIR/FEEDBACK $TOPSRCDIR/NEWS $TOPSRCDIR/README $BUILDPATH
cp $TOPSRCDIR/doc/readmes/Readme-GTK3.txt $BUILDPATH
mkdir $BUILDPATH/doc
cp $TOPBUILDDIR/doc/vice.pdf $BUILDPATH/doc
cp $TOPBUILDDIR/doc/vice.{chm,hlp} $BUILDPATH/doc


if test x"$ZIPKIND" = "xzip"; then
  rm -f $BUILDPATH.zip
  cd $BUILDPATH/..
  if test x"$ZIP" = "x"
    then zip -r -9 -q $BUILDPATH.zip $GTK3NAME-$VICEVERSION-$WINXX$SVN_SUFFIX
    else $ZIP $BUILDPATH.zip $GTK3NAME-$VICEVERSION-$WINXX$SVN_SUFFIX
  fi
  rm -r -f $BUILDPATH
  echo "$WINXX GTK3 port binary distribution archive generated as $BUILDPATH.zip"
else echo "$WINXX GTK3 port binary distribution directory generated as $BUILDPATH/"
fi

if test x"$ENABLEARCH" = "xyes"; then
  echo ''
  echo 'Warning: The binaries are optimized for your system.'
  echo 'They might not run on a different system.'
  echo 'Configure with --enable-arch=no to avoid that.'
  echo ''
fi
