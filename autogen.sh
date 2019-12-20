#!/bin/sh
## as the buildsystem realy sucks I tried to reconf every dir and it worked
## so please execute this script bevore you start building

olddir=`pwd`

dirs=". bfd binutils etc gas gprof intl ld libiberty opcodes libctf"

for dir in $dirs; do
  if [ "$dir" != "." ]; then
    cd $dir
  fi
  echo "executing autoreconf -fi in directory $dir "
  autoreconf -fi
  if [ "$dir" != "." ]; then
    cd ..
  fi
done

