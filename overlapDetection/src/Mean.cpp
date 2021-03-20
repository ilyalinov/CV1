#include "Mean.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>     

using namespace cv;

void Mean::work(cv::Mat& inputFrame, cv::Mat& outputImage, int framesCounter, int framesLimit) 
{
    if (mean32F.empty()) {
        mean32F = Mat(inputFrame.size(), CV_32F, Scalar::all(0));
    }

    Mat gray;
    cv::cvtColor(inputFrame, gray, COLOR_BGR2GRAY);
    gray.convertTo(gray, CV_32F, 1.0 / 255.0);
    mean32F = mean32F + gray;

    int divider = (framesCounter % framesLimit == 0) ? framesLimit : (framesCounter % framesLimit);
    Mat meanCV8U;
    mean32F.convertTo(meanCV8U, CV_8U, 255.0 * 1.0 / divider);
    if (framesCounter % framesLimit == 0) {
        mean32F.setTo(0);
    }

    outputImage = meanCV8U.clone();
}
