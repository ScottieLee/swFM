//Scott Lee, 2013

#ifndef FM_RCV_LINK_H
#define FM_RCV_LINK_H

#include <gr_hier_block2.h>
#include <usrp_source_c.h>
#include <gr_fir_filter_ccf.h>
#include <gr_quadrature_demod_cf.h>
#include <audio_alsa_sink.h>
#include <gr_firdes.h>
#include <gr_rational_resampler_base_fff.h>
#include <gr_iir_filter_ffd.h>
#include <gr_multiply_const_ff.h>
#include "mqtgui_sink_f.h"
#include "pi_power_indicator.h"

class fm_rcv_link;
typedef boost::shared_ptr<fm_rcv_link> fm_rcv_link_sptr;// define a sptr type
static usrp_subdev_spec static_spec(0,0); //CHANGE SIDE A OR B
fm_rcv_link_sptr make_fm_rcv_link_sptr( int hardware_decim = 16,
                                        int rx_gain = 30, 
                                        double rx_freq = 106800000,
                                        usrp_subdev_spec& spec = static_spec,  // sideA:[0,0] ; sideB:[1,0]
                                        int volume_int = 20
                                       ); // declare a function;

fm_rcv_link_sptr make_fm_rcv_link_sptr( QWidget* who,
                                        int volume_int = 20,
                                        int hardware_decim = 16,
                                        int rx_gain = 30,
                                        double rx_freq = 106800000,
                                        usrp_subdev_spec& spec = static_spec  // sideA:[0,0] ; sideB:[1,0]
                                       ); // declare a function;


class fm_rcv_link: public gr_hier_block2 {
private:
    fm_rcv_link(int hardware_decim,int rx_gain, double rx_freq, usrp_subdev_spec spec,int volume_int, QWidget* who =NULL);  // private constructor FIXME ADD ARGUMENTS!

    friend fm_rcv_link_sptr make_fm_rcv_link_sptr(int hardware_decim, int rx_gain, double rx_freq, usrp_subdev_spec& spec, int volume_int); //FIXME ADD ARGUMENTS!
    friend fm_rcv_link_sptr make_fm_rcv_link_sptr(QWidget* who, int volume_int, int hardware_decim, int rx_gain, double rx_freq, usrp_subdev_spec& spec); //FIXME ADD ARGUMENTS!

    // blocks
    usrp_source_c_sptr                         d_usrp_source_sptr;
    gr_fir_filter_ccf_sptr                     d_channel_filter;
    gr_quadrature_demod_cf_sptr                d_fm_demod;
    gr_rational_resampler_base_fff_sptr        d_resampler;
    gr_iir_filter_ffd_sptr                     d_deamphasizer;
    audio_alsa_sink_sptr                       d_audio_sink;
    mqtgui_sink_f_sptr                         d_qtsink;
    gr_multiply_const_ff_sptr                  d_volume_controller;
    // variables
    int                           d_hardware_decim;
    static const long             adc_rate;
    static const int              lpf_decim;
    static const float            max_dev;
    static const float            audio_sample_rate;
    int                           d_rx_gain;
    double                        d_rf_freq;
    db_base_sptr                  d_sub_device;
    usrp_subdev_spec              d_spec;
    int                           d_volume_int; // from 0 to 99

    void assign_rational_resampler( gr_rational_resampler_base_fff_sptr resampler_sptr, 
                                    unsigned int interp,
                                    unsigned int decim, 
                                    float fractional_bw = 0.4);

    inline unsigned int  gcd(unsigned int a, unsigned int b);

public:
    pi_power_indicator_sptr d_power_indicator; //make power indicator public

    // accessors
    bool set_rcv_link_freq (const double target_freq);
    bool set_rcv_link_gain (const int    target_gain);
    void set_volume_int    (const int    volume_int)
    {
        d_volume_int  = volume_int;
        d_volume_controller -> set_k(static_cast<float>(d_volume_int)/30);
    }

};

inline unsigned int fm_rcv_link::gcd( unsigned int a,
                                      unsigned int b)
{
    while( 1 )
    {
        a = a % b;
		if( a == 0 )
			return b;
		b = b % a;

        if( b == 0 )
			return a;
    }

}
#endif
