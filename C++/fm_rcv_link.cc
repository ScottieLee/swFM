//Scott Lee, 2013

#include "fm_rcv_link.h"
#include <iostream>
#include <vector>
#include <gr_io_signature.h>
#include <assert.h>
#include <math.h>

fm_rcv_link_sptr make_fm_rcv_link_sptr(int hardware_decim, int rx_gain, double rx_freq, usrp_subdev_spec& spec)//FIXME ARGUMENTS!
{
    return gnuradio::get_initial_sptr(new fm_rcv_link(hardware_decim, rx_gain, rx_freq, spec));//FIXME ARGUMENTS!
}

const long   fm_rcv_link::adc_rate          = 64000000;
const int    fm_rcv_link::lpf_decim         = 20;
const float  fm_rcv_link::max_dev           = 75000;
const float  fm_rcv_link::audio_sample_rate = 96000;
const double PI                             = 3.1415927;
fm_rcv_link::fm_rcv_link(int hardware_decim,int rx_gain, double rx_freq, usrp_subdev_spec spec):
             gr_hier_block2("fm_rcv_link",
                            gr_make_io_signature(0,0,0),
                            gr_make_io_signature(0,0,0)
                           ),
             d_hardware_decim(hardware_decim),
             d_rx_gain(rx_gain),
             d_rf_freq(rx_freq),
             d_spec(spec)
             
{
    // reassign d_usrp_sink
    d_usrp_source_sptr = usrp_make_source_c();
    d_usrp_source_sptr -> set_decim_rate (d_hardware_decim);
    std::cout << "Hardware Decimation    = " << d_hardware_decim << std::endl;
    d_sub_device = d_usrp_source_sptr -> selected_subdev (d_spec);
    d_usrp_source_sptr -> set_mux (d_usrp_source_sptr -> determine_rx_mux_value (d_spec));
    d_sub_device -> set_gain(d_rx_gain);
    usrp_tune_result tune_result;
    bool ok = d_usrp_source_sptr -> tune (0, d_sub_device, d_rf_freq, &tune_result);
    std::cout << "Tune: Target  frequecy = " << d_rf_freq << std::endl;
    std::cout << "      Actural frequecy = " << (tune_result.baseband_freq + tune_result.dxc_freq) << std::endl;
    std::cout << "Rx Gain                = " << d_rx_gain << " dB" << std::endl;
    printf      ("Subdevice name is %s\n", d_sub_device->name().c_str());
    printf      ("Subdevice freq range: (%g, %g)\n", d_sub_device->freq_min(), d_sub_device->freq_max());
    // reassign d_channel_filter
   
    d_channel_filter = gr_make_fir_filter_ccf ( lpf_decim, gr_firdes::low_pass(
                                                                    1,
                                                                    adc_rate/d_hardware_decim,
                                                                    100000,
                                                                    10000,
                                                                    gr_firdes::WIN_BLACKMAN ,
                                                                    6.76
                                                                     )
                                            );
                                              
    // reassign d_fm_demod
    float fm_demod_gain = adc_rate/d_hardware_decim/lpf_decim/(2 * PI * max_dev);
    std::cout << "Final Audio Sample Rate = " << audio_sample_rate << " Hz" << std::endl;
    d_fm_demod = gr_make_quadrature_demod_cf(fm_demod_gain);
    float fractional_bw = 0.4;
    
    // rational resampler
    unsigned int interp = static_cast<int>(audio_sample_rate/1000);
    unsigned int decim  = static_cast<int>(adc_rate/d_hardware_decim/lpf_decim/1000);

    float beta = 5.0;
    float trans_width = 0.5 - fractional_bw;
    float mid_transition_band = 0.5 - trans_width/2;
    unsigned int d = gcd(interp, decim);

    interp = static_cast<int>(interp / d);
    decim  = static_cast<int>(decim  / d);
    std::vector<float> taps = gr_firdes::low_pass( interp, 1, mid_transition_band/interp, trans_width/interp, gr_firdes::WIN_KAISER, beta);
    std::cout << "Rational Resampler : Interp = " << interp << "\t" << "Decim = " << decim << std::endl;
    d_resampler = gr_make_rational_resampler_base_fff(interp, decim, taps);

    // deamphasizer
    // Implementation of fm_emph.py in C++, notice time constant in china is 50 us
    double tau  = 0.00005;  // 50 us
    double w_p  = 1/tau;
    double w_pp = tan (w_p / (audio_sample_rate * 2));
    double a1   = (w_pp - 1) / (w_pp + 1);
    double b0   = w_pp / (w_pp + 1);
    double b1   = b0;
    std::vector<double> btaps;
    btaps.push_back(b0);
    btaps.push_back(b1);
    std::vector<double> ataps;
    ataps.push_back(1);
    ataps.push_back(a1);
    d_deamphasizer = gr_make_iir_filter_ffd(btaps, ataps);


    // reassign d_audio_sink
    d_audio_sink = audio_alsa_make_sink( static_cast<int> (audio_sample_rate));


    // connect'em up

//    connect(self(), 0, d_usrp_source_sptr, 0);
    connect(d_usrp_source_sptr, 0, d_channel_filter, 0);
    connect(d_channel_filter, 0, d_fm_demod, 0);
    connect(d_fm_demod, 0 , d_resampler, 0);
    connect(d_resampler, 0 , d_deamphasizer, 0);
    connect(d_deamphasizer, 0, d_audio_sink, 0);
    //connect(d_audio_sink, 0, self(), 0);



}

void fm_rcv_link::assign_rational_resampler( gr_rational_resampler_base_fff_sptr resampler_sptr,
                                             unsigned int interp, 
                                             unsigned int decim, 
                                             float fractional_bw)
{

}

bool fm_rcv_link::set_rcv_link_freq(double target_freq)
{
    usrp_tune_result tune_result;
    if (d_usrp_source_sptr -> tune (0, d_sub_device, target_freq, &tune_result)){
        std::cout << "\nFrequency has been tuned to " << target_freq << std::endl;
        return 1;
    }
    else
        return 0;
}
bool fm_rcv_link::set_rcv_link_gain(int gain)
{
    if (d_sub_device -> set_gain(gain)){
        std::cout << "\nRx gain now is " << gain << " dB" << std::endl;
        return 1;
    }
    else
        return 0;
}
