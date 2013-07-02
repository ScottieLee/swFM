//Scott Lee, 2013
/* -*- c++ -*- */
/*
 * Copyright 2006 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_PI_POWER_INDICATOR_H
#define INCLUDED_PI_POWER_INDICATOR_H
#include <stdlib.h> 
#include <gr_sync_block.h>
class pi_power_indicator;
typedef boost::shared_ptr<pi_power_indicator> pi_power_indicator_sptr;

pi_power_indicator_sptr pi_make_power_indicator ();

/*!
 * input

 */
class pi_power_indicator : public gr_sync_block
{
  friend pi_power_indicator_sptr pi_make_power_indicator ();
  pi_power_indicator ();
private:
  float d_power;
 public:
  float get_power(void) {return d_power;}
  int work (int noutput_items,
	    gr_vector_const_void_star &input_items,
	    gr_vector_void_star &output_items);
};

#endif
