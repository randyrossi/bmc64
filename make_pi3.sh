SRC_DIR=$HOME/bmc64-pi3

cd third_party/circle-stdlib

find . -name 'config.cache' -exec rm {} \;

cd libs/circle-newlib
patch -p1 < ../../../../circle_newlib_patch.diff

cd ../circle
patch -p1 < ../../../../circle_patch.diff

cd ../..
patch -p1 < ../../circle_stdlib_patch.diff

./configure -n --raspberrypi=3 --prefix=arm-linux-gnueabihf-
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

CIRCLE_HOME="$SRC_DIR/third_party/circle-stdlib" ARM_HOME="$HOME/rpi-tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf" CXXFLAGS="-funsafe-math-optimizations -fno-exceptions -fno-rtti -nostdinc++ -mfloat-abi=hard -ffreestanding -nostdlib -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -O2 -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$ARM_HOME/lib/gcc/arm-linux-gnueabihf/4.9.3/include -I$ARM_HOME/lib/gcc/arm-linux-gnueabihf/4.9.3/include-fixed" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" ./configure --host=arm-linux-gnueabihf

cd ../..

CIRCLE_HOME="$SRC_DIR/third_party/circle-stdlib" ARM_HOME="$HOME/rpi-tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf" LDFLAGS="-L$CIRCLE_HOME/install/arm-none-circle/lib" CXXFLAGS="-fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" CFLAGS="-O2 -I$CIRCLE_HOME/install/arm-none-circle/include/ -I$CIRCLE_HOME/libs/circle/addon/fatfs -I$ARM_HOME/lib/gcc/arm-linux-gnueabihf/4.9.3/include -I$ARM_HOME/lib/gcc/arm-linux-gnueabihf/4.9.3/include-fixed -fno-exceptions -march=armv8-a -mtune=cortex-a53 -marm -mfpu=neon-fp-armv8 -mfloat-abi=hard -ffreestanding -nostdlib" ./configure --host=arm-linux-gnueabihf --disable-textfield --disable-fullscreen --disable-vte --disable-nls --disable-realdevice --disable-ipv6 --disable-ssi2001 --disable-catweasel --disable-hardsid --disable-parsid --disable-portaudio --disable-ahi --disable-bundle --disable-editline --disable-lame --disable-rs232 --disable-midi --disable-hidmgr --disable-hidutils --without-oss --without-alsa --without-pulse --without-zlib

make x64
cd ../..

make clean
make
