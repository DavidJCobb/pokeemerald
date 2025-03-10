#!/bin/sh

# makefiles are aggressively terrible, and i can't figure out how to get 
# this project's makefile to build the plug-ins
#
# so we're just going to have all builds run through this script

# https://stackoverflow.com/a/65647357
is_installed() {
   dpkg --verify "$1" 2>/dev/null
}

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

if is_installed "lua5.4"; then
   : # no-op statement
else
   >&2 echo "The lua5.4 package must be installed in order to run post-build scripts. Please run the following command, which will likely prompt you for your password: "
   >&2 echo "   sudo apt-get install lua5.4"
   exit 1
fi

make modern -j$(nproc)
if [ $? -eq 0 ]; then
   echo "The build succeeded. Running post-build steps..."
   
   # Post-build script for savedata indexing.
   lua5.4 tools/lu-save-js-indexer/main.lua
   lua5.4 tools/lu-save-report-generator/main.lua
   
   echo "Post-build steps have run."
fi