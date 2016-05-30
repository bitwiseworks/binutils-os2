#!/bin/sh
## as the buildsystem realy sucks I tried to reconf every dir and it worked
## so please execute this script bevore you start building

olddir=`pwd`

# reconf root
echo "libtoolize root..."
libtoolize -fi
echo "autoreconf root..."
autoreconf -fi

dirs="bfd binutils gas intl ld libiberty opcodes"

for dir in $dirs; do
  cd $dir
  echo "autoreconf $dir..."
  autoreconf -fi
  cd ..
done

