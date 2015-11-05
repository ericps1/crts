#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Python Txrx
# Generated: Thu Nov  5 11:43:54 2015
##################################################

from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser


class python_txrx(gr.top_block):

    def __init__(self, rx_server_address="192.168.1.56", tx_server_address="192.168.1.55"):
        gr.top_block.__init__(self, "Python Txrx")

        ##################################################
        # Parameters
        ##################################################
        self.rx_server_address = rx_server_address
        self.tx_server_address = tx_server_address

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 32000

        ##################################################
        # Blocks
        ##################################################
        self.blocks_tuntap_pdu_0 = blocks.tuntap_pdu("tunCRTS", 10000, True)
        self.blocks_socket_pdu_1 = blocks.socket_pdu("UDP_SERVER", tx_server_address, "52001", 10000, False)
        self.blocks_socket_pdu_0 = blocks.socket_pdu("UDP_CLIENT", rx_server_address, "52001", 10000, False)

        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_socket_pdu_0, 'pdus'), (self.blocks_tuntap_pdu_0, 'pdus'))    
        self.msg_connect((self.blocks_tuntap_pdu_0, 'pdus'), (self.blocks_socket_pdu_1, 'pdus'))    

    def get_rx_server_address(self):
        return self.rx_server_address

    def set_rx_server_address(self, rx_server_address):
        self.rx_server_address = rx_server_address

    def get_tx_server_address(self):
        return self.tx_server_address

    def set_tx_server_address(self, tx_server_address):
        self.tx_server_address = tx_server_address

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate


def argument_parser():
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    parser.add_option(
        "", "--rx-server-address", dest="rx_server_address", type="string", default="192.168.1.56",
        help="Set rx_server_address [default=%default]")
    parser.add_option(
        "", "--tx-server-address", dest="tx_server_address", type="string", default="192.168.1.55",
        help="Set tx_server_address [default=%default]")
    return parser


def main(top_block_cls=python_txrx, options=None):
    if options is None:
        options, _ = argument_parser().parse_args()

    tb = top_block_cls(rx_server_address=options.rx_server_address, tx_server_address=options.tx_server_address)
    tb.start()
    tb.wait()


if __name__ == '__main__':
    main()
