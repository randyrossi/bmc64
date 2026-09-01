#!/bin/bash

SRC_DIR=$(cd "$(dirname "$0")" && pwd)

if ! source "$SRC_DIR/get_gnu_toolchain.sh"
then
   echo "Arm GNU Toolchain setup failed." >&2
   exit 1
fi

BOARD=""
IO_STATS=0

for arg in "$@"
do
case "$arg" in
       pi0|pi2|pi3|pi4)
              BOARD="$arg"
              ;;
       --io-stats)
              IO_STATS=1
              ;;
       *)
              echo "Need arg [pi0|pi2|pi3|pi4] [--io-stats]"
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
KERNEL=kernel7l.img
else
echo "Need arg [pi0|pi2|pi3|pi4] [--io-stats]"
exit
fi

# Opt-in storage I/O instrumentation. Must match how make_all.sh was invoked -
# the diskio stats patch is applied there, and the common + kernel builds below
# need -DBMC64_IO_STATS so io_stats.o provides the hooks that patch calls.
if [ "$IO_STATS" = "1" ]
then
       export BMC64_IO_STATS=1
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
make x128
make xvic
make xplus4
make xpet || true
cd ../..

cd third_party/plus4emu
make
cd ../..

MACHINES="C64:c64 C128:c128 VIC20:vic20 Plus4:plus4 Plus4Emu:plus4emu PET:pet"

for m in $MACHINES
do
   P1=`echo $m | sed 's/:.*$//'`
   P2=`echo $m | sed 's/^.*://'`
   echo "Building $P1 -> ${KERNEL}.$P2"
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

   if ! cp "$KERNEL" "${KERNEL}.$P2"
   then
      echo "Copy failed for ${KERNEL}.$P2"
      exit 1
   fi
done
