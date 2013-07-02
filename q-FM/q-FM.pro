#-------------------------------------------------
#
# Project created by QtCreator 2013-03-12T20:06:42
#
#-------------------------------------------------

QT       += core gui

TARGET = q-FM
TEMPLATE = app

INCLUDEPATH += /usr/local/include/gnuradio \
               /usr/local/include . \
               /usr/include/qwt-qt4 /usr/include/qwtplot3d-qt4 /usr/include/python2.6 \
               ./fft_related

LIBS += -L/usr/local/lib -lboost_thread-mt \
        -lboost_date_time-mt -lgnuradio-core \
        -lgruel -lfftw3f -lgsl -lgslcblas -lm \
        -L/usr/local/lib -lboost_thread-mt \
        -lboost_date_time-mt -lgnuradio-core -lgruel \
        -lgnuradio-audio-alsa -lgnuradio-usrp \
        -lgnuradio-qtgui -lqwt-qt4 -lpython2.6
SOURCES += main.cpp\
        mainwindow.cpp \
    model.cpp \
    fm_top_block.cc \
    fm_rcv_link.cc \
    mqtgui_sink_f.cc \
    fft_related/mTimeDomainDisplayPlot.cc \
    fft_related/mspectrumUpdateEvents.cc \
    fft_related/mSpectrumGUIClass.cc \
    fft_related/mspectrumdisplayform.cc \
    fft_related/mFrequencyDisplayPlot.cc \
    pi_power_indicator.cc \
    power_indicator_thread.cpp

HEADERS  += \
    mainwindow.h \
    model.h \
    fm_top_block.h \
    fm_rcv_link.h \
    mqtgui_sink_f.h \
    fft_related/mTimeDomainDisplayPlot.h \
    fft_related/mspectrumUpdateEvents.h \
    fft_related/mSpectrumGUIClass.h \
    fft_related/mspectrumdisplayform.h \
    fft_related/mqtgui.h \
    fft_related/mFrequencyDisplayPlot.h \
    pi_power_indicator.h \
    power_indicator_thread.h

FORMS    += \
    mainwindow.ui \
    fft_related/mspectrumdisplayform.ui

RESOURCES += \
    icons/icons.qrc

QMAKE_CXXFLAGS += -g -O2 -Wall -Woverloaded-virtual -pthread

OTHER_FILES += \
    fft_related/SpectrumGUIClass.lo \
    fft_related/spectrumdisplayform.lo \
    fft_related/Makefile.am \
    fft_related/Makefile \
    fft_related/libgnuradio-qtgui.la
