#!/bin/sh

#
# make-bindist.sh - make binary distribution for the QNX 6.x port
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
# Usage: make-bindist.sh <strip> <vice-version> <prefix> <cross> <--enable-arch> <zip|nozip> <x64sc-included> <topsrcdir> <make-command>
#                         $1      $2             $3       $4      $5              $6          $7               $8          $9
#

STRIP=$1
VICEVERSION=$2
PREFIX=$3
CROSS=$4
ENABLEARCH=$5
ZIPKIND=$6
X64SC=$7
TOPSRCDIR=$8
MAKECOMMAND=$9

if test x"$PREFIX" != "x/opt"; then
  echo Error: installation path is not /opt
  exit 1
fi

if test x"$CROSS" = "xtrue"; then
  echo Error: \"make bindist\" for QNX 6 can only be done on QNX 6
  exit 1
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

echo Generating QNX 6 port binary distribution.
rm -f -r VICE-$VICEVERSION
curdir=`pwd`
$MAKECOMMAND prefix=$curdir/VICE-$VICEVERSION/opt VICEDIR=$curdir/VICE-$VICEVERSION/opt/lib/vice install
for i in $EXECUTABLES
do
  $STRIP VICE-$VICEVERSION/opt/bin/$i
done
if test x"$ZIPKIND" = "xzip"; then
  gcc $TOPSRCDIR/src/arch/unix/qnx6/getsize.c -o ./getsize
  gcc $TOPSRCDIR/src/arch/unix/qnx6/getlibs.c -o ./getlibs
  mkdir -p public/VICE/core-$VICEVERSION/x86/opt/bin
  mv VICE-$VICEVERSION/opt/bin/x* public/VICE/core-$VICEVERSION/x86/opt/bin
  mv VICE-$VICEVERSION/opt/bin/c1541 public/VICE/core-$VICEVERSION/x86/opt/bin
  mv VICE-$VICEVERSION/opt/bin/cartconv public/VICE/core-$VICEVERSION/x86/opt/bin
  mv VICE-$VICEVERSION/opt/bin/petcat public/VICE/core-$VICEVERSION/x86/opt/bin

  current_date=`date +%Y/%m/%d`

cat >manifest.01 <<_END
<?xml version="1.0"?>
<RDF:RDF xmlns:RDF="http://www.w3.org" xmlns:QPM="http://www.qnx.com">
   <RDF:Description about="http://www.qnx.com">
      <QPM:PackageManifest>
         <QPM:PackageDescription>
            <QPM:PackageType>Application</QPM:PackageType>
            <QPM:PackageName>VICE-$VICEVERSION-x86-public</QPM:PackageName>
_END

echo >manifest.02 "            <QPM:PackageSize/>"

cat >manifest.03 <<_END
            <QPM:PackageReleaseNumber>1</QPM:PackageReleaseNumber>
            <QPM:PackageReleaseDate>$current_date</QPM:PackageReleaseDate>
            <QPM:PackageRepository/>
            <QPM:FileVersion>2.0</QPM:FileVersion>
         </QPM:PackageDescription>

         <QPM:ProductDescription>
            <QPM:ProductName>VICE for x86</QPM:ProductName>
_END

  ls -l -R public/VICE/core-$VICEVERSION/x86/opt >size.tmp
  manifest_size=`./getsize size.tmp`
  rm -f -r size.tmp
  echo >manifest.04 "            <QPM:ProductSize>$manifest_size</QPM:ProductSize>"

  echo >manifest.05 "            <QPM:ProductIdentifier>VICE-host_x86</QPM:ProductIdentifier>"

  cat >manifest.06 <<_END
            <QPM:ProductEmail>vice-emu-mail@lists.sourceforge.net</QPM:ProductEmail>
            <QPM:VendorName>Public</QPM:VendorName>
            <QPM:VendorInstallName>public</QPM:VendorInstallName>
            <QPM:VendorURL>http://vice-emu.sourceforge.net</QPM:VendorURL>
            <QPM:VendorEmbedURL/>
            <QPM:VendorEmail>vice-emu-mail@lists.sourceforge.net</QPM:VendorEmail>
            <QPM:AuthorName>The VICE Team</QPM:AuthorName>
            <QPM:AuthorURL>http://vice-emu.sourceforge.net</QPM:AuthorURL>
            <QPM:AuthorEmbedURL/>
            <QPM:AuthorEmail>vice-emu-mail@lists.sourceforge.net</QPM:AuthorEmail>
            <QPM:ProductIconSmall>http://www.trikaliotis.net/vicekb/common/vice-logo.png</QPM:ProductIconSmall>
            <QPM:ProductIconLarge>http://vice-emu.sourceforge.net/images/vice-logo.jpg</QPM:ProductIconLarge>
            <QPM:ProductDescriptionShort>The Versatile Commodore Emulator</QPM:ProductDescriptionShort>
            <QPM:ProductDescriptionLong>VICE is a program that runs on a Unix, MS-DOS, Win32, OS/2, Acorn RISC OS, QNX 6.x, AmigaOS or BeOS machine and executes programs intended for the old 8-bit computers. The current version emulates the C64, the C64DTV, the C128, the VIC20, all the PET models (except the SuperPET 9000, which is out of line anyway), the PLUS4 and the CBM-II (aka C610).</QPM:ProductDescriptionLong>
            <QPM:ProductDescriptionURL>http://vice-emu.sourceforge.net</QPM:ProductDescriptionURL>
            <QPM:ProductDescriptionEmbedURL/>
_END

echo >manifest.07 "            <QPM:InstallPath>public/VICE/core-$VICEVERSION/x86</QPM:InstallPath>"

cat >manifest.08 <<_END
         </QPM:ProductDescription>

         <QPM:ReleaseDescription>
            <QPM:ReleaseDate>$current_date</QPM:ReleaseDate>
            <QPM:ReleaseVersion>$VICEVERSION</QPM:ReleaseVersion>
            <QPM:ReleaseUrgency>High</QPM:ReleaseUrgency>
            <QPM:ReleaseStability>Stable</QPM:ReleaseStability>
            <QPM:ReleaseNoteMinor/>
            <QPM:ReleaseNoteMajor/>
            <QPM:ReleaseBuild>1</QPM:ReleaseBuild>
            <QPM:ExcludeCountries>
               <QPM:Country/>
            </QPM:ExcludeCountries>

            <QPM:ReleaseCopyright>GNU General Public License</QPM:ReleaseCopyright>
         </QPM:ReleaseDescription>

         <QPM:LicenseUrl>rep://VICE-$VICEVERSION-public.repdata/LicenseUrl/COPYING</QPM:LicenseUrl>
         <QPM:ProductKeyUrl/>
         <QPM:RelationshipDescription>
_END

cat >manifest.09 <<_END
            <QPM:RelationshipType>component</QPM:RelationshipType>
            <QPM:ComponentType>Required</QPM:ComponentType>
            <QPM:ComponentParent>VICE</QPM:ComponentParent>
_END

cat >manifest.10 <<_END
            <QPM:ComponentFilter/>
         </QPM:RelationshipDescription>

         <QPM:ContentDescription>
            <QPM:ContentTopic>Games and Diversions/Game Emulatiors</QPM:ContentTopic>
            <QPM:ContentKeyword>c64,c128,vic20,plus4,c16,cbm510,cbm610,c510,c610,cbm,commodore,emulator,pet</QPM:ContentKeyword>
_END

echo >manifest.11 "            <QPM:Processor>x86</QPM:Processor>"

cat >manifest.12 <<_END
            <QPM:TargetProcessor/>
            <QPM:TargetOS>qnx6</QPM:TargetOS>
            <QPM:HostOS>none</QPM:HostOS>
            <QPM:DisplayEnvironment>X11R6</QPM:DisplayEnvironment>
            <QPM:TargetAudience>User</QPM:TargetAudience>
_END

  ./getlibs public/VICE/core-$VICEVERSION/x86/opt/bin/x64 >manifest.13

cat >manifest.14 <<_END
         </QPM:ContentDescription>

         <QPM:ProductInstallationDependencies>
            <QPM:ProductRequirements>This package requires the following libraries to operate correctly : libintl, libSDL, libpng, libX11.</QPM:ProductRequirements>
         </QPM:ProductInstallationDependencies>
_END

cat >manifest.15 <<_END
         <QPM:QNXLicenseDescription/>
      </QPM:PackageManifest>

      <QPM:FileManifest>
         <QPM:Listing>
            <QPM:Dir name="">
               <QPM:Dir name="opt">
                  <QPM:Dir name="bin">
                     <QPM:File>c1541</QPM:File>
                     <QPM:File>petcat</QPM:File>
                     <QPM:File>cartconv</QPM:File>
                     <QPM:File>vsid</QPM:File>
                     <QPM:File>x128</QPM:File>
                     <QPM:File>x64</QPM:File>
                     <QPM:File>xscpu64</QPM:File>
                     <QPM:File>x64dtv</QPM:File>
_END

if test x"$X64SC" = "xyes"; then
  cat >>manifest.15 <<_END
                     <QPM:File>x64sc</QPM:File>
_END
fi

cat >>manifest.15 <<_END
                     <QPM:File>xcbm2</QPM:File>
                     <QPM:File>xcbm5x0</QPM:File>
                     <QPM:File>xpet</QPM:File>
                     <QPM:File>xplus4</QPM:File>
                     <QPM:File>xvic</QPM:File>
                  </QPM:Dir>
               </QPM:Dir>
            </QPM:Dir>
         </QPM:Listing>
      </QPM:FileManifest>
   </RDF:Description>
</RDF:RDF>
_END

  cat >public/VICE/core-$VICEVERSION/x86/MANIFEST manifest.01 manifest.02 \
      manifest.03 manifest.04 manifest.05 manifest.06 manifest.07 manifest.08 \
      manifest.09 manifest.10 manifest.11 manifest.12 manifest.13 manifest.14 \
      manifest.15

  tar cf VICE-x86.tar public
  gzip VICE-x86.tar
  mv VICE-x86.tar.gz VICE-$VICEVERSION-x86-public.qpk

  ls -l -R VICE-$VICEVERSION-x86-public.qpk >size.tmp
  manifest_size=`./getsize size.tmp` 
  rm -f -r size.tmp

  echo >manifest.04 "            <QPM:PackageSize>$manifest_size</QPM:PackageSize>"

  cat >VICE-$VICEVERSION-x86-public.qpm manifest.01 manifest.02 manifest.03 manifest.04 \
      manifest.05 manifest.06 manifest.07 manifest.08 manifest.09 manifest.10 \
      manifest.11 manifest.12 manifest.13 manifest.14 manifest.15

  rm -f -r public

  mkdir -p public/VICE/core-$VICEVERSION
  mv VICE-$VICEVERSION/opt public/VICE/core-$VICEVERSION

  mkdir -p VICE-$VICEVERSION-public.repdata/LicenseUrl
  cp public/VICE/core-$VICEVERSION/opt/lib/vice/doc/COPYING VICE-$VICEVERSION-public.repdata/LicenseUrl

  echo >manifest.02 "            <QPM:PackageSize/>"

  ls -l -R public/VICE/core-$VICEVERSION/opt >size.tmp
  manifest_size=`./getsize size.tmp`
  rm -f -r size.tmp
  echo >manifest.04 "            <QPM:ProductSize>$manifest_size</QPM:ProductSize>"

  echo >manifest.05 "            <QPM:ProductIdentifier>VICE</QPM:ProductIdentifier>"

  echo >manifest.07 "            <QPM:InstallPath>public/VICE/core-$VICEVERSION</QPM:InstallPath>"

  cat >manifest.09 <<_END
            <QPM:RelationshipType>core</QPM:RelationshipType>
            <QPM:ComponentType>Required</QPM:ComponentType>
            <QPM:ComponentParent/>
_END

  echo >manifest.11 "            <QPM:Processor/>"

  cat >manifest.15 <<_END
         <QPM:ProductInstallationProcedure>
            <QPM:Script>
               <QPM:ScriptName>/usr/photon/bin/launchmenu_notify</QPM:ScriptName>
               <QPM:ScriptTiming>Post</QPM:ScriptTiming>
               <QPM:ScriptType>Use</QPM:ScriptType>
            </QPM:Script>

            <QPM:Script>
               <QPM:ScriptName>/usr/photon/bin/launchmenu_notify</QPM:ScriptName>
               <QPM:ScriptTiming>Post</QPM:ScriptTiming>
               <QPM:ScriptType>Unuse</QPM:ScriptType>
            </QPM:Script>
         </QPM:ProductInstallationProcedure>

         <QPM:QNXLicenseDescription/>
      </QPM:PackageManifest>

      <QPM:FileManifest>
         <QPM:PkgInclude>
            <QPM:Item>\$(PROCESSOR)/MANIFEST</QPM:Item>
         </QPM:PkgInclude>

         <QPM:Listing>
            <QPM:Dir name="">
               <QPM:Dir name="opt">
                  <QPM:Union link="../\$(PROCESSOR)/opt/bin">bin</QPM:Union>
                  <QPM:Dir name="bin">
                     <QPM:File>vsid</QPM:File>
                  </QPM:Dir>

                  <QPM:Dir name="info">
                     <QPM:File>vice.info</QPM:File>
                     <QPM:File>vice.txt</QPM:File>
                  </QPM:Dir>

                  <QPM:Dir name="lib">
                     <QPM:Dir name="locale">
                        <QPM:Dir name="da">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="de">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="es">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="fr">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="hu">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="it">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="ko">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="nl">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="pl">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="ru">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="sv">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>

                        <QPM:Dir name="tr">
                           <QPM:Dir name="LC_MESSAGES">
                              <QPM:File>vice.mo</QPM:File>
                           </QPM:Dir>
                        </QPM:Dir>
                     </QPM:Dir>

                     <QPM:Dir name="vice">
                        <QPM:Dir name="C128">
                           <QPM:File>basic64</QPM:File>
                           <QPM:File>basichi</QPM:File>
                           <QPM:File>basiclo</QPM:File>
                           <QPM:File>c64hq.vpl</QPM:File>
                           <QPM:File>c64s.vpl</QPM:File>
                           <QPM:File>ccs64.vpl</QPM:File>
                           <QPM:File>chargch</QPM:File>
                           <QPM:File>chargde</QPM:File>
                           <QPM:File>chargen</QPM:File>
                           <QPM:File>chargfr</QPM:File>
                           <QPM:File>chargse</QPM:File>
                           <QPM:File>community-colors.vpl</QPM:File>
                           <QPM:File>deekay.vpl</QPM:File>
                           <QPM:File>default.vrs</QPM:File>
                           <QPM:File>frodo.vpl</QPM:File>
                           <QPM:File>godot.vpl</QPM:File>
                           <QPM:File>kernal</QPM:File>
                           <QPM:File>kernal64</QPM:File>
                           <QPM:File>kernalch</QPM:File>
                           <QPM:File>kernalde</QPM:File>
                           <QPM:File>kernalfi</QPM:File>
                           <QPM:File>kernalfr</QPM:File>
                           <QPM:File>kernalit</QPM:File>
                           <QPM:File>kernalno</QPM:File>
                           <QPM:File>kernalse</QPM:File>
                           <QPM:File>pc64.vpl</QPM:File>
                           <QPM:File>pepto-ntsc.vpl</QPM:File>
                           <QPM:File>pepto-ntsc-sony.vpl</QPM:File>
                           <QPM:File>pepto-pal.vpl</QPM:File>
                           <QPM:File>pepto-palold.vpl</QPM:File>
                           <QPM:File>ptoing.vpl</QPM:File>
                           <QPM:File>rgb.vpl</QPM:File>
                           <QPM:File>vdc_comp.vpl</QPM:File>
                           <QPM:File>vdc_deft.vpl</QPM:File>
                           <QPM:File>vice.vpl</QPM:File>
                           <QPM:File>x11_pos.vkm</QPM:File>
                           <QPM:File>x11_sym.vkm</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="C64">
                           <QPM:File>basic</QPM:File>
                           <QPM:File>c64hq.vpl</QPM:File>
                           <QPM:File>c64mem.sym</QPM:File>
                           <QPM:File>c64s.vpl</QPM:File>
                           <QPM:File>ccs64.vpl</QPM:File>
                           <QPM:File>chargen</QPM:File>
                           <QPM:File>community-colors.vpl</QPM:File>
                           <QPM:File>deekay.vpl</QPM:File>
                           <QPM:File>default.vrs</QPM:File>
                           <QPM:File>edkernal</QPM:File>
                           <QPM:File>frodo.vpl</QPM:File>
                           <QPM:File>godot.vpl</QPM:File>
                           <QPM:File>gskernal</QPM:File>
                           <QPM:File>jpchrgen</QPM:File>
                           <QPM:File>jpkernal</QPM:File>
                           <QPM:File>kernal</QPM:File>
                           <QPM:File>pc64.vpl</QPM:File>
                           <QPM:File>pepto-ntsc.vpl</QPM:File>
                           <QPM:File>pepto-ntsc-sony.vpl</QPM:File>
                           <QPM:File>pepto-pal.vpl</QPM:File>
                           <QPM:File>pepto-palold.vpl</QPM:File>
                           <QPM:File>ptoing.vpl</QPM:File>
                           <QPM:File>rgb.vpl</QPM:File>
                           <QPM:File>sxkernal</QPM:File>
                           <QPM:File>vice.vpl</QPM:File>
                           <QPM:File>x11_pos.vkm</QPM:File>
                           <QPM:File>x11_pos_de.vkm</QPM:File>
                           <QPM:File>x11_sym.vkm</QPM:File>
                           <QPM:File>x11_sym_de.vkm</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="SCPU64">
                           <QPM:File>c64hq.vpl</QPM:File>
                           <QPM:File>c64s.vpl</QPM:File>
                           <QPM:File>ccs64.vpl</QPM:File>
                           <QPM:File>chargen</QPM:File>
                           <QPM:File>community-colors.vpl</QPM:File>
                           <QPM:File>deekay.vpl</QPM:File>
                           <QPM:File>default.vrs</QPM:File>
                           <QPM:File>frodo.vpl</QPM:File>
                           <QPM:File>godot.vpl</QPM:File>
                           <QPM:File>jpchrgen</QPM:File>
                           <QPM:File>pc64.vpl</QPM:File>
                           <QPM:File>pepto-ntsc.vpl</QPM:File>
                           <QPM:File>pepto-ntsc-sony.vpl</QPM:File>
                           <QPM:File>pepto-pal.vpl</QPM:File>
                           <QPM:File>pepto-palold.vpl</QPM:File>
                           <QPM:File>ptoing.vpl</QPM:File>
                           <QPM:File>rgb.vpl</QPM:File>
                           <QPM:File>scpu64</QPM:File>
                           <QPM:File>scpu64mem.sym</QPM:File>
                           <QPM:File>vice.vpl</QPM:File>
                           <QPM:File>x11_pos.vkm</QPM:File>
                           <QPM:File>x11_pos_de.vkm</QPM:File>
                           <QPM:File>x11_sym.vkm</QPM:File>
                           <QPM:File>x11_sym_de.vkm</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="C64DTV">
                           <QPM:File>basic</QPM:File>
                           <QPM:File>c64mem.sym</QPM:File>
                           <QPM:File>chargen</QPM:File>
                           <QPM:File>default.vrs</QPM:File>
                           <QPM:File>dtvrom.bin</QPM:File>
                           <QPM:File>kernal</QPM:File>
                           <QPM:File>spiff.vpl</QPM:File>
                           <QPM:File>x11_pos.vkm</QPM:File>
                           <QPM:File>x11_pos_de.vkm</QPM:File>
                           <QPM:File>x11_sym.vkm</QPM:File>
                           <QPM:File>x11_sym_de.vkm</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="CBM-II">
                           <QPM:File>amber.vpl</QPM:File>
                           <QPM:File>basic.128</QPM:File>
                           <QPM:File>basic.256</QPM:File>
                           <QPM:File>basic.500</QPM:File>
                           <QPM:File>c64hq.vpl</QPM:File>
                           <QPM:File>c64s.vpl</QPM:File>
                           <QPM:File>ccs64.vpl</QPM:File>
                           <QPM:File>chargen.500</QPM:File>
                           <QPM:File>chargen.600</QPM:File>
                           <QPM:File>chargen.700</QPM:File>
                           <QPM:File>community-colors.vpl</QPM:File>
                           <QPM:File>deekay.vpl</QPM:File>
                           <QPM:File>frodo.vpl</QPM:File>
                           <QPM:File>godot.vpl</QPM:File>
                           <QPM:File>green.vpl</QPM:File>
                           <QPM:File>kernal</QPM:File>
                           <QPM:File>kernal.500</QPM:File>
                           <QPM:File>pc64.vpl</QPM:File>
                           <QPM:File>pepto-ntsc.vpl</QPM:File>
                           <QPM:File>pepto-ntsc-sony.vpl</QPM:File>
                           <QPM:File>pepto-pal.vpl</QPM:File>
                           <QPM:File>pepto-palold.vpl</QPM:File>
                           <QPM:File>ptoing.vpl</QPM:File>
                           <QPM:File>rgb.vpl</QPM:File>
                           <QPM:File>rom128h.vrs</QPM:File>
                           <QPM:File>rom128l.vrs</QPM:File>
                           <QPM:File>rom256h.vrs</QPM:File>
                           <QPM:File>rom256l.vrs</QPM:File>
                           <QPM:File>rom500.vrs</QPM:File>
                           <QPM:File>vice.vpl</QPM:File>
                           <QPM:File>white.vpl</QPM:File>
                           <QPM:File>x11_sym.vkm</QPM:File>
                           <QPM:File>x11_sym_de.vkm</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="DRIVES">
                           <QPM:File>d1541II</QPM:File>
                           <QPM:File>d1571cr</QPM:File>
                           <QPM:File>dos1001</QPM:File>
                           <QPM:File>dos1540</QPM:File>
                           <QPM:File>dos1541</QPM:File>
                           <QPM:File>dos1551</QPM:File>
                           <QPM:File>dos1570</QPM:File>
                           <QPM:File>dos1571</QPM:File>
                           <QPM:File>dos1581</QPM:File>
                           <QPM:File>dos2031</QPM:File>
                           <QPM:File>dos2040</QPM:File>
                           <QPM:File>dos3040</QPM:File>
                           <QPM:File>dos4040</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="PET">
                           <QPM:File>amber.vpl</QPM:File>
                           <QPM:File>basic1</QPM:File>
                           <QPM:File>basic2</QPM:File>
                           <QPM:File>basic4</QPM:File>
                           <QPM:File>characters.901640-01.bin</QPM:File>
                           <QPM:File>chargen</QPM:File>
                           <QPM:File>chargen.de</QPM:File>
                           <QPM:File>edit1g</QPM:File>
                           <QPM:File>edit2b</QPM:File>
                           <QPM:File>edit2g</QPM:File>
                           <QPM:File>edit4b40</QPM:File>
                           <QPM:File>edit4b80</QPM:File>
                           <QPM:File>edit4g40</QPM:File>
                           <QPM:File>green.vpl</QPM:File>
                           <QPM:File>hre.vrs</QPM:File>
                           <QPM:File>hre-9000.324992-02.bin</QPM:File>
                           <QPM:File>hre-a000.324993-02.bin</QPM:File>
                           <QPM:File>kernal1</QPM:File>
                           <QPM:File>kernal2</QPM:File>
                           <QPM:File>kernal4</QPM:File>
                           <QPM:File>rom1g.vrs</QPM:File>
                           <QPM:File>rom2b.vrs</QPM:File>
                           <QPM:File>rom2g.vrs</QPM:File>
                           <QPM:File>rom4b40.vrs</QPM:File>
                           <QPM:File>rom4b80.vrs</QPM:File>
                           <QPM:File>rom4g40.vrs</QPM:File>
                           <QPM:File>romsuperpet.vrs</QPM:File>
                           <QPM:File>waterloo-a000.901898-01.bin</QPM:File>
                           <QPM:File>waterloo-b000.901898-02.bin</QPM:File>
                           <QPM:File>waterloo-c000.901898-03.bin</QPM:File>
                           <QPM:File>waterloo-d000.901898-04.bin</QPM:File>
                           <QPM:File>waterloo-e000.901897-01.bin</QPM:File>
                           <QPM:File>waterloo-f000.901898-05.bin</QPM:File>
                           <QPM:File>white.vpl</QPM:File>
                           <QPM:File>x11_bude_pos.vkm</QPM:File>
                           <QPM:File>x11_bude_pos_de.vkm</QPM:File>
                           <QPM:File>x11_bude_sym.vkm</QPM:File>
                           <QPM:File>x11_bude_sym_de.vkm</QPM:File>
                           <QPM:File>x11_buuk_pos.vkm</QPM:File>
                           <QPM:File>x11_buuk_pos_de.vkm</QPM:File>
                           <QPM:File>x11_buuk_sym.vkm</QPM:File>
                           <QPM:File>x11_buuk_sym_de.vkm</QPM:File>
                           <QPM:File>x11_grus_pos.vkm</QPM:File>
                           <QPM:File>x11_grus_pos_de.vkm</QPM:File>
                           <QPM:File>x11_grus_sym.vkm</QPM:File>
                           <QPM:File>x11_grus_sym_de.vkm</QPM:File>
                           <QPM:File>x11_sym.vkm</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="PLUS4">
                           <QPM:File>3plus1hi</QPM:File>
                           <QPM:File>3plus1lo</QPM:File>
                           <QPM:File>basic</QPM:File>
                           <QPM:File>c2lo.364</QPM:File>
                           <QPM:File>default.vrs</QPM:File>
                           <QPM:File>kernal</QPM:File>
                           <QPM:File>kernal.005</QPM:File>
                           <QPM:File>kernal.232</QPM:File>
                           <QPM:File>kernal.364</QPM:File>
                           <QPM:File>vice.vpl</QPM:File>
                           <QPM:File>x11_pos.vkm</QPM:File>
                           <QPM:File>x11_sym.vkm</QPM:File>
                           <QPM:File>x11_sym_de.vkm</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="PRINTER">
                           <QPM:File>1520.vpl</QPM:File>
                           <QPM:File>cbm1526</QPM:File>
                           <QPM:File>mps801</QPM:File>
                           <QPM:File>mps803</QPM:File>
                           <QPM:File>mps803.vpl</QPM:File>
                           <QPM:File>nl10.vpl</QPM:File>
                           <QPM:File>nl10-cbm</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="VIC20">
                           <QPM:File>basic</QPM:File>
                           <QPM:File>chargen</QPM:File>
                           <QPM:File>default.vrs</QPM:File>
                           <QPM:File>kernal</QPM:File>
                           <QPM:File>mike-ntsc.vpl</QPM:File>
                           <QPM:File>mike-pal.vpl</QPM:File>
                           <QPM:File>vice.vpl</QPM:File>
                           <QPM:File>x11_pos.vkm</QPM:File>
                           <QPM:File>x11_sym.vkm</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="doc">
                           <QPM:File>Android-Howto.txt</QPM:File>
                           <QPM:File>CIA-README.txt</QPM:File>
                           <QPM:File>CIPS2008.jpg</QPM:File>
                           <QPM:File>COPYING</QPM:File>
                           <QPM:File>Documentation-Howto.txt</QPM:File>
                           <QPM:File>Doxygen-Howto.txt</QPM:File>
                           <QPM:File>Linux-Mingw32-Howto.txt</QPM:File>
                           <QPM:File>Linux-Native-Howto.txt</QPM:File>
                           <QPM:File>MSDOS-Ethernet-Howto.txt</QPM:File>
                           <QPM:File>MSDOS-Howto.txt</QPM:File>
                           <QPM:File>MacOSX-Howto.txt</QPM:File>
                           <QPM:File>MacOSX-ResourceTree.txt</QPM:File>
                           <QPM:File>Minix-Howto.txt</QPM:File>
                           <QPM:File>NEWS</QPM:File>
                           <QPM:File>NLS-Howto.txt</QPM:File>
                           <QPM:File>OpenVMS-Howto.txt</QPM:File>
                           <QPM:File>OpenWatcom-Howto.txt</QPM:File>
                           <QPM:File>Readme-Amiga.txt</QPM:File>
                           <QPM:File>Readme-BeOS.txt</QPM:File>
                           <QPM:File>Readme-DOS.txt</QPM:File>
                           <QPM:File>Readme-MacOSX.txt</QPM:File>
                           <QPM:File>Readme-OS2.txt</QPM:File>
                           <QPM:File>Readme-SDL.txt</QPM:File>
                           <QPM:File>Readme-SDL2.txt</QPM:File>
                           <QPM:File>Readme-Unix.txt</QPM:File>
                           <QPM:File>SDL-Howto.txt</QPM:File>
                           <QPM:File>Win32-Cygwin-Howto.txt</QPM:File>
                           <QPM:File>Win32-MSVC-Howto.txt</QPM:File>
                           <QPM:File>Win32-Mingw32-Howto.txt</QPM:File>
                           <QPM:File>amigaos.html</QPM:File>
                           <QPM:File>beos.html</QPM:File>
                           <QPM:File>coding-guidelines.txt</QPM:File>
                           <QPM:File>hardware-sids.txt</QPM:File>
                           <QPM:File>iec-bus.txt</QPM:File>
                           <QPM:File>index.html</QPM:File>
                           <QPM:File>macosx.html</QPM:File>
                           <QPM:File>new.gif</QPM:File>
                           <QPM:File>ports-status.txt</QPM:File>
                           <QPM:File>qnx.html</QPM:File>
                           <QPM:File>sco.html</QPM:File>
                           <QPM:File>solaris.html</QPM:File>
                           <QPM:File>vice-logo-small.png</QPM:File>
                           <QPM:File>vice-logo.jpg</QPM:File>
                           <QPM:File>vice.chm</QPM:File>
                           <QPM:File>vice.css</QPM:File>
                           <QPM:File>vice.guide</QPM:File>
                           <QPM:File>vice.hlp</QPM:File>
                           <QPM:File>vice.inf</QPM:File>
                           <QPM:File>vice.info</QPM:File>
                           <QPM:File>vice.pdf</QPM:File>
                           <QPM:File>vice.txt</QPM:File>
                           <QPM:File>vice_1.html</QPM:File>
                           <QPM:File>vice_10.html</QPM:File>
                           <QPM:File>vice_11.html</QPM:File>
                           <QPM:File>vice_12.html</QPM:File>
                           <QPM:File>vice_13.html</QPM:File>
                           <QPM:File>vice_14.html</QPM:File>
                           <QPM:File>vice_15.html</QPM:File>
                           <QPM:File>vice_16.html</QPM:File>
                           <QPM:File>vice_17.html</QPM:File>
                           <QPM:File>vice_18.html</QPM:File>
                           <QPM:File>vice_19.html</QPM:File>
                           <QPM:File>vice_2.html</QPM:File>
                           <QPM:File>vice_20.html</QPM:File>
                           <QPM:File>vice_21.html</QPM:File>
                           <QPM:File>vice_22.html</QPM:File>
                           <QPM:File>vice_3.html</QPM:File>
                           <QPM:File>vice_4.html</QPM:File>
                           <QPM:File>vice_5.html</QPM:File>
                           <QPM:File>vice_6.html</QPM:File>
                           <QPM:File>vice_7.html</QPM:File>
                           <QPM:File>vice_8.html</QPM:File>
                           <QPM:File>vice_9.html</QPM:File>
                           <QPM:File>vice_toc.html</QPM:File>
                        </QPM:Dir>

                        <QPM:Dir name="fonts">
                           <QPM:File>CBM.ttf</QPM:File>
                           <QPM:File>fonts.dir</QPM:File>
                           <QPM:File>vice-cbm.bdf</QPM:File>
                           <QPM:File>vice-cbm.pcf</QPM:File>
                        </QPM:Dir>
                     </QPM:Dir>
                  </QPM:Dir>


                  <QPM:Dir name="man">
                     <QPM:Dir name="man1">
                        <QPM:File>c1541.1</QPM:File>
                        <QPM:File>cartconv.1</QPM:File>
                        <QPM:File>petcat.1</QPM:File>
                        <QPM:File>vice.1</QPM:File>
                     </QPM:Dir>
                  </QPM:Dir>
               </QPM:Dir>

               <QPM:Dir name="usr">
                  <QPM:Union link="../opt/info">info</QPM:Union>
                  <QPM:Union link="../opt/bin">bin</QPM:Union>
                  <QPM:Union link="../\$(PROCESSOR)/opt/bin">bin</QPM:Union>
                  <QPM:Dir name="lib">
                     <QPM:Dir name="locale">
                        <QPM:Dir name="da">
                           <QPM:Union link="../../../../opt/lib/locale/da/LC_MESSAGES">LC_MESSAGES</QPM:Union>
                        </QPM:Dir>

                        <QPM:Dir name="de">
                           <QPM:Union link="../../../../opt/lib/locale/de/LC_MESSAGES">LC_MESSAGES</QPM:Union>
                        </QPM:Dir>

                        <QPM:Dir name="fr">
                           <QPM:Union link="../../../../opt/lib/locale/fr/LC_MESSAGES">LC_MESSAGES</QPM:Union>
                        </QPM:Dir>

                        <QPM:Dir name="hu">
                           <QPM:Union link="../../../../opt/lib/locale/hu/LC_MESSAGES">LC_MESSAGES</QPM:Union>
                        </QPM:Dir>

                        <QPM:Dir name="it">
                           <QPM:Union link="../../../../opt/lib/locale/it/LC_MESSAGES">LC_MESSAGES</QPM:Union>
                        </QPM:Dir>

                        <QPM:Dir name="nl">
                           <QPM:Union link="../../../../opt/lib/locale/nl/LC_MESSAGES">LC_MESSAGES</QPM:Union>
                        </QPM:Dir>

                        <QPM:Dir name="pl">
                           <QPM:Union link="../../../../opt/lib/locale/pl/LC_MESSAGES">LC_MESSAGES</QPM:Union>
                        </QPM:Dir>

                        <QPM:Dir name="sv">
                           <QPM:Union link="../../../../opt/lib/locale/sv/LC_MESSAGES">LC_MESSAGES</QPM:Union>
                        </QPM:Dir>

                        <QPM:Dir name="tr">
                           <QPM:Union link="../../../../opt/lib/locale/tr/LC_MESSAGES">LC_MESSAGES</QPM:Union>
                        </QPM:Dir>
                     </QPM:Dir>

                     <QPM:Dir name="vice">
                        <QPM:Union link="../../../opt/lib/vice/fonts">fonts</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/doc">doc</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/VIC20">VIC20</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/PRINTER">PRINTER</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/PLUS4">PLUS4</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/PET">PET</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/DRIVES">DRIVES</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/CBM-II">CBM-II</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/C64">C64</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/C64DTV">C64DTV</QPM:Union>
                        <QPM:Union link="../../../opt/lib/vice/C128">C128</QPM:Union>
                     </QPM:Dir>
                  </QPM:Dir>

                  <QPM:Dir name="man">
                     <QPM:Union link="../../opt/man/man1">man1</QPM:Union>
                  </QPM:Dir>
               </QPM:Dir>
            </QPM:Dir>
         </QPM:Listing>
      </QPM:FileManifest>

      <QPM:Launch name="x64">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/x64"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="x64 (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/x64 -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xscpu64">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xscpu64"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xscpu64 (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xscpu64 -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="x64dtv">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/x64dtv"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="x64dtv (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/x64dtv -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

_END

if test x"$X64SC" = "xyes"; then
  cat >>manifest.15 <<_END
      <QPM:Launch name="x64sc">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/x64sc"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="x64sc (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/x64sc -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

_END
fi

cat >>manifest.15 <<_END
      <QPM:Launch name="vsid">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/vsid"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="x128">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/x128"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="x128 (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/x128 -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xvic">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xvic"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xvic (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xvic -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xpet">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xpet"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xpet (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xpet -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xplus4">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xplus4"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xplus4 (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xplus4 -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xcbm2">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xcbm2"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xcbm2 (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xcbm2 -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xcbm5x0">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xcbmx0"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

      <QPM:Launch name="xcbm5x0 (no sound)">
         <QPM:String name="Topic" value="Applications/Emulators"/>
         <QPM:String name="Command" value="/opt/bin/xcbm5x0 -sounddev dummy"/>
         <QPM:String name="Icon" value="/usr/share/icons/topics/chameleon.gif"/>
      </QPM:Launch>

   </RDF:Description>
</RDF:RDF>
_END

  cat >public/VICE/core-$VICEVERSION/MANIFEST manifest.01 manifest.02 manifest.03 \
      manifest.04 manifest.05 manifest.06 manifest.07 manifest.08 manifest.09 \
      manifest.10 manifest.11 manifest.12 manifest.14 manifest.15

  tar cf VICE-core.tar public
  gzip VICE-core.tar
  mv VICE-core.tar.gz VICE-$VICEVERSION-public.qpk

  ls -l -R VICE-$VICEVERSION-public.qpk >size.tmp
  manifest_size=`./getsize size.tmp` 
  rm -f -r size.tmp

  echo >manifest.04 "            <QPM:PackageSize>$manifest_size</QPM:PackageSize>"

  cat >VICE-$VICEVERSION-public.qpm manifest.01 manifest.02 manifest.03 manifest.04 \
      manifest.05 manifest.06 manifest.07 manifest.08 manifest.09 manifest.10 \
      manifest.11 manifest.12 manifest.14 manifest.15

  rm -f -r public

  tar cf VICE-x86.tar VICE-$VICEVERSION-public.qp? VICE-$VICEVERSION-public.repdata VICE-$VICEVERSION-x86-public.qp?
  gzip VICE-x86.tar
  mv VICE-x86.tar.gz VICE-$VICEVERSION-x86-public.qpr

  rm -f -r *.qpk *.qpm *.repdata manifest.* VICE-$VICEVERSION

  echo QNX 6 port binary package part generated as VICE-$VICEVERSION-x86-public.qpr
else
  echo QNX 6 port binary distribution directory generated as VICE-$VICEVERSION
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
