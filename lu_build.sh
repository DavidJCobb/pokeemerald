#
# get number of usable threads
#
proc_count=`nproc`
#
# perform a build with that many threads
#
make -j$proc_count