//Scott Lee, 2013
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QtGui/QMessageBox>
#include <QProcess>
const double MainWindow::MEGA = 1000000;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),DataModel(0),
    ui(new Ui::MainWindow),power_indicator()
{
    ui->setupUi(this);
    ui ->PlayButton -> setDisabled(true);// Disable  play button
    ui ->actionPlay -> setDisabled(true);// Disable  play button
    ui ->StopButton -> setDisabled(true);// Disable  stop button
    ui ->actionPause ->setDisabled(true); // Disable  stop button
    DataModel = new model;
    QObject::connect(&power_indicator,SIGNAL(power_value_changed(int)),this,SLOT(on_change_power_indicator_value(int)),Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete DataModel;
}

bool MainWindow::boot_usrp()
{

    //First create a model instance
    //Then get initial freq and gains from UI to DataModel
    DataModel -> set_model_freq(ui -> FreqEdit -> text().toDouble());
    DataModel -> set_model_gain(ui -> GainEdit -> text().toInt());
    DataModel -> set_model_volume (ui -> VolumeSlider -> value());
    //Then build top block but don't start it
    try{
//        top_block_instance = make_fm_top_block_sptr();//This will take about 2s when first boot.

        //we should pass a widget pointer down to the mqtgui_sink_f block to tell it who is the parent widget.
        top_block_instance = make_fm_top_block_sptr(ui ->CentralWidget, ui -> VolumeSlider -> value());

    }
    catch(std::exception const & ex){
        return 1;
    }

    power_indicator.set_fm_top_block(&top_block_instance);
    return 0;
 }

bool MainWindow::play()
{
    //When Play Button Clicked
    top_block_instance -> start();
    power_indicator.start();
    return 0;
}


void MainWindow::on_StartButton_clicked()
{
    ui ->actionBoot_USRP ->setDisabled(true);
    ui ->StartButton ->setDisabled(true);// Disable  boot button
    this ->setCursor(Qt::WaitCursor);
    // FIXME this is not proper way to handle boot issue
    if (boot_usrp()){
        QMessageBox::warning(this,
                             tr("Error"),
                             tr("Can't open USRP.Please check the power and connection."),
                             QMessageBox::Yes);
        qApp->quit();
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());   //Restart programme

    }
    // USRP boot OK
    this ->setCursor(Qt::ArrowCursor);
    ui ->PlayButton -> setEnabled(true);// Enable  play button
    ui ->actionPlay -> setEnabled(true);// Enable  play button

}


void MainWindow::on_PlayButton_clicked()
{
    if(play()){
        QMessageBox::warning(this,
                             tr("Error"),
                             tr("Can't start Flow Graph!"),
                             QMessageBox::Yes);
    }
    ui ->StartButton ->setDisabled(true);         // Disable  play button
    ui ->actionPlay ->setDisabled(true);         // Disable  play button
    ui ->StopButton ->setEnabled(true);         // Enable  stop button
    ui ->actionPause ->setEnabled(true);         // Enable  stop button
}


void MainWindow::on_StopButton_clicked()
{
    top_block_instance -> stop();
    top_block_instance -> wait();
    ui ->PlayButton ->setEnabled(true);         // Enable  play button
    ui ->actionPlay ->setEnabled(true);         // Enable  play button
    ui ->StopButton ->setDisabled(true);         // Disable  stop button
    ui ->actionPause ->setDisabled(true);         // Disable  stop button


    power_indicator.pause();
    on_change_power_indicator_value(0);
}

void MainWindow::on_QuitButton_clicked()
{
    top_block_instance -> stop();
    this-> close();
    power_indicator.pause();

}

void MainWindow::on_FreqEdit_returnPressed()
{
    //Get frequency from editor
    DataModel -> set_model_freq((ui -> FreqEdit -> text().toDouble()* MEGA));
    if (ui ->PlayButton ->isEnabled()) // USRP's fired up
        top_block_instance -> set_freq_now(DataModel -> get_model_freq());
}

void MainWindow::on_actionBoot_USRP_triggered()
{
    on_StartButton_clicked();
}

void MainWindow::on_actionPause_triggered()
{
    on_StopButton_clicked();
}

void MainWindow::on_actionPlay_triggered()
{
    on_PlayButton_clicked();
}

void MainWindow::on_VolumeSlider_valueChanged(int value)
{
    DataModel -> set_model_volume(value);
    if (ui ->PlayButton ->isEnabled()) // USRP's fired up
        top_block_instance -> set_volume_now (value);

}



void MainWindow::on_volumeButton_clicked()
{
//    ui ->volumeButton ->setDown(true);
    if (ui ->PlayButton ->isEnabled()) // USRP's fired up
        top_block_instance -> set_volume_now (0);
}

void MainWindow::on_change_power_indicator_value(int value)
{
    ui->sound_power_indicator->setValue(value);
}
