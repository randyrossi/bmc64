#!/bin/sh

#
# make-bindist.sh - make binary distribution for the OpenServer 5.x/6.x, UnixWare and Solaris ports
#
# Written by
#  Marco van den Heuvel <blackystardust68@yahoo.com>
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
# Usage: make-bindist.sh <strip> <vice-version> <prefix> <cross> <--enable-arch> <zip|nozip> <x64sc-included> <system> <gui> <infodir> <mandir> <topsrcdir> <make-command> <host-cpu> <host-os>
#                         $1      $2             $3       $4      $5              $6          $7               $8       $9    $10       $11      $12         $13            $14        $15
#

STRIP=$1
VICEVERSION=$2
PREFIX=$3
CROSS=$4
ENABLEARCH=$5
ZIPKIND=$6
X64SC=$7
SYSTEM=$8
GUI=$9

shift
INFODIR=$9

shift
MANDIR=$9

shift
TOPSRCDIR=$9

shift
MAKECOMMAND=$9

shift
HOSTCPU=$9

shift
HOSTOS=$9

setnormalmake()
{
  makefound="none"
  OLD_IFS=$IFS
  IFS=":"

  TOTALPATH="/usr/ccs/bin:$PATH"

  for i in $TOTALPATH
  do
    if [ -e $i/make ]; then
      GNUMAKE=`$i/make --version`
      case "$GNUMAKE" in
        GNU*)
          ;;
        *)
          if test x"$makefound" = "xnone"; then
            makefound="$i/make"
          fi
          ;;
      esac
    fi
  done
  if test x"$makefound" = "xnone"; then
    echo no suitable make found for bindist
    exit 1
  else
    MAKECOMMAND=$makefound
  fi
  IFS=$OLD_IFS
}

checkmake()
{
  GNUMAKE=`$MAKECOMMAND --version`
  case "$GNUMAKE" in
    GNU*)
      ;;
    *)
      setnormalmake
      ;;
  esac
}

trimpath()
{
  pmlen=`expr length $1`
  pmlen=`expr $pmlen - 1`
  pmpath=`expr substr $1 2 $pmlen`
}

splitpmfile()
{
  pmmask=$1
  pmsize=$2
  pmdate=$3
  pmrealblocks=$4
  pmblocksize=$5

  if test x"$pmblocksize" != "x512"; then
    pmblocks=`expr $pmrealblocks \* $pmblocksize \/ 512`
  else
    pmblocks=$pmrealblocks
  fi
}

splitsum()
{
  pmrealsum=$1
}

#check if sum -s is needed
echo >sum.tmp "hello"
pmsum=`sum sum.tmp`
splitsum $pmsum
if test x"$pmrealsum" != "x542"; then
  pmsum=`sum -s sum.tmp`
  splitsum $pmsum
  if test x"$pmrealsum" != "x542"; then
    echo "No sum command found that gives the right result"
    exit 1
  else
    SUMCOMMAND="sum -s"
  fi
else
  SUMCOMMAND=sum
fi
rm -f sum.tmp

if test x"$PREFIX" != "x/usr/local"; then
  echo Error: installation path is not /usr/local
  exit 1
fi

if test x"$SYSTEM" = "xsco7"; then
  PLATFORM="UNIXWARE 7.x"
  checkmake
fi

if test x"$SYSTEM" = "xsco6"; then
  PLATFORM="OPENSERVER 6.x"
  checkmake
fi

if test x"$SYSTEM" = "xsco5"; then
  PLATFORM="OPENSERVER 5.x"
  checkmake
fi

if test x"$SYSTEM" = "xsol"; then
  PLATFORM="SOLARIS"
  checkmake
fi

if test x"$X64SC" = "xyes"; then
  SCFILE="x64sc"
else
  SCFILE=""
fi

EMULATORS="x64 xscpu64 x64dtv $SCFILE x128 xcbm2 xcbm5x0 xpet xplus4 xvic vsid"
CONSOLE_TOOLS="c1541 cartconv petcat"
EXECUTABLES="$EMULATORS $CONSOLE_TOOLS"

for i in $EXECUTABLES
do
  if [ ! -e src/$i ]
  then
    echo Error: \"make\" needs to be done first
    exit 1
  fi
done

echo Generating $PLATFORM port binary distribution.
curdir=`pwd`
if [ ! -e make-e-failed.tmp ]
then
  rm -f -r VICE-$VICEVERSION
  $MAKECOMMAND -e prefix=$curdir/VICE-$VICEVERSION/usr/local VICEDIR=$curdir/VICE-$VICEVERSION/usr/local/lib/vice install
  if [ ! -e VICE-$VICEVERSION/usr/local/bin/x64 ]
  then
    echo "For some unknown reason the make did not create the correct directories, please enter the following command:"
    echo "$MAKECOMMAND -e prefix=$curdir/VICE-$VICEVERSION/usr/local VICEDIR=$curdir/VICE-$VICEVERSION/usr/local/lib/vice install"
    echo ""
    echo "After the command finishes do a make bindist(zip) again"
    touch make-e-failed.tmp
    exit 1
  fi
else
  if [ ! -e VICE-$VICEVERSION/usr/local/bin/x64 ]
  then
    echo "For some unknown reason the make did not create the correct directories, please enter the following command:"
    echo "$MAKECOMMAND -e prefix=$curdir/VICE-$VICEVERSION/usr/local VICEDIR=$curdir/VICE-$VICEVERSION/usr/local/lib/vice install"
    echo ""
    echo "After the command finishes do a make bindist(zip) again"
    touch make-e-failed.tmp
    exit 1
  else
    rm -f make-e-failed.tmp
  fi
fi

for i in $EXECUTABLES
do
  $STRIP VICE-$VICEVERSION/usr/local/bin/$i
done
mkdir -p VICE-$VICEVERSION/$MANDIR/man1
if test x"$ZIPKIND" = "xzip"; then

  if test x"$GUI" = "xgtk"; then
    GUIVICE=GTKVICE
  else
    GUIVICE=XAWVICE
  fi

  if test x"$SYSTEM" = "xsol"; then
    arch_cpu=unknown

    file VICE-$VICEVERSION/usr/local/bin/x64 >file.tmp
    cpu_is_64bit=`sed -n -e "s/.*\(64-bit\).*/\1/p" file.tmp`
    rm -f file.tmp

    if test x"$HOSTCPU" = "xs390"; then
      arch_cpu=s390
      arch_expected_cpu=s390
    fi

    if test x"$HOSTCPU" = "xarm"; then
      arch_cpu=arm
      arch_expected_cpu=arm
    fi

    if test x"$HOSTCPU" = "xi386"; then
      arch_cpu=x86
      arch_expected_cpu=i386
    fi

    if test x"$HOSTCPU" = "xi486"; then
      arch_cpu=x86
      arch_expected_cpu=i386
    fi

    if test x"$HOSTCPU" = "xi586"; then
      arch_cpu=x86
      arch_expected_cpu=i386
    fi

    if test x"$HOSTCPU" = "xi686"; then
      arch_cpu=x86
      arch_expected_cpu=i386
    fi

    if test x"$HOSTCPU" = "xamd64"; then
      arch_cpu=amd64
      arch_expected_cpu=i386
    fi

    if test x"$HOSTCPU" = "xx86_64"; then
      arch_cpu=amd64
      arch_expected_cpu=i386
    fi

    if test x"$HOSTCPU" = "xsparc"; then
      arch_cpu=sparc
      arch_expected_cpu=sparc
    fi

    if test x"$HOSTCPU" = "xsparc64"; then
      arch_cpu=sparc64
      arch_expected_cpu=sparc
    fi

    if test x"$arch_cpu" = "xx86"; then
      if test x"$cpu_is_64bit" = "x64-bit"; then
        arch_cpu=amd64
      fi
    fi

    if test x"$arch_cpu" = "xsparc"; then
      if test x"$cpu_is_64bit" = "x64-bit"; then
        arch_cpu=sparc64
      fi
    fi

    if test x"$HOSTOS" = "xsolaris2.5.1"; then
      arch_version=sol25
      arch_expected_version=5.5.1
    fi

    if test x"$HOSTOS" = "xsolaris2.6"; then
      arch_version=sol26
      arch_expected_version=5.6
    fi

    if test x"$HOSTOS" = "xsolaris2.7"; then
      arch_version=sol7
      arch_expected_version=5.7
    fi

    if test x"$HOSTOS" = "xsolaris2.8"; then
      arch_version=sol8
      arch_expected_version=5.8
    fi

    if test x"$HOSTOS" = "xsolaris2.9"; then
      arch_version=sol9
      arch_expected_version=5.9
    fi

    if test x"$HOSTOS" = "xsolaris2.10"; then
      arch_version=sol10
      arch_expected_version=5.10
    fi

    if test x"$HOSTOS" = "xsolaris2.11"; then
      opensolaris=`uname -v`
      if test x"$opensolaris" != "x11.0"; then
        arch_version=osol
        arch_expected_version=osol
      else
        arch_version=sol11
        arch_expected_version=5.11
      fi
    fi
  else
    arch_cpu=x86

    if test x"$SYSTEM" = "xsco5"; then
      arch_version=osr5
    fi

    if test x"$SYSTEM" = "xsco6"; then
      arch_version=osr6
    fi

    if test x"$SYSTEM" = "xsco7"; then
      arch_version=uw7
    fi
  fi

  mv VICE-$VICEVERSION $GUIVICE
  mkdir $GUIVICE/root
  mkdir $GUIVICE/install
  mv $GUIVICE/usr $GUIVICE/root/

  cat >$GUIVICE/pkginfo <<_END
PKG="$GUIVICE"
NAME="VICE-$GUI"
ARCH="$arch_cpu"
VERSION="$VICEVERSION"
CATEGORY="emulator"
VENDOR="The VICE Team"
EMAIL="vice-emu-mail@lists.sourceforge.net"
PSTAMP="Marco van den Heuvel"
BASEDIR=/
CLASSES="none"
_END

  if test x"$SYSTEM" = "xsol"; then
    cat >$GUIVICE/install/checkinstall.tmp <<_END
#!/bin/sh
#
expected_release="$arch_expected_version"
expected_platform="$arch_expected_cpu"
#
_END
    cat >$GUIVICE/install/checkinstall $GUIVICE/install/checkinstall.tmp $TOPSRCDIR/src/arch/unix/sco_sol/checkinstall
    rm -f $GUIVICE/install/checkinstall.tmp
  else
    cat >$GUIVICE/install/checkinstall <<_END
#!/bin/sh
#
exit 0
_END
  fi

  cd $GUIVICE/root
  totalblocks=0
  for i in `find .`
  do
    if test x"$i" != "x." -a x"$i" != "x./usr" -a x"$i" != "x./usr/local"; then
      filetype=`stat -c %F $i`
      if test x"$filetype" = "xdirectory"; then
        trimpath $i
        pmmask=`stat -c %a $i`
        echo >>../pkgmap.tmp "1 d none $pmpath 0$pmmask root root"
      else
        trimpath $i
        pmfile=`stat -c "%a %s %Y %b %B" $i`
        splitpmfile $pmfile
        pmsum=`$SUMCOMMAND $i`
        splitsum $pmsum
        echo >>../pkgmap.tmp "1 f none $pmpath 0$pmmask root root $pmsize $pmrealsum $pmdate"
        totalblocks=`expr $totalblocks + $pmblocks`
      fi
    fi
  done
  cd ..
  pmfile=`stat -c "%a %s %Y %b %B" install/checkinstall`
  splitpmfile $pmfile
  pmsum=`$SUMCOMMAND install/checkinstall`
  splitsum $pmsum
  echo >>pkgmap.tmp "1 i checkinstall $pmsize $pmrealsum $pmdate"
  pmfile=`stat -c "%a %s %Y %b %B" pkginfo`
  splitpmfile $pmfile
  pmsum=`$SUMCOMMAND pkginfo`
  splitsum $pmsum
  echo >>pkgmap.tmp "1 i pkginfo $pmsize $pmrealsum $pmdate"
  echo >>pkgmap.hdr ": 1 $totalblocks"
  cat >pkgmap pkgmap.hdr pkgmap.tmp
  rm -f pkgmap.hdr pkgmap.tmp

  cd ..
  packagename=vice-$GUI-$VICEVERSION-$arch_version-$arch_cpu-local.pkg

  tar cf $packagename.tar $GUIVICE
  gzip $packagename.tar
  mv $packagename.tar.gz $packagename.tgz
  rm -f -r $GUIVICE

  echo $PLATFORM port binary package generated as $packagename.tgz
else
  echo $PLATFORM port binary distribution directory generated as VICE-$VICEVERSION
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
