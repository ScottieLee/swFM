#! /usr/bin/env python
#
# Copyright 2005,2006,2007,2009 Free Software Foundation, Inc.
# 
# This file is part of GNU Radio
# 
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 
from gnuradio import gr, audio, usrp, blks2
import math
class fm_rx_top_block (gr.top_block):
    def __init__(self):
        gr.top_block.__init__(self)
        #build graph now

        #usrp_source
        self.usrp = usrp.source_c()
        adc_rate  = self.usrp.adc_rate()  #64MHz
        hw_decim  = 16                    #so Sample rate into host is 4MHz
        self.usrp.set_decim_rate(hw_decim)
        self.subdev = usrp.selected_subdev(self.usrp, usrp.pick_rx_subdevice(self.usrp))
        self.usrp.set_mux(usrp.determine_rx_mux_value(self.usrp, usrp.pick_rx_subdevice(self.usrp)))
        print "Using RX d'board %s" % (self.subdev.side_and_name(),)
        self.subdev.set_gain(30)
        rf_freq = 106800000               # 106.8MHz
        self.set_freq(rf_freq)
        print "Freq: ", rf_freq
        self.subdev.select_rx_antenna("TX/RX")

        #low pass filter
        self.sample_rate = adc_rate / hw_decim
        self.lpf_decim = 20               # so after channel filter, the sample rate is 200KHz
        self.lp_filter = gr.fir_filter_ccf( self.lpf_decim, gr.firdes.low_pass ( 1, 
                                                                                self.sample_rate, 
                                                                                100e3, # cut off freq
                                                                                10e3,  # transition band
                                                                                gr.firdes.WIN_BLACKMAN, # Window function
                                                                                6.76              # not used
                                                                                 ))
        # WBFM receiver
        quad_rate = self.sample_rate  #input rate of demodulator
        max_dev = 75e3        #max deviation of FM Broadcast
        fm_demod_gain = quad_rate/(2 * math.pi * max_dev)
        self.fm_decoder = gr.quadrature_demod_cf (fm_demod_gain)


        # Rational Resampler
        self.audio_sample_rate  = 96000
        self.rational_resampler = blks2.rational_resampler_fff ( interpolation = int(self.audio_sample_rate/1000),
                                                                decimation    = int(self.sample_rate/self.lpf_decim/1000),
                                                                taps          = None,
                                                                fractional_bw = None,
                                                              )
        self.audio_sink = audio.sink (int(self.audio_sample_rate), "", True)

        #connections
        self.connect ( self.usrp, self.lp_filter, self.fm_decoder, self.rational_resampler, self.audio_sink )

    def set_freq(self, target_freq):
        """
        Set the center frequency we're interested in.

        @param target_freq: frequency in Hz
        @rypte: bool

        Tuning is a two step process.  First we ask the front-end to
        tune as close to the desired frequency as it can.  Then we use
        the result of that operation and our target_frequency to
        determine the value for the digital down converter.
        """
        r = self.usrp.tune(0, self.subdev, target_freq)
        
        if r:
            print "Baseband = ", r.baseband_freq
	    print "Dxc Freq = ", r.dxc_freq
	    print "Residual Freq = ", r.residual_freq
	    print "Inverted = ", r.inverted
            return True

        return False
    def pick_subdevice(self, u):
        """
        The user didn't specify a subdevice on the command line.
        If there's a daughterboard on A, select A.
        If there's a daughterboard on B, select B.
        Otherwise, select A.
        """
        if u.db(0, 0).dbid() >= 0:       # dbid is < 0 if there's no d'board or a problem
            return (0, 0)
        if u.db(1, 0).dbid() >= 0:
            return (1, 0)
        return (0, 0)
if __name__ == '__main__':
    tb = fm_rx_top_block()
    try:
        tb.run()
    except KeyboardInterrupt:
        pass
