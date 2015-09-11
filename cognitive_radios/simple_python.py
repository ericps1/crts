#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Simple Python
# Generated: Fri Sep 11 11:03:30 2015
##################################################

from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser

class simple_python(gr.top_block):

    def __init__(self, freq=440e6, mod_scheme="qpsk"):
        gr.top_block.__init__(self, "Simple Python")

        ##################################################
        # Parameters
        ##################################################
        self.freq = freq
        self.mod_scheme = mod_scheme

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 32000

        ##################################################
        # Blocks
        ##################################################


    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq

    def get_mod_scheme(self):
        return self.mod_scheme

    def set_mod_scheme(self, mod_scheme):
        self.mod_scheme = mod_scheme

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate

if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    parser.add_option("", "--freq", dest="freq", type="eng_float", default=eng_notation.num_to_str(440e6),
        help="Set freq [default=%default]")
    parser.add_option("", "--mod-scheme", dest="mod_scheme", type="string", default="qpsk",
        help="Set mod_scheme [default=%default]")
    (options, args) = parser.parse_args()
    tb = simple_python(freq=options.freq, mod_scheme=options.mod_scheme)
    tb.start()
    tb.wait()
    print 'freq: ', tb.freq
    print 'mod_scheme: ', tb.mod_scheme

