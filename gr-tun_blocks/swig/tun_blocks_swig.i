/* -*- c++ -*- */

#define TUN_BLOCKS_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "tun_blocks_swig_doc.i"

%{
#include "tun_blocks/tun_source_block.h"
#include "tun_blocks/debug_block.h"
#include "tun_blocks/tun_sink_block.h"
%}


%include "tun_blocks/tun_source_block.h"
GR_SWIG_BLOCK_MAGIC2(tun_blocks, tun_source_block);

%include "tun_blocks/debug_block.h"
GR_SWIG_BLOCK_MAGIC2(tun_blocks, debug_block);
%include "tun_blocks/tun_sink_block.h"
GR_SWIG_BLOCK_MAGIC2(tun_blocks, tun_sink_block);
