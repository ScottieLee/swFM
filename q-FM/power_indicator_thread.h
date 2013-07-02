//Scott Lee, 2013
#ifndef POWER_INDICATOR_THREAD_H
#define POWER_INDICATOR_THREAD_H

#include <QThread>
#include "fm_top_block.h"
#include <QProgressBar>


//NOTICE: thread should not control UI, let the mainwindow control it , thread just emit signals to the mainwindow.
//        In addition, using thread the
class power_indicator_thread : public QThread
{
    Q_OBJECT

private:
    int d_pause;
    fm_top_block_sptr _fm_top_block_we_get;
    void run();
public:
    power_indicator_thread();
    void set_fm_top_block(fm_top_block_sptr* fm_top_block ) { _fm_top_block_we_get = *fm_top_block;}
    void pause() {d_pause = 1;}
signals:
    void power_value_changed(int value);

};

#endif // POWER_INDICATOR_THREAD_H
