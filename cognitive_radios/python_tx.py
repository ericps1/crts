#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Python Tx
# Generated: Mon Nov  2 12:00:32 2015
##################################################

from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser


class python_tx(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "Python Tx")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 32000

        ##################################################
        # Blocks
        ##################################################
        self.blocks_tuntap_pdu_0 = blocks.tuntap_pdu("tunCRTS", 10000, True)
        self.blocks_socket_pdu_0 = blocks.socket_pdu("TCP_SERVER", "192.168.1.55", "52001", 10000, False)
        self.blocks_message_debug_0 = blocks.message_debug()

        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_tuntap_pdu_0, 'pdus'), (self.blocks_message_debug_0, 'print_pdu'))    
        self.msg_connect((self.blocks_tuntap_pdu_0, 'pdus'), (self.blocks_socket_pdu_0, 'pdus'))    

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate


def main(top_block_cls=python_tx, options=None):

    tb = top_block_cls()
    tb.start()
    tb.wait()


if __name__ == '__main__':
    main()
