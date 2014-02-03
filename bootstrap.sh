#!/bin/bash
export BUILDDIR=`pwd`/build
mkdir -p ${BUILDDIR}
mkdir -p ${BUILDDIR}/include
cp -R -p include/x86/linux/* ${BUILDDIR}/include
GCCDIR=/usr/lib/gcc/`gcc -v 2>&1 | sed -r -n -e 's/Target: (.+$)/\1/p'`/`gcc -v 2>&1 | sed -r -n -e 's/gcc version (([0-9]+\.?)+).+/\1/p'`
ln -s ${GCCDIR} $BUILDDIR/gcc
LCCDIR=${BUILDDIR}/
CPPPATH=`whereis -b cpp | grep -o -E '/.+$'`
CRT1O=`whereis -b crt1.o | grep -o -E '/.+$'`
CRTIO=`whereis -b crti.o | grep -o -E '/.+$'`
CRTNO=`whereis -b crtn.o | grep -o -E '/.+$'`
LDLINUXSO=`whereis -b ld-linux.so.2 | grep -o -E '/.+$'`
make -e HOSTFILE=etc/linux.c CFLAGS="-DLCCDIR=\"\\\"${LCCDIR}\\\"\" -DCPPPATH=\"\\\"${CPPPATH}\\\"\" -DLD_LINUX_SO=\"\\\"${LDLINUXSO}\\\"\" -DCRT1_O=\"\\\"${CRT1O}\\\"\" -DCRTI_O=\"\\\"${CRTIO}\\\"\" -DCRTN_O=\"\\\"${CRTNO}\\\"\"" lcc
make TARGET=x86/linux test
make triple
echo "If there were no errors everything has been build. From now on rebuild by invoking 'make triple'"
