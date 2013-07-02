#include "fm_top_block.h"
#include <iostream>
int main (int argc, char* argv[])
{
    fm_top_block_sptr top_block_instance = make_fm_top_block_sptr();

    top_block_instance -> start(); //notice IF WE use start is runs in a seperate thread, so need to sleep or do some loop to prevent progran exit(0). //IF WE use run, it will block in this line, so there's no bother.
    std::cout << " Flow Graph is running..." << std::endl;
    sleep(3);
    top_block_instance -> set_freq_now(99000000);
    sleep(10);
    top_block_instance -> set_freq_now(101400000);
    sleep(10);
    top_block_instance -> stop();
    top_block_instance -> wait();
    top_block_instance -> start(); //restart
    sleep(10);
    return 0;
}
