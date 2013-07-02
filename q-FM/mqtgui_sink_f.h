// Modified by Scottlee 2013
/* -*- c++ -*- */
/*
 * Copyright 2008,2009 Free Software Foundation, Inc.
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

#ifndef INCLUDED_MQTGUI_SINK_F_H
#define INCLUDED_MQTGUI_SINK_F_H

#include <gr_block.h>
#include <gr_firdes.h>
#include <gri_fft.h>
#include <qapplication.h>
#include "mqtgui.h"
#include <Python.h>
#include "mSpectrumGUIClass.h"

class mqtgui_sink_f;
typedef boost::shared_ptr<mqtgui_sink_f> mqtgui_sink_f_sptr;

mqtgui_sink_f_sptr mqtgui_make_sink_f (int fftsize, int wintype,
				     double fc=0, double bw=1.0,
				     const std::string &name="Spectrum Display",
                                     bool plotfreq=true, bool plottime=true,
				     QWidget *parent=NULL);

class mqtgui_sink_f : public gr_block
{
private:
  friend mqtgui_sink_f_sptr mqtgui_make_sink_f (int fftsize, int wintype,
                                              double fc, double bw,         //fc is 0, bw is the [-f,f] we can see.
					      const std::string &name,
                                              bool plotfreq, bool plottime,
					      QWidget *parent);
  mqtgui_sink_f (int fftsize, int wintype,
		double fc, double bw,
		const std::string &name,
                bool plotfreq, bool plottime,
		QWidget *parent);

  void initialize();

  int d_fftsize;
  gr_firdes::win_type d_wintype;
  std::vector<float> d_window;
  double d_center_freq;
  double d_bandwidth;
  std::string d_name;
  
  pthread_mutex_t d_pmutex;

  bool d_shift;
  gri_fft_complex *d_fft;
  std::complex<float> *d_fftdata;

  int d_index;
  float *d_residbuf;

  bool d_plotfreq, d_plottime;

  QWidget *d_parent;
  SpectrumGUIClass *d_main_gui; 

  void windowreset();
  void buildwindow();
  void fftresize();
  void fft(const float *data_in, int size, gr_complex *data_out);
  
public:
  ~mqtgui_sink_f();
  void exec_();
  void lock();
  void unlock();
  QWidget*  qwidget();
  PyObject* pyqwidget();

  void set_frequency_range(const double centerfreq,
			   const double bandwidth);

  void set_time_domain_axis(double min, double max);
  void set_frequency_axis(double min, double max);

  QApplication *d_qApplication;
  qtgui_obj *d_object;

  int general_work (int noutput_items,
		    gr_vector_int &ninput_items,
		    gr_vector_const_void_star &input_items,
		    gr_vector_void_star &output_items);
};

#endif /* INCLUDED_MQTGUI_SINK_F_H */
