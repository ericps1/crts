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


#ifndef INCLUDED_TUN_BLOCKS_TUN_SINK_BLOCK_H
#define INCLUDED_TUN_BLOCKS_TUN_SINK_BLOCK_H

#include <tun_blocks/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace tun_blocks {

    /*!
     * \brief <+description of block+>
     * \ingroup tun_blocks
     *
     */
    class TUN_BLOCKS_API tun_sink_block : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<tun_sink_block> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of tun_blocks::tun_sink_block.
       *
       * To avoid accidental use of raw pointers, tun_blocks::tun_sink_block's
       * constructor is in a private implementation
       * class. tun_blocks::tun_sink_block::make is the public interface for
       * creating new instances.
       */
      static sptr make(std::string interface_name="tun0", std::string interface_address="10.10.10.2");
    };

  } // namespace tun_blocks
} // namespace gr

#endif /* INCLUDED_TUN_BLOCKS_TUN_SINK_BLOCK_H */

