#!/bin/bash

#
# build-gtk.sh - build the gtk+ toolkit on macs for the gtk+ version of VICE
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
# call this function twice: one for each architecture
#

# get build tools
SCRIPT_DIR="`dirname \"$0\"`"
. "$SCRIPT_DIR/build-inc.sh"

# parse args
parse_args "$@"

# create target dirs
make_dirs bin lib include man share

# setup compiler environment
USE_X11=1
set_compiler_env

export COMPILE_IN_SRC=1

echo "===== gtk+ build $BUILD_TAG ====="

# ----- Tool Libs -----
# gettext
configure_make_install gettext-0.17.tar.gz gettext-0.17 lib/libintl.a install \
                       "http://www.gnu.org/software/gettext/" \
                       "--disable-java --disable-csharp --disable-libasprintf"

# pkg-config
configure_make_install pkg-config-0.23.tar.gz pkg-config-0.23 bin/pkg-config install \
                       "http://pkgconfig.freedesktop.org/wiki/"

# jpeg
configure_make_install jpegsrc.v6b.tar.gz jpeg-6b lib/libjpeg.a install-lib \
                       "http://www.ijg.org/files/"
# png
configure_make_install libpng-1.2.24.tar.bz2 libpng-1.2.24 lib/libpng12.a install \
                       "http://www.libpng.org/pub/png/libpng.html" \
                       "--disable-shared"
# tiff
configure_make_install tiff-3.8.2.tar.gz tiff-3.8.2 lib/libtiff.a install \
                       "http://www.libtiff.org/" \
                       "--disable-shared --disable-cxx"

# freetype
configure_make_install freetype-2.3.5.tar.bz2 freetype-2.3.5 lib/libfreetype.a install \
                       "http://www.freetype.org/"
# libxml2
configure_make_install libxml2-2.6.31.tar.gz libxml2-2.6.31 lib/libxml2.a install \
                       "http://xmlsoft.org/" \
                       "--without-python"
# fontconfig
configure_make_install fontconfig-2.5.0.tar.gz fontconfig-2.5.0 lib/libfontconfig.a install \
                       "http://www.fontconfig.org/wiki/"

# ----- Gtk+ Libs -----
# glib
configure_make_install glib-2.14.5.tar.bz2 glib-2.14.5 lib/libglib-2.0.dylib install \
                       "http://www.gtk.org/"

# cairo
configure_make_install cairo-1.4.14.tar.gz cairo-1.4.14 lib/libcairo.dylib install \
                       "http://cairographics.org/"
# pango
configure_make_install pango-1.18.4.tar.bz2 pango-1.18.4 lib/libpango-1.0.dylib install \
                       "http://www.gtk.org/" \
                       "--with-included-modules=yes --with-dynamic-modules=no"
# atk
configure_make_install atk-1.9.1.tar.bz2 atk-1.9.1 lib/libatk-1.0.dylib install \
                       "http://www.gtk.org/"
# gtk+
export ac_cv_path_CUPS_CONFIG=no # cups does not cross-compile
configure_make_install gtk+-2.12.7.tar.bz2 gtk+-2.12.7 lib/libgtk-x11-2.0.dylib install \
                       "http://www.gtk.org/" \
                       "--disable-modules --with-included-loaders=yes"

# gtkglext
configure_make_install gtkglext-1.2.0.tar.bz2 gtkglext-1.2.0 lib/libgtkglext-x11-1.0.dylib install \
                       "gtkglext-1.2.0.tar.bz2" \
                       "--with-gl-libdir=/System/Library/Frameworks/OpenGL.framework/Versions/Current/Libraries/"

echo "===== gtk+ ready $BUILD_TAG ====="

                       