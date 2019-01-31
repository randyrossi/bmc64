#!/bin/sh

#
# fixdox.sh - fix up the texi file for proper generation of the final document.
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
# Usage: fixdox.sh <format-to-fix-for>
#                   $1
#

FORMAT=$1

table4chm()
{
  rm -f chmtmp*.txt
  read data
  newdata=""
  for i in $data
  do
    if test x"$i" != "x@item"; then
      newdata="$newdata $i"
    fi
  done
  echo >>chmtmp.txt "@multitable $newdata"
  read data
  while test x"$data" != "x@end multitable"
  do
    echo >>chmtmp.txt "$data"
    read data
  done
  sed 's/@item/@rowstart/g' <chmtmp.txt >chmtmp2.txt
  cat chmtmp2.txt
  rm -f chmtmp*.txt
}

fixtxt()
{
  while read data
  do
    if test x"$data" != "xSTART-INFO-DIR-ENTRY" -a x"$data" != "xEND-INFO-DIR-ENTRY"; then
      echo "$data"
    else
      if test x"$data" = "xSTART-INFO-DIR-ENTRY"; then
        read data
        header=""
        for i in $data
        do
          if test x"$header" != "x"; then
             header="$header $i"
          fi
          if test x"$header" = "x" -a x"$i" = "x(vice)."; then
             header=" "
          fi
        done
        echo $header
      fi
    fi
  done
}

fixchm()
{
  outputok=yes
  while read data
  do
    case x"${data}" in
      "x@multitable"*)
        table4chm
        ;;
      "x@ifset"*)
        outputok=no
        ;;
      "x@ifclear"*)
        outputok=no
        ;;
    esac

    if test x"$data" = "x@ifinfo"; then
      outputok=no
    fi

    if test x"$outputok" = "xyes"; then
      echo $data
    fi

    if test x"$data" = "x@ifset platformwindows"; then
      outputok=yes
    fi

    if test x"$data" = "x@end ifinfo"; then
      outputok=yes
    fi

    if test x"$data" = "x@end ifset"; then
      outputok=yes
    fi

    if test x"$data" = "x@end ifclear"; then
      outputok=yes
    fi
  done
}

fixhlp()
{
  sed <tmp.texi >vicetmp.texi -e 's/@firstparagraphindent none//g' -e 's/@exampleindent 0//g' -e 's/@verbatim/@smallexample/g' -e 's/@end verbatim/@end smallexample/g'
  rm -f -r tmp.texi
}

fixguide()
{
  sed <tmp.texi >vicetmp.texi -e 's/@ifinfo//g' -e 's/@end ifinfo//g' -e 's/@format//g' -e 's/@end format//g' -e 's/@firstparagraphindent none//g' -e 's/@exampleindent 0//g'
}

fixpdf()
{
  echo not implemented yet
}

fixipf()
{
  sed <tmp.texi >vicetmp.texi -e 's/@firstparagraphindent//g' -e 's/@paragraphindent//g' -e 's/@exampleindent//g' -e 's/@ifcommanddefined//g' -e 's/@end ifcommanddefined//g'
  rm -f -r tmp.texi
}

if test x"$FORMAT" = "xtxt"; then
  fixtxt
fi

if test x"$FORMAT" = "xchm"; then
  fixchm
fi

if test x"$FORMAT" = "xhlp"; then
  fixhlp
fi

if test x"$FORMAT" = "xguide"; then
  fixguide
fi

if test x"$FORMAT" = "xpdf"; then
  fixpdf
fi

if test x"$FORMAT" = "xipf"; then
  fixipf
fi
