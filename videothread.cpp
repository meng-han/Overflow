#include "videothread.h"

videoThread::videoThread(QObject *parent) :
    QThread(parent)
{
}

/*!
 * Thread resumes
 */
void videoThread::resume()
{
    this->pauseFlag = false;
    this->pauseCond.wakeAll();
}

/*!
 * Thread pauses
 */
void videoThread::pause()
{
    this->pauseFlag = true;
}

/*!
 * \brief videoThread::run
 * videoThread Starts from here.
 * Override
 */

void videoThread::run()
{
    //this->sync.lock();

    // Initialize model
    this->init();
    int cnt = 0;    //Count of frame numbers
    while(this->capture.read(this->frame))
    {
        double overflowAmount = 0;
        //If stop button is pressed, stop this thread.
        if(this->Stop) break;
        //frame count ++
        cnt ++;
        //Emit signal for progress bar: current frame count with respect to the overall video
        emit videoProgress(cnt * 100.0 / this->totalFrameNumber);
        //If current frame is to be processed, then process it.
        //If not, pass; go to next frame.
        if (cnt % this->fpsMod == 0)
        {
            //This is to solve problem of iPhone video capturing.
            //1. Make is portrait
            //2. Resize resolution to (480 * 854)
            if (this->width > this->height)
            {
                resize(this->frame, this->frame, cv::Size(854, 480));
                cv::transpose(this->frame, this->frame);
                cv::flip(this->frame, this->frame, 1);
            } else {
                resize(this->frame, this->frame, cv::Size(480, 854));}

            //Perform background subtraction
            this->bgs->process(this->frame, this->background, this->bgModel);
            //If this is the third frame that is processed, then go on;
            //If not, then initialize background subtraction model (Require first two frames to initialize);
            //Then go to next frame.
            if (cnt > 2 * this->fpsMod)
            {
                //Detect motion
                bool motion = detectMotion();
                //If there is motion, go on processing;
                //If there is no motion, pass current frame, go to next;
                if (motion)
                {
                    //Find contours after motion is detected
                    int contour_size = findContoursOnMotion();
                    //Delete small contours, regard as noise
                    this->deleteSmallContours(contour_size);
                    //Update contour_size
                    contour_size = (int)this->contours.size();
                    //If there is no overflow
                    if (!this->overflowDetected)
                    {
                        //Check if there is overflow
                        this->checkOverflow(contour_size, cnt);
                    } else {
                        //Check if there is detection
                        if (this->contours.size() > 0)
                        {
                            //Check which index of contour is overflow
                            int max_hist_index = prepareMeasurement();
                            //Quantify overflow
                            overflowAmount = measureOverflow(subClosing);
                            //Draw contours on the original frame
                            drawContours( this->frame, contours, max_hist_index, redColor, 2, 8, hierarchy, 0, Point() );
                        }
                    }
                } else {
                    this->overflowDetected = false;
                }
            }
            //Write the frame to the resulting video
            this->wtr.write(this->frame);
            //string saveImageAdd = QCoreApplication::applicationDirPath().toStdString() + "//image" + std::to_string(cnt) + ".png";
            //cout << saveImageAdd << endl;
            //cout << this->overflowDetected << endl;

            //Show result in another window
            emit showResult(this->frame);
            //if (!subClosing.empty())
            //{
                //emit showResult(subClosing);
            //    imwrite(saveImageAdd, this->frame);
            //}
            //emit flow rate for plotting
            emit flowRate(cnt, overflowAmount);
        }

    }
    //Release video writer
    this->wtr.release();
    //indicate video thread is finished
    emit finished(true);
    //this->sync.unlock();
}

void videoThread::init()
{
    this->bgs = new WeightedMovingVarianceBGS;
    this->capture.open(this->videoFileName);
    this->width = this->capture.get(CV_CAP_PROP_FRAME_WIDTH);
    this->height = this->capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    this->fps = this->capture.get(CV_CAP_PROP_FPS);
    if (this->fps == 29) this->fps = 30;
    this->fpsMod = this->fps / TARGET_FPS; //Target FPS is 5
    this->totalFrameNumber = this->capture.get(CV_CAP_PROP_FRAME_COUNT);
    string xmlFileDirectory = QCoreApplication::applicationDirPath().toStdString() + "//config2.xml";
    FileStorage fs(xmlFileDirectory, FileStorage::READ);
    FileNode matFileNode = fs["histMat"];
    read( matFileNode, this->trainedSubfigure);

    FileNode contourFileNode = fs["contour"];
    read(contourFileNode, this->contour);

    fs.release();
    for(int i = 0; i < 3; i++) this->correct[i] = 0;
    this->overflowDetected = false;
    this->videoSaveAddress = this->getSaveVideoAddress(this->videoFileName);
    this->wtr.open(videoSaveAddress, CV_FOURCC('X', 'V', 'I', 'D'), TARGET_FPS, cv::Size(480, 854), true);

}

string videoThread::getSaveVideoAddress(string tempFileName)
{
    //Get name
    size_t index = tempFileName.find_last_of("/");
    if(index == string::npos) {
        index = tempFileName.find_last_of("\\");
    }
    size_t index2 = tempFileName.find_last_of(".");
    string prefix = tempFileName.substr(0,index+1);
    //string suffix = tempFileName.substr(index2);
    string name = tempFileName.substr(index+1, index2-index-1);
    //Find name ending
    string string1 = prefix;
    string string2 = name + "_Processed";
    string string3 = ".avi";
    string saveAddress = string1 + string2 + string3;
    return saveAddress;
}

/**
 * @brief videoThread::detectMotion
 * @return
 * 1. Morphological closing
 * 2. Count number of white pixels (pixel with motion)
 * 3. Calculate white pixel percentage
 * 4. Compare with motion detection threshold.
 */
bool videoThread::detectMotion()
{
    morphologyEx(this->background, this->img_closing, MORPH_CLOSE, this->element);
    int numOfWhitePixels = countNonZero(this->img_closing);
    double percentageWhitePixels = numOfWhitePixels * 1.0 / (TARGET_RESOLUTION_HEIGHT * TARGET_RESOLUTION_WIDTH);
    return percentageWhitePixels > MOTION_THRESHOLD ? true : false;
}

int videoThread::findContoursOnMotion()
{

    Mat img_closing_rgb;
    cvtColor(this->img_closing, img_closing_rgb, CV_GRAY2RGB);
    bitwise_and(this->frame, img_closing_rgb, and_mask);
    Mat threshold_output;

    int thresh = 100;
    threshold( this->img_closing, threshold_output, thresh, 10, THRESH_BINARY );
    findContours( threshold_output, this->contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    return (int)contours.size();
}

void videoThread::deleteSmallContours(int contour_size)
{
    if (contour_size > 0)
    {
        for( int i = contour_size - 1; i >= 0; i-- )
        {
            double area = contourArea( this->contours[i] );
            double percentageContourArea = area * 1.0 / (854 * 480);
            if (percentageContourArea < 0.004) this->contours.erase(this->contours.begin() + i);
        }
    }
}
double videoThread::compareHistogram(Mat img1, Mat img2)
{
    Mat img1_hsv, img2_hsv;
    cvtColor( img1, img1_hsv, COLOR_BGR2HSV );
    cvtColor( img2, img2_hsv, COLOR_BGR2HSV );
    int h_bins = 50; int s_bins = 60;
    int histSize[] = { h_bins, s_bins };
    float h_ranges[] = { 0, 180 };
    float s_ranges[] = { 0, 256 };
    const float* ranges[] = { h_ranges, s_ranges };
    // Use the o-th and 1-st channels
    int channels[] = { 0, 1 };
    MatND hist_1;
    calcHist( &img1_hsv, 1, channels, Mat(), hist_1, 2, histSize, ranges, true, false );
    normalize( hist_1, hist_1, 0, 1, NORM_MINMAX, -1, Mat() );


    MatND hist_2;
    calcHist( &img2_hsv, 1, channels, Mat(), hist_2, 2, histSize, ranges, true, false );
    normalize( hist_2, hist_2, 0, 1, NORM_MINMAX, -1, Mat() );
    return compareHist( hist_1, hist_2, 0 );
}

void videoThread::checkOverflow(int contour_size, int cnt)
{
    for (int i = 0; i < contour_size; i++)
    {
        Rect bdRect = boundingRect(this->contours[i]);
        this->subFigure = and_mask(bdRect);
        double comp = this->compareHistogram(this->subFigure, this->trainedSubfigure);
        double match_points = matchShapes(this->contour, this->contours[i], CV_CONTOURS_MATCH_I2, 0);
        //MAKE THE CHECK 5 CONSECUTIVE CHECK!!!!!
        if (comp > 0.999 && match_points < 8)
        {
            if (this->correct[0]==0) {this->overflowDetected = false; this->correct[0] = cnt; break;}
            else if (this->correct[0] == cnt - fpsMod)
            {
                this->overflowDetected = false;
                this->correct[1] = cnt;
                break;
            }

            else if (this->correct[0] == this->correct[1] - fpsMod && this->correct[1] == cnt - fpsMod)
            {
                this->correct[2] = cnt;
                this->overflowDetected = true;
                break;
            }
            else {
                this->overflowDetected = false;
                this->correct[0] = cnt;
                this->correct[1] = 0;
            }
        }
    }
}

int videoThread::prepareMeasurement()
{
    int contour_size = (int) this->contours.size();
    double max_comp = 0;
    int max_hist_index = 0;
    for (int i = 0; i < contour_size; i++)
    {
        Rect bdRect = boundingRect(this->contours[i]);
        this->subFigure = and_mask(bdRect);
        double comp = compareHistogram(this->subFigure, this->trainedSubfigure);
        if (comp > max_comp)
        {
            max_hist_index = i;
            max_comp = comp;
        }
    }
    Rect bdRect = boundingRect(contours[max_hist_index]);
    trainedSubfigure = and_mask(bdRect);
    this->subClosing = img_closing(bdRect);
    return max_hist_index;
}

double videoThread::measureOverflow(Mat& subClosing)
{
    //First find 15 candidate points for fitting parabola
    int Y[15];
    int X[15];
    //Find 15 white pixels on the bottom border of the overflow region
    for (int i = 0; i < 15; i++)
    {
        Y[i] = ceil(subClosing.rows * (0.2 + i * 0.05));
        for (int j = 0; j < subClosing.cols; j ++)
        {
            if (subClosing.at<uchar>(Y[i], j) == 255)
            {
                X[i] = j;
                break;
            }
        }
    }
    int cntCorrect = 0;
    int cntPossible = 0;
    double A = 0;
    double B = 0;
    double C = 0;

    while(true)
    {
        cntPossible ++;
        if (cntPossible >= 105) //cntPossible equals to C15/2 = 15 * 14 / 2
        {
            break;
        }
        int index1 = rand() % 15; //Randomly pick the first point
        int index2 = index1;
        while(index2 == index1) //Pick the random second point, different from first one
        {
            index2 = rand() % 15;
        }
        int index3 = index2;
        while(index3 == index2 || index3 == index1) //Randomly pick the third point, different from first and second one.
        {
            index3 = rand() % 15;
        }
        //Record the pixel coordinates of three points
        int X1 = X[index1];
        int Y1 = Y[index1];
        int X2 = X[index2];
        int Y2 = Y[index2];
        int X3 = X[index3];
        int Y3 = Y[index3];
        //Begin fitting curve
        //Determinate of the matrix to solve for A, b, and c;
        double det = X1 * X1 * (X2 - X3) - X1 * (X2 * X2 - X3 * X3) + (X2 * X2 * X3 - X3 * X3 * X2);
        if (det == 0) continue;
        double a11 = (X2 - X3) * 1.0 / det;
        double a12 = (X3 - X1) * 1.0 / det;
        double a13 = (X1 - X2) * 1.0 / det;
        double a21 = (X3 * X3 - X2 * X2) / det;
        double a22 = (X1 * X1 - X3 * X3) / det;
        double a23 = (X2 * X2 - X1 * X1) / det;
        double a31 = (X2 * X2 * X3 - X3 * X3 * X2) / det;
        double a32 = (X3 * X3 * X1 - X1 * X1 * X3) / det;
        double a33 = (X1 * X1 * X2 - X2 * X2 * X1) / det;
        double tempA = a11 * Y1 + a12 * Y2 + a13 * Y3;
        double tempB = a21 * Y1 + a22 * Y2 + a23 * Y3;
        double tempC = a31 * Y1 + a32 * Y2 + a33 * Y3;

        if (tempA < 0 || tempB < 0 || tempC < 0) continue;
        else {
            A += tempA;
            B += tempB;
            C += tempC;
            cntCorrect ++;
        }

    }
    //Average the result from random picked combinations
    if (cntCorrect != 0)
    {
        A /= cntCorrect;
        B /= cntCorrect;
        C /= cntCorrect;
    }
    //Average the current result with last two solutions
    A = 0.5 * A + 0.3 * prev_A + 0.2 * prev2_A;
    B = 0.5 * B + 0.3 * prev_B + 0.2 * prev2_B;
    C = 0.5 * C + 0.3 * prev_C + 0.2 * prev2_C;
    //Update the current with last result
    prev2_A = prev_A;
    prev2_B = prev_B;
    prev2_C = prev_C;
    prev_A = A;
    prev_B = B;
    prev_C = C;

    int width = subClosing.cols;
    //float miny = -1, maxy = 1;
    vector<Point2f> list_point(width);
    for(int i = 0; i < width; i++){
        list_point[i].x = i;
        list_point[i].y = A*i*i+B*i+C;
    }
    //Draw the curve
    cvtColor(subClosing, subClosing, CV_GRAY2RGB);
    for(int i = 1; i < width; i++) line(subClosing,list_point[i-1],list_point[i],Scalar(0,0,255), 2);

    //Get flow depth, with respect to height, in inches
    double flowDepth = C / (subClosing.rows - C) * H;

    int tempCC = C - subClosing.rows;
    //-b + sqrt(b2 - 4ac) / 2a
    double horizontalDistance = (-B + sqrt(B*B - 4*A*tempCC)) / 2 / A; //Pixel distance
    horizontalDistance = horizontalDistance / (subClosing.rows - C) * H; //Real world dimension
    //Radii
    double R = D / 2;
    //flow area in the cross section
    double crossArea = (atan2(sqrt(2*R*flowDepth - flowDepth * flowDepth), R - flowDepth) * R * R) - (R - flowDepth) * sqrt(2 * R * flowDepth - flowDepth * flowDepth);
    //Average area with last two
    crossArea = 0.5 * crossArea + 0.3 * prev_area + 0.2 * prev2_area; //Area in inch^2
    //Update last two areas
    prev2_area = prev_area;
    prev_area = crossArea;
    // Travelling time and velocity
    double t = sqrt(2 * H /385.82677); //Travelling time, in s
    double v = horizontalDistance / t; //horizontal velocity, in inch/s
    //Update last two velocities
    v = 0.5 * v + 0.3 * prev_v + 0.2 * prev2_v;
    prev2_v = prev_v;
    prev_v = v;
    //1 INCHES^3/s equals to 0.26 Gallon Per Minute
    return crossArea * v * 0.26;
}
