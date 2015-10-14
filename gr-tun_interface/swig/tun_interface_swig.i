/* -*- c++ -*- */

#define TUN_INTERFACE_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "tun_interface_swig_doc.i"

%{
#include "tun_interface/t_source.h"
#include "tun_interface/t_sink.h"
%}


%include "tun_interface/t_source.h"
GR_SWIG_BLOCK_MAGIC2(tun_interface, t_source);
%include "tun_interface/t_sink.h"
GR_SWIG_BLOCK_MAGIC2(tun_interface, t_sink);
