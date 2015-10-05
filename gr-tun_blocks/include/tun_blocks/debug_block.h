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


#ifndef INCLUDED_TUN_BLOCKS_DEBUG_BLOCK_H
#define INCLUDED_TUN_BLOCKS_DEBUG_BLOCK_H

#include <tun_blocks/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace tun_blocks {

    /*!
     * \brief <+description of block+>
     * \ingroup tun_blocks
     *
     */
    class TUN_BLOCKS_API debug_block : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<debug_block> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of tun_blocks::debug_block.
       *
       * To avoid accidental use of raw pointers, tun_blocks::debug_block's
       * constructor is in a private implementation
       * class. tun_blocks::debug_block::make is the public interface for
       * creating new instances.
       */
      static sptr make();
    };

  } // namespace tun_blocks
} // namespace gr

#endif /* INCLUDED_TUN_BLOCKS_DEBUG_BLOCK_H */

