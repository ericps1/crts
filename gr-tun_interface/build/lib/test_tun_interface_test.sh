#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/users/ericps1/src/gnuradio/gr-tun_interface/lib
export PATH=/users/ericps1/src/gnuradio/gr-tun_interface/build/lib:$PATH
export LD_LIBRARY_PATH=/users/ericps1/src/gnuradio/gr-tun_interface/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PYTHONPATH
test-tun_interface 
