//Scott Lee, 2013

#include "fm_top_block.h"
#include <gr_io_signature.h>

fm_top_block_sptr make_fm_top_block_sptr()
{
    return gnuradio::get_initial_sptr(new fm_top_block());
}

fm_top_block_sptr make_fm_top_block_sptr(QWidget* who, int volume_int)
{
    return gnuradio::get_initial_sptr(new fm_top_block(who, volume_int));
}

fm_top_block_sptr make_fm_top_block_sptr(int hardware_decim, int rx_gain, int rx_freq, usrp_subdev_spec& spec)
{
    return gnuradio::get_initial_sptr(new fm_top_block(hardware_decim, rx_gain, rx_freq, spec));
}

fm_top_block_sptr make_fm_top_block_sptr(int hardware_decim, int rx_gain, int rx_freq, usrp_subdev_spec& spec,QWidget* who, int volume_int)
{
    return gnuradio::get_initial_sptr(new fm_top_block(hardware_decim, rx_gain, rx_freq, spec, who, volume_int));
}


fm_top_block::fm_top_block(int hardware_decim, int rx_gain, int rx_freq, usrp_subdev_spec& spec): gr_top_block("FM Receiver Core")
{

    d_fm_rcv_link_block = make_fm_rcv_link_sptr(hardware_decim, rx_gain, rx_freq, spec);

    this -> connect (d_fm_rcv_link_block);
}

fm_top_block::fm_top_block(int hardware_decim, int rx_gain, int rx_freq, usrp_subdev_spec& spec, QWidget* who, int volume_int): gr_top_block("FM Receiver Core")
{

    d_fm_rcv_link_block = make_fm_rcv_link_sptr(who,  volume_int, hardware_decim, rx_gain, rx_freq, spec);

    this -> connect (d_fm_rcv_link_block);
}

fm_top_block::fm_top_block(): gr_top_block("FM Receiver Core")
{

    d_fm_rcv_link_block = make_fm_rcv_link_sptr();

    this -> connect (d_fm_rcv_link_block);
}

fm_top_block::fm_top_block(QWidget* who, int volume_int): gr_top_block("FM Receiver Core")
{

    d_fm_rcv_link_block = make_fm_rcv_link_sptr(who,volume_int);

    this -> connect (d_fm_rcv_link_block);
}


bool fm_top_block::set_freq_now(const double freq)
{
    return d_fm_rcv_link_block->set_rcv_link_freq(freq);
}
bool fm_top_block::set_gain_now(const int    gain)
{
    return d_fm_rcv_link_block->set_rcv_link_gain(gain);
}

