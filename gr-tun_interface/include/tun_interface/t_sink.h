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


#ifndef INCLUDED_TUN_INTERFACE_T_SINK_H
#define INCLUDED_TUN_INTERFACE_T_SINK_H

#include <tun_interface/api.h>
#include <gnuradio/sync_block.h>
#include <string>

namespace gr {
  namespace tun_interface {

    /*!
     * \brief <+description of block+>
     * \ingroup tun_interface
     *
     */
    class TUN_INTERFACE_API t_sink : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<t_sink> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of tun_interface::t_sink.
       *
       * To avoid accidental use of raw pointers, tun_interface::t_sink's
       * constructor is in a private implementation
       * class. tun_interface::t_sink::make is the public interface for
       * creating new instances.
       */
      static sptr make(std::string interface_name, std::string ip_address);
    };

  } // namespace tun_interface
} // namespace gr

#endif /* INCLUDED_TUN_INTERFACE_T_SINK_H */

