SRC_DIR=$HOME/bmc64-pi2

cd third_party/circle-stdlib

find . -name 'config.cache' -exec rm {} \;

cd libs/circle-newlib
patch -p1 < ../../../../circle_newlib_patch.diff

cd ../circle
patch -p1 < ../../../../circle_patch.diff

cd ../..
patch -p1 < ../../circle_stdlib_patch.diff

./configure -n --raspberrypi=2
make -j4

cd libs/circle/addon

cd fatfs
make clean
make
cd ..

cd linux
make clean
make
cd ..

cd vc4/vchiq
make clean
make 

cd ../../../../../../..
cd third_party/vice-3.2

cd src/resid

CXXFLAGS="-funsafe-math-optimizations -fno-exceptions -fno-rtti -nostdinc++ -mfloat-abi=hard -ffreestanding -nostdlib -march=armv7-a -marm -mfpu=neon-vfpv4 --specs=nosys.specs -O2 -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$ARM_HOME/lib/gcc/arm-none-eabi/7.3.1/include -I$ARM_HOME/lib/gcc/arm-none-eabi/7.3.1/include-fixed" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" ./configure --host=arm-none-eabi

cd ../..

CIRCLE_HOME="$SRC_DIR/third_party/circle-stdlib" ARM_HOME="$HOME/gcc-arm-none-eabi-7-2018-q2-update" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-O2 -mfloat-abi=hard -ffreestanding -march=armv7-a -marm -mfpu=neon-vfpv4 -fno-exceptions -fno-rtti -nostdinc++ --specs=nosys.specs" CFLAGS="-O2 -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$ARM_HOME/lib/gcc/arm-none-eabi/7.3.1/include -I$ARM_HOME/lib/gcc/arm-none-eabi/7.3.1/include-fixed -I$CIRCLE_HOME/libs/circle/addon/fatfs -fno-exceptions --specs=nosys.specs -mfloat-abi=hard -ffreestanding -nostdlib -march=armv7-a -marm -mfpu=neon-vfpv4 -nostdinc" ./configure --host=arm-none-eabi --disable-textfield --disable-fullscreen --disable-vte --disable-nls --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-editline --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib

#make clean
#git status | grep deleted: | sed 's/deleted:/git checkout /' | sh

make x64
cd ../..

make clean
make
