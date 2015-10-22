/* -*- c++ -*- */
/* 
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "debug_block_impl.h"
#include <stdio.h>
namespace gr {
  namespace tun_blocks {

    debug_block::sptr
    debug_block::make()
    {
      return gnuradio::get_initial_sptr
        (new debug_block_impl());
    }

    /*
     * The private constructor
     */
    debug_block_impl::debug_block_impl()
      : gr::block("debug_block",
              gr::io_signature::make(1, 1, 1032*sizeof(char)),
              gr::io_signature::make(1, 1, 1032*sizeof(char)))
    {}

    /*
     * Our virtual destructor.
     */
    debug_block_impl::~debug_block_impl()
    {
    }

    void
    debug_block_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
         ninput_items_required[0] = noutput_items; 
    }

    int
    debug_block_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const char *in = (const char *) input_items[0];
        char *out = (char *) output_items[0];

        printf("received %d items\n", ninput_items[0]);
        printf("noutput_items: %d\n\n", noutput_items);
        // Do <+signal processing+>
        // Tell runtime system how many input items we consumed on
        // each input stream.
        for(int i = 0; i < ninput_items[0]; i++)
            out[i] = in[i];
        consume_each (ninput_items[0]);

        // Tell runtime system how many output items we produced.
        return ninput_items[0];
    }

  } /* namespace tun_blocks */
} /* namespace gr */

