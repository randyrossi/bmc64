#!/bin/bash

if [ "$1" = "pi3" ]
then
KERNEL=kernel8-32.img
elif [ "$1" = "pi0" ]
then
KERNEL=kernel.img
elif [ "$1" = "pi2" ]
then
KERNEL=kernel7.img
else
echo "Need arg [pi2|pi3|pi0]"
exit
fi

cd third_party/vice-3.3
make x64
make x128
make xvic
cd ../..

make clean
make -f Makefile-C64
cp $KERNEL ${KERNEL}.c64

make clean
make -f Makefile-C128
cp $KERNEL ${KERNEL}.c128

make clean
make -f Makefile-VIC20
cp $KERNEL ${KERNEL}.vic20
