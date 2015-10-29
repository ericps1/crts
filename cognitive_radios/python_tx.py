#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Python Tx
# Generated: Thu Oct 29 12:51:22 2015
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
        self.blocks_tuntap_pdu_0 = blocks.tuntap_pdu("tun0", 10000)
        self.blocks_socket_pdu_0 = blocks.socket_pdu("TCP_SERVER", "192.168.1.55", "52001", 10000, False)

        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_tuntap_pdu_0, 'pdus'), (self.blocks_socket_pdu_0, 'pdus'))    


    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate

if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    (options, args) = parser.parse_args()
    tb = python_tx()
    tb.start()
    tb.wait()
