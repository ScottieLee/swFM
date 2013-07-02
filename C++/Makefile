all: fm_rcv
CXX = g++
CXXFLAGS = -g -O2 -Wall -Woverloaded-virtual
GNURADIO_CORE_CFLAGS = -pthread -I/usr/local/include/gnuradio -I/usr/local/include  
GNURADIO_CORE_CPPFLAGS =  -I/usr/local/include/gnuradio -I/usr/local/include
GNURADIO_CORE_INCLUDEDIR = /usr/local/include/gnuradio
GNURADIO_CORE_LIBS = -L/usr/local/lib -lboost_thread-mt -lboost_date_time-mt -lgnuradio-core -lgruel -lfftw3f -lgsl -lgslcblas -lm -lgnuradio-audio-alsa -lgnuradio-usrp
LIBS =  -L/usr/local/lib -lboost_thread-mt -lboost_date_time-mt -lgnuradio-core -lgruel -lfftw3f -lgsl -lgslcblas -lm  
LIBUSRP = -lusrp
INCLUDE_CURRENT_DIR = -I.
OPTIONS = ${GNURADIO_CORE_CFLAGS} ${GNURADIO_CORE_CPPFLAGS} ${GNURADIO_CORE_LIBS} ${LIBS} ${LIBUSRP} ${CXXFLAGS} ${INCLUDE_CURRENT_DIR}



fm_rcv: fm_rcv main.cc fm_rcv_link.cc fm_top_block.cc
	${CXX} ${OPTIONS} main.cc fm_rcv_link.cc fm_top_block.cc
