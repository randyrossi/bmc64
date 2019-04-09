#!/bin/sh

#
# autogen.sh - generate the auto* files of VICE
#
# Written by
#  Spiro Trikaliotis <spiro.trikaliotis@gmx.de>
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

# minimum autoconf version required
ACONF_VERSION_REQ_MAJ=2
ACONF_VERSION_REQ_MIN=60

# minimum automake version required (1.10.2 was the lowest version above 1.9 I
# could test against)
AMAKE_VERSION_REQ_MAJ=1
AMAKE_VERSION_REQ_MIN=10
AMAKE_VERSION_REQ_REV=2


generate_configure_in() {
    # $1 - major automake version
    # $2 - minor automake version
    # $3 - build

    configure_needs_ac=no
    if [ $1 -gt 1 ]; then
        configure_needs_ac=yes
    else
        if [ $2 -gt 12 ]; then
            configure_needs_ac=yes
        fi
    fi

    if test x"$configure_needs_ac" = "xyes"; then
        sed s/AM_CONFIG_HEADER/AC_CONFIG_HEADERS/g <configure.proto >configure.ac
    else
        cp configure.proto configure.ac
    fi
}


get_autoconf_version() {
    # $1 - "autoconf"
    # $2 - "(GNU"
    # $3 - "Autconf)"
    # $4 - version

    autoconf_version=$4
}


# Check minimum required autoconf version against used autoconf
check_autoconf_version() {
    # $1 - version string ("M.m")
    aconf_maj=`echo "$1" | cut -d '.' -f 1`
    aconf_min=`echo "$1" | cut -d '.' -f 2`
    version_ok=false

    echo -n "Checking autoconf version >= $ACONF_VERSION_REQ_MAJ.$ACONF_VERSION_REQ_MIN .. "

    if test "$aconf_maj" -gt "$ACONF_VERSION_REQ_MAJ"; then
        version_ok=true
    elif test "$aconf_maj" -eq "$ACONF_VERSION_REQ_MAJ"; then
        if test "$aconf_min" -ge "$ACONF_VERSION_REQ_MIN"; then
            version_ok=true
        fi
    fi

    if test x"$version_ok" = x"true"; then
        echo "OK: $autoconf_version"
    else
        echo "failed: $autoconf_version, please update autoconf."
        exit 1
    fi
}


get_automake_version() {
    # $1 - "automake"
    # $2 - "(GNU"
    # $3 - "Automake)"
    # $4 - version

    automake_version=$4
}


# Check minimum required automake version against used automake
check_automake_version() {
    # $1 version string ("M.m.r")
    amake_maj=`echo "$1" | cut -d '.' -f 1`
    amake_min=`echo "$1" | cut -d '.' -f 2`
    amake_rev=`echo "$1" | cut -d '.' -f 3`
    version_ok=false

    echo -n "Checking automake version >= $AMAKE_VERSION_REQ_MAJ.$AMAKE_VERSION_REQ_MIN.$AMAKE_VERSION_REQ_REV .. "

    if test "$amake_maj" -gt "$AMAKE_VERSION_REQ_MAJ"; then
        version_ok=true
    elif test "$amake_maj" -eq "$AMAKE_VERSION_REQ_MAJ"; then
        if test "$amake_min" -gt "$AMAKE_VERSION_REQ_MIN"; then
            version_ok=true
        elif test "$amake_min" -eq "$AMAKE_VERSION_REQ_MIN"; then
            if test "$amake_rev" -ge "$AMAKE_VERSION_REQ_REV"; then
                version_ok=true
            fi
        fi
    fi

    if test x"$version_ok" = x"true"; then
        echo "OK: $automake_version"
    else
        echo "failed: $automake_version, please update automake."
        exit 1
    fi
}


do_command() {
    # $1 - command
    # $2 - options

    echo `pwd`: $1 $2
    $1 $2 > .$1.out 2>&1
    ret=$?
    if [ ! $ret = 0 ] ; then
        echo "ERROR: $1 failed in `pwd`"
        echo "check '.$1.out' for details"
        exit 1
    fi
}

do_aclocal() {
    do_command aclocal
}

do_autoconf() {
    do_command autoconf -f
}

do_autoheader() {
    if [ -e configure.in ]; then
        if [ ! x"`sed -ne "s/.*AM_CONFIG_HEADER\((.*)\).*/\1/p" configure.in`" = x ]; then
            do_command autoheader
        fi
    else
        do_command autoheader
    fi
}

do_automake() {
    do_command automake "-a -c" # " -f"
}

buildfiles() {
    FILES_TO_REMEMBER="INSTALL"

    # Save some files which should not be overwritten

    if [ -f configure.ac ] || [ -f configure.in ]; then

        for A in $FILES_TO_REMEMBER; do
            [ -e "$A" ] && mv -f "$A" "$A.backup"
        done

        do_aclocal

        do_autoconf
        do_autoheader
        do_automake

        # Restore the files which should not be overwritten

        for A in $FILES_TO_REMEMBER; do
            [ -e "$A.backup" ] && mv -f "$A.backup" "$A"
        done
    fi
}

# Script entry point

autoconf_line=`autoconf --version`
if test x"$autoconf_line" = "x"; then
    echo "No autoconf installed"
    exit 1
fi
get_autoconf_version $autoconf_line
check_autoconf_version $autoconf_version

automake_line=`automake --version`
if test x"$automake_line" = "x"; then
    echo "No automake installed"
    exit 1
fi
get_automake_version $automake_line
check_automake_version $automake_version
old_IFS=$IFS
IFS="."
generate_configure_in $automake_version
IFS=$old_IFS

SUBDIRECTORIES=`sed -ne "s/.*AC_CONFIG_SUBDIRS(\(.*\)).*/\1/p" configure.ac`

for A in $SUBDIRECTORIES; do
    (
    cd $A
    buildfiles
    )
done

buildfiles

if [ x"$1" = x"--dist" ]; then

    ./configure
    (cd src/monitor/; make mon_lex.c mon_parse.c)

    (cd po; make cat-id-tbl.c)

    SVN_ADD_FILES="configure src/config.h.in config.guess config.sub src/monitor/mon_parse.c src/monitor/mon_parse.h src/monitor/mon_lex.c src/resid/depcomp src/resid/mkinstalldirs src/resid/missing src/resid/install-sh doc/texinfo.tex po/cat-id-tbl.c"
    SVN_ADD_MAKEFILES="`find . -name Makefile.in`"

    svn add $SVN_ADD_FILES $SVN_ADD_MAKEFILES

fi

