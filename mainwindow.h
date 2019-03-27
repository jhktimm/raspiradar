#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/time.h>
#include <math.h>
#include "kiss_fft.h"
#include <bitset>
#include <iostream>
#include <thread>

#include <QString>
#include <QFileDialog>
#include <QVector>
#include <QProcess>
#include <QTimer>
#include <QThread>
#include <QTimer>
#include <QMainWindow>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    const char *device;
    uint32_t mode;
    uint8_t bits;
    uint32_t speed;
    uint16_t delay;
    int transfer_size;
    
    int fd;
    int  samples;
    uint16_t* values;
    uint16_t* fftvalues;
    uint8_t* tx;
    uint8_t* rx;
    size_t len;

    QTimer daqTimer;

    struct spi_ioc_transfer spiiotr;
    
    void check_spi();

    void init_std_values();
    
 //   void on_actionOszi_mode_triggered(bool checked);
   // void on_actionRefresh_triggered();
    //void on_actionOpen_device_triggered();

private slots:
    void on_actionOpen_device_triggered();

    void on_actionRefresh_triggered();

    void on_actionOszi_mode_triggered(bool checked);

    void on_actionExit_triggered();

private:
    void loop();
    
    bool osziMode = false;
    
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
