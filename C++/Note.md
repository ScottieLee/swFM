Scott Lee ,2013


#C++ Level Implementation of FM receiver using WBX daughterboard

##Issues:
    1. Make sure that audio.sink is implemented in C++ level, we'll use audio_alsa_sink 
         Done.
    2. Need to implement hier block wfm_rcv.py in C++ Level:Actually we can simply use quad demod as simplicity.
         Done.
    3. We use a dial tone demo to expand to our application 
         Done.
         First stable no-gui core code in master branch:  git commit 3666934c12819f235fbb893a7f9126c4718857b7
    4. Need to add deamphasizer to improve signal quality. Implement this in C++.
         New branch created, named as add_deamph
         Notice: A parameter named as Time Constant is 50us specificated in China.
         Done.
    5. While all the parameters are hard coded inside, make this right.
         Done. git commit fecce415f62718903c62dc1412c8ed42c082698a
    6. Need to add some accessors letting topblock get control of freq,gain,
         Patch for ALSA sink, SEE dir patch_for_alsa
         Add accessors, now it's able to change freq and gain dynamically through top block instance.
         Done. git commit b3099468bfc86ddc506f13b12e176b2e54ba575f
    
    7. NOW START BUILDING QTGUI for application
         New branch created, named as qt_gui.
    <----

##qt_gui Implementation:

    1. Things like these:
        A main window, with start and stop button to control flow graph
        Several setting area to type custom frequency, rx_gain
        how to set volum?  add a multiply const block!
        A slicer for volum
        A stuff for  fft of baseband display.
        A button for record sounds add file_sink block, not so easy

##FINDSOME ERRORS IN PREVIOUS CODE: (FIX001)
    1. type of rx_freq is double, wrong:there's some ints in the code

    2. When using qt, DO NOT LINK "libusrp"!!!!, this will cause
                                    Warning: Treating daughterboard with invalid EEPROM contents as if it were a "Basic Rx."
                                    Warningarning: This is almost certainly wrong...  Use appropriate burn-*-eeprom utility.:
       
       <<<THE REASON OF THIS ERROR>>>: we've got two libusrp.so. One in path /usr/lib/ is the old lib which doesn't support WBX.
                                                                 The other in path /usr/local/lib is the 3.3.0 lib which works normally.
         If we would like to use libusrp and WBX, we need to tell the exact lib path so that link knows which lib to link.


    3. There's some subdev_specs initialized using new ,which leads to memory leak.
    FIX001 DONE. git commit: commit 58ea42f475b6588b52d08f4a797f32f20d647580 at branch qt_qui


##Issues:
    1. How to add a wave strength indicator:Need to create a custom sink block, expose the float ample, which indicates the instant wave strength. Notice this ample should be connected before multiply_const.
    2. Add volume controller and silence radio button: Need to connect a multiply const button

    FIX002: Block connection is wrong, should be fm_demod -> deamphasizer -> resampler -> soundcard
                                                                  |
                                                               qt_sink
        Done. git commit: commit d134c69537a6b4f9d6969010b6b092b2eb95e0d8 This version also gets an FFT widget but has some issues.
    3. Need to make a custom fftwidget, create branch "qt_gui_fft"
    <-

Custom fftwidget Implemenetation:
    1. Don't use libgr-qtgui. Modify and compile the source code.
    2. How to merge the fft tab and the Radio Selection Tab? Or alternatively display them saperately?

##TODOLIST:
    1. Change Buttons style, background style
    2. Can we combine the play and pause button?
    3. Disable maximizing window
    4. Add radio station bookmarks, and enable switching radio station within the bookmarks
    5. Add sound record function
    6. Add a fine tuning function!

##FIX003 
fix FFT display error, The error is caused by the d_shift flag in qtgui_sink_f. Making it FALSE can fix this issue. In addition, the parameter of bw in qtgui_sink should be the very sample rate fed into the qtgui_sink. Done git branch qt_gui_fft commit 2c7ca83990d5f144367d574a368ebff634da9e64.  We can easily see the pilot tone at BaseFreq = 19KHz RFFreq = 99MHz!!

##FIX004
fix FFT canvas size problem, simply modify the maximum size of FrequencyPlotDisplayFrame in spectrum displayUI.  git commit 65f0f9049a7eb8bb50874261ddeb1f437a1c7421
***
Bug still exits: FFT windows sometimes won't display spectrum.
***

    commit  4c7d8ba8e812a99114162ffcf462afd7d3df9a29: add a sound power indicator.           
##There were some commit have undetached headers(no branch issue).
Create a new brach named "trunk" for future development, merge commit 4c7d8ba8e812a99114162ffcf462afd7d3df9a29 into this branch                                                                 

