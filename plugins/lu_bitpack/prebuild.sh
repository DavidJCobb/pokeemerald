#!/bin/bash
if [[ ! -f /usr/include/x86_64-linux-gnu/gmp.h ]]; then
   >&2 echo "The libgmp-dev package must be installed in order to build a GCC plug-in. Please run the following command, which will likely prompt you for your password: "
   >&2 echo "   sudo apt-get install libgmp3-dev"
   exit 1
fi
