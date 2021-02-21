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
make xpet
cd ../..

cd third_party/plus4emu
make
cd ../..

MACHINES="C64:c64 C128:c128 VIC20:vic20 Plus4:plus4 Plus4Emu:plus4emu PET:pet"

for m in $MACHINES
do
   P1=`echo $m | sed 's/:.*$//'`
   P2=`echo $m | sed 's/^.*://'`
   make clean
   BOARD=$BOARD make -f Makefile-$P1
   cp $KERNEL ${KERNEL}.$P2
done
