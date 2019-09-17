#!/bin/bash

export VERSION=2.5

SRC_PI2=bmc64-pi2-v39
SRC_PI3=bmc64-pi3-v39

if [ "$1" = "c64" ]
then
export MACHINE_L=c64
export MACHINE_U=C64
elif [ "$1" = "c128" ]
then
export MACHINE_L=c128
export MACHINE_U=C128
elif [ "$1" = "vic20" ]
then
export MACHINE_L=vic20
export MACHINE_U=VIC20
elif [ "$1" = "plus4" ]
then
export MACHINE_L=plus4
export MACHINE_U=PLUS4
else
echo "Need machine argument"
exit
fi

rm -f new.drv
rm -rf stage_dir
mkdir -p stage_dir
dd if=/dev/zero of=new.drv count=300 bs=1M

sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk new.drv
  o # clear the in memory partition table
  n # new partition
  p # primary partition
  1 # partition number 1
    # default - start at beginning of disk 
    # default - end of disk
  t # new partition
  b # primary partition
  w # write the partition table
  q # and we're done
EOF

sudo losetup -o 1048576 -f new.drv
sudo mkfs.vfat /dev/loop0 
sudo mount -t vfat /dev/loop0 stage_dir
sudo cp -R common_release_files/* stage_dir/
sudo cp -R ${MACHINE_L}_release_files/* stage_dir/

sudo cp ../${SRC_PI2}/kernel7.img.${MACHINE_L} stage_dir/kernel7.img
sudo cp ../${SRC_PI3}/sdcard/config.txt stage_dir
sudo cp ../${SRC_PI3}/sdcard/cmdline.txt stage_dir
sudo cp ../${SRC_PI3}/sdcard/$MACHINE_L/* stage_dir/$MACHINE_U
sudo cp ../${SRC_PI3}/kernel8-32.img.${MACHINE_L} stage_dir/kernel8-32.img
sudo cp ../${SRC_PI3}/LICENSE stage_dir
sudo cp ../${SRC_PI3}/README.md stage_dir

cd stage_dir
zip -r ../bm${MACHINE_L}-${VERSION}.files.zip .
cd ..

sudo umount stage_dir
sudo losetup -D

sleep 2

rm -rf stage_dir

sleep 1

mv new.drv bm${MACHINE_L}-${VERSION}.img
zip bm${MACHINE_L}-${VERSION}.img.zip bm${MACHINE_L}-${VERSION}.img
rm bm${MACHINE_L}-${VERSION}.img

# Mount .img
# sudo mount -t vfat -o loop,ro,offset=1048576 bmc64-1.3.img /mnt/here
