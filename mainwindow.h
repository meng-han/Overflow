#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//Qt Headers
#include <QThread>
#include <QStringList>
#include <QtCore>
#include <QDateTime>
#include <QFileInfo>
#include <QFileDialog>

//Windows headers
#include "Windows.h"

//OpenCV Headers
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include "videothread.h"
#include "aboutdialog.h"
//C++ Headers
#include <iostream>
#include <ctime>


using namespace std;
using namespace cv;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_openPushButton_clicked();
    void on_runPushButton_clicked();
    void on_stopPushButton_clicked();
    void on_flowRate(int, double);

    void on_actionOpen_triggered();
    void on_showResult(Mat);
    void on_videoProgress(double);
    void on_finished(bool);
    void on_savePushButton_clicked();

    void on_actionRun_triggered();

    void on_actionStop_triggered();

    void on_actionExit_triggered();

    void on_actionAbout_triggered();

    void on_diameterPushButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    videoThread vt;
    QVector<double> x;
    QVector<double> y;
    void initPlot();
    int xRng = 60; //60s
    int yRng = 40; //40GPM

};

#endif // MAINWINDOW_H
