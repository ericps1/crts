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

#ifndef INCLUDED_TUN_INTERFACE_T_SINK_IMPL_H
#define INCLUDED_TUN_INTERFACE_T_SINK_IMPL_H

#include <tun_interface/t_sink.h>
#include <string>
#include <net/if.h>

namespace gr {
  namespace tun_interface {

    class t_sink_impl : public t_sink
    {
     private:
      // Nothing to declare in this block.

     public:
      t_sink_impl(std::string interface_name, std::string ip_address);
      ~t_sink_impl();
      char buffer[10000];
	  char tun_name[IFNAMSIZ];
	  char ip[16];
	  int tunfd;
	  int packet_len;
	  int idx;

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace tun_interface
} // namespace gr

#endif /* INCLUDED_TUN_INTERFACE_T_SINK_IMPL_H */

