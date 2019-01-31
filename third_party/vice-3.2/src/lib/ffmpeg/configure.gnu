#!/bin/sh

#
# configure.gnu - configure the liblame, libx264 and ffmpeg projects for building with VICE
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

splitcpuos()
{
  cpu=$1
  shift
  man=$1
  shift
  if test x"$2" != "x"; then
    os=$1
    shift
    for i in $*
    do
      os="$os-$i"
    done
  else
    if test x"$1" != "x"; then
      os=$1
    else
      os=$man
    fi
  fi
}

get_last_part()
{
  for i in $*
  do
    part="$i"
  done
}

change_host()
{
  host=""
  part=""

  for i in $*
  do
    if test x"$part" != "x"; then
      if test x"$host" = "x"; then
        host="$part"
      else
        host="$host-$part"
      fi
    fi
    part="$i"
  done
}

check_compiler()
{
  if test x"$compiler" != "x"; then
    if test x"$host" != "x"; then
      get_last_part `echo $compiler | sed 's/-/ /g'`
      if test x"$compiler" != x"$part"; then
        if test x"$compiler" != x"$host-$part"; then
          change_host `echo $compiler | sed 's/-/ /g'`
        fi
      fi
    fi
  fi
}

srcdir=""
shared=no
static=no
makecommand=""
yasmcommand=""
extra_generic_enables=""
extra_ffmpeg_enables=""
extra_x264_enables=""
host=""
cpu=""
os=""
compiler=""
part=""
prefix=""

for i in $*
do
  case "$i" in
    --srcdir*)
      srcdir=`echo $i | sed -e 's/^[^=]*=//g'`
      ;;
    --enable-yasm-command*)
      yasmcommand=`echo $i | sed -e 's/^[^=]*=//g'`
      ;;
    --enable-make-command*)
      makecommand=`echo $i | sed -e 's/^[^=]*=//g'`
      ;;
    --enable-compiler*)
      compiler=`echo $i | sed -e 's/^[^=]*=//g'`
      check_compiler
      ;;
    --enable-shared-ffmpeg)
      shared=yes
      ;;
    --enable-static-ffmpeg)
      static=yes
      ;;
    --enable-w32threads)
      extra_ffmpeg_enables="$extra_ffmpeg_enables $i"
      extra_x264_enables="$extra_x264_enables --enable-win32thread"
      ;;
    --enable-full-host*)
      host=`echo $i | sed -e 's/^[^=]*=//g'`
      splitcpuos `echo $host | sed 's/-/ /g'`
      check_compiler
      ;;
    --host*)
      hostprefix=`echo $i | sed -e 's/^[^=]*=//g'`
      ;;
    --prefix*)
      prefix=`echo $i | sed -e 's/^[^=]*=//g'`
      ;;
esac
done

NEW_SHELL=""

if test x"$BASH" = "x"; then
  for i in /bin/bash /usr/bin/bash /usr/local/bin/bash /usr/pkg/bin/bash
  do
    if test -e "$i"; then
      NEW_SHELL=$i
    fi
  done
else
  NEW_SHELL=${SHELL}
fi

curdir=`pwd`

if [ ! -d "../liblame" ]; then
  mkdir ../liblame
fi

cd ../liblame
cur=`pwd`
if test x"$shared" = "xyes"; then
  if test x"$hostprefix" != "x"; then
    config_line="$srcdir/../liblame/configure -v --enable-shared --disable-frontend --prefix=$prefix $extra_generic_enables --host=$host"
  else
    config_line="$srcdir/../liblame/configure -v --enable-shared --disable-frontend --prefix=$prefix $extra_generic_enables"
  fi
else
  if test x"$hostprefix" != "x"; then
    config_line="$srcdir/../liblame/configure -v --disable-shared --enable-static --disable-frontend --prefix=$prefix $extra_generic_enables --host=$host"
  else
    config_line="$srcdir/../liblame/configure -v --disable-shared --enable-static --disable-frontend --prefix=$prefix $extra_generic_enables"
  fi
fi
cat <<__END
Running configure in liblame with $config_line
__END

CONFIG_SHELL=/bin/sh $config_line

if test x"$shared" = "xyes"; then
    $makecommand install-stuff
else
    $makecommand -e prefix=$cur/../libffmpeg install-stuff
fi

if test x"$shared" != "xyes"; then
  if [ ! -d "$cur/../libffmpeg/lib" ]; then
    mkdir -p $cur/../libffmpeg/lib
  fi

  if [ -f "$cur/../libffmpeg/lib64/libmp3lame.a" ]; then
    cp $cur/../libffmpeg/lib64/libmp3lame.a $cur/../libffmpeg/lib/libmp3lame.a
  fi
fi

if [ ! -d "../libx264" ]; then
  mkdir ../libx264
fi

cd ../libx264
cur=`pwd`

case "$host" in
  *-*-linux*)
    pic_option="--enable-pic"
    ;;
  *)
    pic_option=""
    ;;
esac

case "$host" in
  *-*-amigaos*)
    asm_option="--disable-asm"
    ;;
  *-*-msdosdjgpp*)
    asm_option="--disable-asm"
    ;;
  *)
    asm_option=""
    ;;
esac

if test x"$shared" = "xyes"; then
  if test x"$hostprefix" != "x"; then
    config_line="$srcdir/../libx264/configure --enable-shared --enable-static --yasm-prog=${yasmcommand} --prefix=$prefix $extra_generic_enables $extra_x264_enables --host=$host --cross-prefix=$hostprefix-"
  else
    config_line="$srcdir/../libx264/configure --enable-shared --enable-static --yasm-prog=${yasmcommand} --prefix=$prefix $extra_generic_enables $extra_x264_enables --compiler=${compiler}"
  fi
else
  if test x"$hostprefix" != "x"; then
    config_line="$srcdir/../libx264/configure --enable-static $pic_option $asm_option --yasm-prog=${yasmcommand} --prefix=$prefix --host=$host --cross-prefix=$hostprefix-"
  else
    config_line="$srcdir/../libx264/configure --enable-static $pic_option $asm_option --yasm-prog=${yasmcommand} --prefix=$prefix --compiler=${compiler}"
  fi
fi

cat <<__END
Running configure in libx264 with $config_line --extra-cflags="-Wno-deprecated-declarations"
__END

${NEW_SHELL} $config_line --extra-cflags="-Wno-deprecated-declarations"

if test x"$shared" = "xyes"; then
    $makecommand install-stuff
else
    $makecommand -e prefix=$cur/../libffmpeg install-stuff
fi

if test x"$shared" != "xyes"; then
  if [ -f "$cur/../libffmpeg/lib64/libx264.a" ]; then
    cp $cur/../libffmpeg/lib64/libx264.a $cur/../libffmpeg/lib/libx264.a
  fi
fi

if [ ! -d "../libffmpeg" ]; then
  mkdir ../libffmpeg
fi

cd ../libffmpeg
cur=`pwd`

case "$host" in
  *-amigaos*)
    asm_option="--disable-asm"
    ;;
  *)
    asm_option=""
    ;;
esac


if test x"$shared" = "xyes"; then
  if test x"$hostprefix" != "x"; then
    config_line="$srcdir/../libffmpeg/configure --yasmexe=${yasmcommand} $asm_option --enable-libmp3lame --enable-libx264 --enable-shared --disable-static --disable-programs --enable-gpl $extra_ffmpeg_enables $extra_generic_enables --arch=$cpu --target-os=$os --cross-prefix=$hostprefix- --prefix=$prefix"
  else
    config_line="$srcdir/../libffmpeg/configure --yasmexe=${yasmcommand} $asm_option --enable-libmp3lame --enable-libx264 --enable-shared --disable-static --disable-programs --enable-gpl $extra_ffmpeg_enables $extra_generic_enables --cc=${compiler} --prefix=$prefix"
  fi
else
  if test x"$hostprefix" != "x"; then
    config_line="$srcdir/../libffmpeg/configure --yasmexe=${yasmcommand} $asm_option --enable-libmp3lame --enable-libx264 --enable-static --disable-programs --enable-gpl $extra_ffmpeg_enables $extra_generic_enables --arch=$cpu --target-os=$os --cross-prefix=$hostprefix-"
  else
    config_line="$srcdir/../libffmpeg/configure --yasmexe=${yasmcommand} $asm_option --enable-libmp3lame --enable-libx264 --enable-static --disable-programs --enable-gpl $extra_ffmpeg_enables $extra_generic_enables --cc=${compiler}"
  fi
fi

if test x"$shared" = "xyes"; then
cat <<__END
Running configure in libffmpeg with $config_line --extra-ldflags="-L$prefix/lib -L$prefix/lib64" --extra-cflags="-I$prefix/include -Wno-deprecated-declarations"
__END
else
cat <<__END
Running configure in libffmpeg with $config_line --extra-ldflags="-Llib -Llib64" --extra-cflags="-Iinclude -Wno-deprecated-declarations"
__END
fi

if test x"$shared" = "xyes"; then
  ${NEW_SHELL} $config_line --extra-cflags="-I$prefix/include -Wno-deprecated-declarations" --extra-ldflags="-L$prefix/lib -L$prefix/lib64"
else
  ${NEW_SHELL} $config_line --extra-cflags="-Iinclude -Wno-deprecated-declarations" --extra-ldflags="-Llib -Llib64"
fi
