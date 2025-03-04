#!/bin/sh

# makefiles are aggressively terrible, and i can't figure out how to get 
# this project's makefile to build the plug-ins
#
# so we're just going to have all builds run through this script

for dname in plugins/*; do
  if [ -f ${dname}/prebuild.sh ]; then
    bash ${dname}/prebuild.sh
  fi
  if [ -f ${dname}/Makefile ]; then
    make -C ${dname} all
  fi
done
if [[ ! -f plugins/lu_bitpack/lu_bitpack.so ]]; then
   >&2 echo "The plug-ins failed to build."
   exit 1
fi


make modern -j$(nproc)