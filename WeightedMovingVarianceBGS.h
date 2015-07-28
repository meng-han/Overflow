/*
This file is part of BGSLibrary.

BGSLibrary is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

BGSLibrary is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BGSLibrary.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __WMV_INCLUDED__   // if x.h hasn't been included yet...
#define __WMV_INCLUDED__   //   #define this so the compiler knows it has been include

#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "IBGS.h"
#include <QtCore>

using namespace std;
using namespace cv;

class WeightedMovingVarianceBGS : public IBGS
{
private:
    bool firstTime;
    cv::Mat img_input_prev_1;
    cv::Mat img_input_prev_2;
    bool enableWeight;
    bool enableThreshold;
    int threshold;
    bool showOutput;

public:
    WeightedMovingVarianceBGS();
    ~WeightedMovingVarianceBGS();

    cv::Mat computeWeightedMean(const std::vector<cv::Mat> &v_img_input_f, const std::vector<double> weights);
    cv::Mat computeWeightedVariance(const cv::Mat &img_input_f, const cv::Mat &img_mean_f, const double weight);

    void process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel);

private:
    void saveConfig();
    void loadConfig();
};

#endif
