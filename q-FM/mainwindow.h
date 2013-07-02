//Scott Lee, 2013

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "model.h"
#include "fm_top_block.h"
#include "power_indicator_thread.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    model* DataModel;                      // DataModel Ptr
    Ui::MainWindow *ui;                    // ui
    fm_top_block_sptr top_block_instance;  // Top Block Instance
    static const double MEGA;
    power_indicator_thread power_indicator;

public:
    bool boot_usrp();                      // when click boot usrp button
    bool play();
    bool stop();

private slots:
    void on_volumeButton_clicked();
    void on_VolumeSlider_valueChanged(int value);
    void on_actionPlay_triggered();
    void on_actionPause_triggered();
    void on_actionBoot_USRP_triggered();
    void on_FreqEdit_returnPressed();
    void on_QuitButton_clicked();
    void on_StopButton_clicked();
    void on_PlayButton_clicked();
    void on_StartButton_clicked();
    void on_change_power_indicator_value(int value);
};

#endif // MAINWINDOW_H

