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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "tun_sink_block_impl.h"

int tun_alloc(char *dev, int flags);
int cwrite(int fd, char *buf, int n){

    int nwrite;

    if((nwrite=write(fd, buf, n)) < 0){
        printf("Error writing data");
        exit(1);
    }
    return nwrite;
}
namespace gr {
  namespace tun_blocks {

    tun_sink_block::sptr
    tun_sink_block::make(std::string interface_name, std::string interface_address)
    {
      return gnuradio::get_initial_sptr
        (new tun_sink_block_impl(interface_name, interface_address));
    }

    /*
     * The private constructor
     */
    tun_sink_block_impl::tun_sink_block_impl(std::string interface_name, std::string interface_address)
      : gr::sync_block("tun_sink_block",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(0, 0, 0))
    {
        strcpy(tun_name, interface_name.c_str());
        strcpy(ip, interface_address.c_str());
        memset(buffer, 0, sizeof(buffer));
        packet_len = 0;

        tunfd = tun_alloc(tun_name, IFF_TUN);

        char command[100];
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
    tun_sink_block_impl::~tun_sink_block_impl()
    {
        char command[100];
        int sys_return;
        sprintf(command, "route del -net 10.0.0.0 netmask 255.255.255.0 dev %s", tun_name);
        sys_return = system(command);
        sprintf(command, "ip link set dev %s down", tun_name);
        sys_return = system(command);

    }

    int
    tun_sink_block_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        char *in = (char *) input_items[0];

        int result = write(tunfd, in, noutput_items);
        printf("wrote %d bytes to interface\n", result);
        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace tun_blocks */
} /* namespace gr */

