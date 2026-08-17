# Build Instructions

These instructions have been tested on Debian/Ubuntu/Mint distributions. If you are using a different platform or distribution, adjust accordingly. For example, changing *apt* to *yum* on CentOS.

----
## Prerequisites
1. If your system doesn't already have them, install all of the regular build tools:

        sudo apt-get install git build-essential automake autoconf libtool pkg-config autoconf-archive autotools-dev

2. Install the 65xx cross assembler:

        sudo apt-get install xa65

3. Install other prerequisites you may need:

        sudo apt-get install make cmake automake m4 bison byacc flex texinfo

----

## Downloading The Required Files

1. Clone the repo:

        cd /path/to/store/files/
        git clone https://github.com/randyrossi/bmc64.git --recursive


## Manual Installation of GNU Toolchain for Arm (Optional)

Installation and use of the GNU toolchain is automated by the build scripts. To optonally install it manually follow the steps below

1. Download and unpack the *GNU Embedded Toolchain for Arm*:

    * Visit [https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
    * Download the package for your system. For example, at the time of writing [arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz](https://gitlab.arm.com/api/v4/projects/tooling%2Fgnu-toolchains-for-arm/packages/generic/gnu-toolchain/15.2.rel1/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz) was available for Linux x86\_64.
    * Once downloaded, extract the file somewhere. Note down the path to the extracted directory. This will be used later.
    * In the extracted directory, find the /lib/gcc/arm-none-eabi/ directory and note down the version number. This will be used later.

2. Set the ARM_HOME environment variable to point to the extracted Arm toolchain directory:

        export ARM_HOME=/path/to/extracted/toolchain/directory

3. Set the PATH environment variable to point to the Arm toolchain's *bin* directory:

        export PATH=$PATH:$ARM_HOME/bin

4. Set the ARM_VERSION environment variable to the version number you noted previously:

        export ARM_VERSION=15.2.1

----
## Building

Below are the individual steps to clean and build useful when developing. For a one step build process see the `build_sdcard.sh` below.

1. If this is your first time building, run *clean_all.sh* from the bmc64 directory:

        cd /path/to/store/files/bmc64/
        ./clean_all.sh

2. Run *make_all.sh* from the bmc64 directory to build the third party libraries and kernel:

        cd /path/to/store/files/bmc64/
        ./make_all.sh [pi0|pi2|pi3]

        NOTE: In the above, set the required Pi version, such as ./make_all.sh pi3

3. Run *make_machines.sh* to build the kernel images for each machine (C64, C128, etc):

        cd /path/to/store/files/bmc64/
        ./make_machines.sh [pi0|pi2|pi3]

        NOTE: In the above, set the required Pi version, such as ./make_machines.sh pi3


----
## Output
Once the *make_all.sh* build is complete, you will have a kernel file for the Pi version that you built for:

* *Raspberry Pi Zero*: kernel.img
* *Raspberry Pi 2*: kernel7.img
* *Raspberry Pi 3*: kernel8-32.img

Once the *make_machines.sh* build is complete, you will have kernel files with a suffix (.c64, .c128, .vic20, etc) for the Pi version that you built for. For example, if pi3 was used you might get the following files:

* kernel8-32.img.c128
* kernel8-32.img.c64
* kernel8-32.img.pet
* kernel8-32.img.plus4
* kernel8-32.img.plus4emu
* kernel8-32.img.vic20


----
## More About The Build Files

* *clean\_all.sh*: Cleans everything
* *make\_all.sh [pi0|pi2|pi3]*: Builds third party libraries (Vice/Circle) and will also build a C64 image for the selected Pi model. This step will likely fail if you don't run *clean\_all.sh* first.
* *make\_machines.sh [pi0|pi2|pi3]*: Builds all machines for the selected Pi model. This step will likely fail if you don't run *make\_all.sh* first.


----
## Problems
1. When you run *make\_all.sh* you get an error such as '#error "<dirent.h> not supported"'

    Ensure you have all of the build tools installed from the prerequisites step.
    
    If it still does not work, try rebooting your machine and following the "Building" steps again.

2. When you run one of the build scripts (*clean\_all.sh*, *make\_all.sh* or *make\_machines.sh*), you get an error such as 'arm-eabi-g++: Command not found'

    Before you build, set the PREFIX environment variable so arm-none-eabi-g++ is used instead since the *GNU Embedded Toolchain for Arm* download seems to only have the commands with "none" in the filenames. For example: export PREFIX=arm-none-eabi-

3. I have a different build issue...

    * Ensure you have set all environment variables in the "Building" step. For example, if you forget to set the ARM\_HOME and ARM\_VERSION variables, *make\_all.sh* will default to "$HOME/gcc-arm-none-eabi-7-2018-q2-update" with a version of "7.3.1" which may or may not exist on your system.
    * Ensure you have all prerequisites installed.
    * Ensure you follow each step above.
    * Ensure you run the build scripts in the following order: *clean\_all.sh*, *make\_all.sh*, *make\_machines.sh*

----
## Building SDCard Layout

To build a full SDCard layout use the *build_sdcard.sh*. 

This will use all of the above scripts together to build a full SDCard layout in the 'build/sdcard' folder. 

If you pass no arguments it will build all three Pi versions pi0,pi2,pi3. Or you can pass one of the Pi versions to build a single version.

        cd /path/to/store/files/bmc64/
        ./build_sdcard.sh [pi0|pi2|pi3]

        NOTE: In the above, optinally set the required Pi version, such as ./build_sdcard.sh pi3 to build a single version

### Development options

If you need to build a kernel with additional logging and memory debugging you can build just a single machine.

To build and stage one emulator image, add `--machine` followed by `c64`, `c128`, `vic20`, `plus4`, `plus4emu`, or `pet`. The selected build preserves matching image and debug artifacts (`.elf`, `.lst`, and `.map`) with the machine suffix.

        ./build_sdcard.sh pi3 --machine vic20

For a KASAN diagnostic build, set `KASAN_ENABLED = 1` in Circle's `Config.mk` before running the script, or pass `--kasan`. The script preserves an enabled setting when it regenerates Circle's configuration.

----
## Resources

* [https://github.com/raspberrypi/tools](https://github.com/raspberrypi/tools)
* [https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)


----
## Dependencies

These are the git hashes for the repos under circle-stdlib this project is known to work with. If the patches don't apply successfully, it's possible those projects have moved forward causing incompatibilities. If that happens, reset these repos using "git reset HASH --hard" commands in the following directories:

* circle-stdlib: dda16112cdb5470240cd51fb33bf72b311634340
* libs/circle: fe24b6bebd1532f2a0ee981af12eaf50cc9e97fb
* libs/circle-newlib: c01f95bcb08278d9e00f9795c7641284d4f89931
* libs/mbedtls: d81c11b8ab61fd5b2da8133aa73c5fe33a0633eb
