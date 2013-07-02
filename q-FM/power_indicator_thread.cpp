//Scott Lee, 2013
#include "power_indicator_thread.h"
#include <iostream>
power_indicator_thread::power_indicator_thread() :
    d_pause(1)
{
}

void power_indicator_thread::run()
{
    d_pause = 0;

    while(1){
        if (_fm_top_block_we_get == 0)
            ;
        else{
            float power = _fm_top_block_we_get -> get_power();
            emit power_value_changed(ceil(power*150));
        }
        QThread::usleep(10000);
        if (d_pause)
            break;
    }
}
