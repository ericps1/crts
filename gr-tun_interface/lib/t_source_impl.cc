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
#include <net/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "t_source_impl.h"
#include "TUN.hpp"

namespace gr {
  namespace tun_interface {

    t_source::sptr
    t_source::make(std::string interface_name, std::string ip_address)
    {
      return gnuradio::get_initial_sptr
        (new t_source_impl(interface_name, ip_address));
    }

    /*
     * The private constructor
     */
    t_source_impl::t_source_impl(std::string interface_name, std::string ip_address)
      : gr::sync_block("t_source",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(char)))
    {
	  strcpy(tun_name, interface_name.c_str());
	  strcpy(ip, ip_address.c_str());
	  memset(buffer, 0, sizeof(buffer));
      idx = 0;
	  packet_len = 0;
      buffer_empty = 1;

	  tunfd = tun_alloc(tun_name, IFF_TUN);
      int flags = fcntl(tunfd, F_GETFL, 0);
	  fcntl(tunfd, F_SETFL, flags | O_NONBLOCK);

	  char command[50];
	  int sys_return;
	  sprintf(command, "ip link set dev %s up", tun_name);
	  sys_return = system(command);
	  sprintf(command, "ip addr add %s/24 dev %s", ip, tun_name);
	  sys_return = system(command);
	  sprintf(command, "route add -net 10.0.0.0 netmask 255.255.255.0 dev %s", tun_name);
	  sys_return = system(command);
	}

    /*
     * Our virtual destructor.
     */
    t_source_impl::~t_source_impl()
    {
	  char command[50];
	  int sys_return;
	  sprintf(command, "route del -net 10.0.0.0 netmask 255.255.255.0 dev %s", tun_name);
	  sys_return = system(command);
	  sprintf(command, "ip link set dev %s down", tun_name);
	  sys_return = system(command);
    }

    int
    t_source_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        char *out = (char *) output_items[0];

        if(buffer_empty){
          packet_len = cread(tunfd, &buffer[sizeof(int)], 10000);
		  if(packet_len > 0){
		    buffer_empty = 0;
		    memcpy(buffer, &packet_len, sizeof(int));
			*out = buffer[0];
			noutput_items = 1;
			idx = 1;
		  }
		  else noutput_items = 0;
		}
		else{
          *out = buffer[idx];
		  noutput_items = 1;
		  idx++;
		  if(idx == packet_len)
		    buffer_empty = 1;
		}
		
		// Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace tun_interface */
} /* namespace gr */

