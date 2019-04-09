#!/bin/sh

# see if we are in the top of the tree
if [ ! -f configure.proto ]; then
  cd ../..
  if [ ! -f configure.proto ]; then
    echo "please run this script from the base of the VICE directory"
    echo "or from the appropriate build directory"
    exit 1
  fi
fi

curdir=`pwd`
./configure -v --prefix=/usr/local cross_compiling=yes ac_cv_c_bigendian=no resid_cv_builtin_expect=no
make CC="/bin/sh $curdir/src/arch/unix/qnx4/qnx4-cc.sh"
