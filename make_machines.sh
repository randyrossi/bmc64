#!/bin/bash
#
# make_machines.sh
#
# BMC64 - A bare metal Commodore emulator for Raspberry Pi
# Copyright (C) 2019 Randy Rossi <randy.rossi@gmail.com>
# Copyright (C) 2026 aminch <https://github.com/aminch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

SRC_DIR=$(cd "$(dirname "$0")" && pwd)

# The Pi 4 and 5 are built 64-bit (AArch64)
case " $* " in
       *" pi4 "*|*" pi5 "*) export BMC64_AARCH=64 ;;
esac

if ! source "$SRC_DIR/get_gnu_toolchain.sh"
then
   echo "Toolchain setup failed (Arm GNU Toolchain or Node.js)." >&2
   exit 1
fi

BOARD=""
IO_STATS=0
PERF_STATS=0

for arg in "$@"
do
case "$arg" in
       pi0|pi2|pi3|pi4|pi5)
              BOARD="$arg"
              ;;
       --perf-stats)
              PERF_STATS=1
              ;;
       --io-stats)
              IO_STATS=1
              ;;
       *)
              echo "Need arg [pi0|pi2|pi3|pi4|pi5] [--io-stats] [--perf-stats]"
              exit 1
              ;;
esac
done

if [ "$BOARD" = "pi3" ]
then
KERNEL=kernel8-32.img
elif [ "$BOARD" = "pi0" ]
then
KERNEL=kernel.img
elif [ "$BOARD" = "pi2" ]
then
KERNEL=kernel7.img
elif [ "$BOARD" = "pi4" ]
then
# Circle builds kernel8-rpi4.img; on the card it is kernel8.img, the
# firmware's default for a 64-bit Pi 4 (so switching back to the C64,
# which removes kernel=, still boots).
KERNEL=kernel8-rpi4.img
CARD_KERNEL=kernel8.img
elif [ "$BOARD" = "pi5" ]
then
KERNEL=kernel_2712.img
else
echo "Need arg [pi0|pi2|pi3|pi4|pi5] [--io-stats] [--perf-stats]"
exit
fi

# Opt-in storage I/O instrumentation. Must match how make_all.sh was invoked -
# the diskio stats patch is applied there, and the common + kernel builds below
# need -DBMC64_IO_STATS so io_stats.o provides the hooks that patch calls.
if [ "$IO_STATS" = "1" ]
then
       export BMC64_IO_STATS=1
fi

# Opt-in frame/audio performance instrumentation. Must match how make_all.sh
# was invoked: VICE was configured there with -DBMC64_PERF_STATS, and the
# common + kernel builds below need it so perf_stats.o provides the hooks.
if [ "$PERF_STATS" = "1" ]
then
       export BMC64_PERF_STATS=1
fi

cd third_party/common
make clean
BOARD=$BOARD make
if [ "$?" != "0" ]
then
	exit
fi
cd ../..

cd third_party/vice-3.3
make x64
if [ "$BOARD" = "pi4" ] || [ "$BOARD" = "pi5" ]
then
# The Pi 4 and 5 run the cycle exact C64 (Makefile-C64)
make x64sc
fi
make x128
make xvic
make xplus4
make xpet || true
cd ../..

cd third_party/plus4emu
make
cd ../..

CARD_KERNEL=${CARD_KERNEL:-$KERNEL}

MACHINES="C64:c64 C128:c128 VIC20:vic20 Plus4:plus4 Plus4Emu:plus4emu PET:pet"

for m in $MACHINES
do
   P1=`echo $m | sed 's/:.*$//'`
   P2=`echo $m | sed 's/^.*://'`
   echo "Building $P1 -> ${CARD_KERNEL}.$P2"
   if ! make clean
   then
      exit 1
   fi

   if ! BOARD=$BOARD make -f Makefile-$P1
   then
      echo "Build failed for $P1"
      exit 1
   fi

   if [ ! -f "$KERNEL" ]
   then
      echo "Expected output $KERNEL was not created for $P1"
      exit 1
   fi

   if ! cp "$KERNEL" "${CARD_KERNEL}.$P2"
   then
      echo "Copy failed for ${CARD_KERNEL}.$P2"
      exit 1
   fi
done
