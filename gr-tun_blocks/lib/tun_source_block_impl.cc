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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "tun_source_block_impl.h"

/**************************************************************************
 * tun_alloc: allocates or reconnects to a tun/tap device. The caller     *
 *            must reserve enough space in *dev.                          *
 **************************************************************************/
int tun_alloc(char *dev, int flags) {

    struct ifreq ifr;
    int fd, err;
    char clonedev[] = "/dev/net/tun";

    if( (fd = open(clonedev , O_RDWR)) < 0 ) {
        perror("Opening /dev/net/tun");
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags;

    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        return err;
    }

    strcpy(dev, ifr.ifr_name);

    return fd;
}

/**************************************************************************
 * cread: read routine that checks for errors and exits if an error is    *
 *        returned.                                                       *
 **************************************************************************/
fd_set tx_set;
int cread(int fd, char *buf, int n)
{
    FD_ZERO(&tx_set);

    FD_SET(fd, &tx_set);
    struct timeval timeout = {1000,0};
    int retval = select(fd + 1, &tx_set, NULL, NULL, &timeout);
    if(retval < 0)
    {
        perror("select()");
        exit(1);
    }
    

    uint16_t nread = 0;
    if(FD_ISSET(fd, &tx_set))
    {
        if((nread = read(fd, buf, n)) < 0)
        {
            perror("read()");
            exit(1);
        }
    }
    return nread;
}


namespace gr {
  namespace tun_blocks {

    tun_source_block::sptr
    tun_source_block::make(std::string interface_name, std::string interface_address)
    {
      return gnuradio::get_initial_sptr
        (new tun_source_block_impl(interface_name, interface_address));
    }

    /*
     * The private constructor
     */
    tun_source_block_impl::tun_source_block_impl(std::string interface_name, std::string interface_address)
	    : gr::sync_block("tun_source_block",
			    gr::io_signature::make(0, 0, 0),
			    gr::io_signature::make(1, 1, 1032*sizeof(char)))
	  {
		  strcpy(tun_name, interface_name.c_str());
		  strcpy(ip, interface_address.c_str());
		  memset(buffer, 0, sizeof(buffer));
		  idx = 0;
		  packet_len = 0;
		  buffer_empty = 1;

		  tunfd = tun_alloc(tun_name, IFF_TUN);
		  //int flags = fcntl(tunfd, F_GETFL, 0);
          //printf("calling fcntl\n");
		  //fcntl(tunfd, F_SETFL, flags | O_NONBLOCK);

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
    tun_source_block_impl::~tun_source_block_impl()
    {
	    char command[100];
	    int sys_return;
	    sprintf(command, "route del -net 10.0.0.0 netmask 255.255.255.0 dev %s", tun_name);
	    sys_return = system(command);
	    sprintf(command, "ip link set dev %s down", tun_name);
	    sys_return = system(command);

    }

    int
    tun_source_block_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
	    char *out = (char *) output_items[0];

	    // Do <+signal processing+>
	   /* if(buffer_empty){
		    packet_len = cread(tunfd, &buffer[sizeof(int)], 10000);
            printf("packet_len: %d\n", packet_len);
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
        */
        packet_len = cread(tunfd, buffer, 10000);
        printf("packet_len: %d\n", packet_len);

        if(packet_len > 0)
        {
            memcpy(out, buffer, packet_len); 
            return packet_len;
        }
        else
            return 0;
    }

  } /* namespace tun_blocks */
} /* namespace gr */

