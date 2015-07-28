#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

//Qt Headers
#include <QThread>
#include <QStringList>
#include <QtCore>
#include <QDateTime>
#include <QFileInfo>

//Windows headers
#include "Windows.h"

//OpenCV Headers
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//C++ Headers
#include <iostream>
#include <ctime>
#include "WeightedMovingVarianceBGS.h"

using namespace std;
using namespace cv;


class videoThread : public QThread
{
    Q_OBJECT
public:
    explicit videoThread(QObject *parent = 0);

    //Thread Control functions
    void run(); //Overwrite qthread 'run' function as the main function of this thread. When thread starts, it starts here
    void resume(); //Self defined resume and pause functions. Use QMutex.
    void pause(); //Used for pausing and resuming thread when a dialog needs to collect user preference
    bool Stop;  //No built-in thread stop function. Use a flag to denote whether to stop.
    bool pauseFlag; //To help determine whether the resizing process is stopped by user or finished naturally
    string videoFileName;
    Mat frame;
    double H ;        //Horizontal distance
    double D ;           //Diameter of horizontal pipe
    int fps;
    string videoSaveAddress;
private:
    QMutex sync; //QThread control. lock or unlock
    QWaitCondition pauseCond; //QThread pause condition
    WeightedMovingVarianceBGS* bgs;
    Mat background;
    Mat bgModel;
    VideoCapture capture;
    VideoWriter wtr;
    int width;
    int height;
    int totalFrameNumber;

    int fpsMod;
    Mat subFigure;
    Mat prev_subFigure;
    string getSaveVideoAddress(string tempFileName);

    Mat trainedSubfigure;
    vector<Point> contour;

    Mat img_closing;
    const double THRESHOLD_WHITE = 0.002;

    int morph_size = 10;
    Mat element = getStructuringElement( MORPH_RECT, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );

    void init();
    bool detectMotion();
    vector<vector<Point> > contours;
    void deleteSmallContours(int contour_size);

    bool overflowDetected;
    double compareHistogram(Mat img1, Mat img2);
    int correct[3];
    double measureOverflow(Mat& subClosing);

    double prev2_A = 0;
    double prev2_B = 0;
    double prev2_C = 0;
    double prev_A = 0;
    double prev_B = 0;
    double prev_C = 0;

    double prev_area = 0;
    double prev2_area = 0;

    double prev_v = 0;
    double prev2_v = 0;
    const int TARGET_FPS = 5;
    const int TARGET_RESOLUTION_HEIGHT = 854;
    const int TARGET_RESOLUTION_WIDTH = 480;
    const int MOTION_THRESHOLD = 0.0001;
    const Scalar redColor = cv::Scalar( 0, 0, 255 );
    int findContoursOnMotion();
    void checkOverflow(int contour_size, int cnt);
    Mat and_mask;
    vector<Vec4i> hierarchy;
    Mat subClosing;
    int prepareMeasurement();







signals:
    void showResult(Mat);
    void flowRate(int, double);
    void videoProgress(double);
    void finished(bool);

public slots:

};

#endif // VIDEOTHREAD_H
