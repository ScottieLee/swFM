//Scott Lee, 2013

#ifndef FM_TOP_BLOCK_H
#define FM_TOP_BLOCK_H

#include <gr_top_block.h>
#include "fm_rcv_link.h"

class fm_top_block;
typedef boost::shared_ptr<fm_top_block> fm_top_block_sptr;// define a sptr type

fm_top_block_sptr make_fm_top_block_sptr(); // declare a function;
fm_top_block_sptr make_fm_top_block_sptr(int hardware_decim, int rx_gain, double rx_freq, usrp_subdev_spec& spec); // override function;

class fm_top_block: public gr_top_block {

private:
    fm_top_block();  // private default constructor
    fm_top_block(int hardware_decim, int rx_gain, double rx_freq, usrp_subdev_spec& spec); // private constructor
    friend fm_top_block_sptr make_fm_top_block_sptr(); 
    friend fm_top_block_sptr make_fm_top_block_sptr(int hardware_decim, int rx_gain, double rx_freq, usrp_subdev_spec& spec); 
    fm_rcv_link_sptr         d_fm_rcv_link_block;
public:
    bool set_freq_now (const double freq);
    bool set_gain_now (const int    gain);
};
#endif
