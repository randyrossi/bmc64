#!/bin/bash

BOARD=$1

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
echo "Need arg [pi0|pi2|pi3|pi4]"
exit
fi

cd third_party/common
make
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
