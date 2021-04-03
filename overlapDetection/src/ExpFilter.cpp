#include "ExpFilter.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>   
#include <opencv2/highgui.hpp>

using namespace cv;

void ExpFilter::work(cv::Mat& inputFrame, cv::Mat& outputImage, int framesCounter, int framesLimit) {
    Mat gray;
    inputFrame.convertTo(gray, CV_32F, 1.0 / 255.0);
    if (framesCounter == 1) {
        acc32F = gray;
    }
    else {
        acc32F = alpha * gray + (1 - alpha) * acc32F;
    }

    normalize(acc32F, outputImage, 0, 1, NORM_MINMAX);
    outputImage.convertTo(outputImage, CV_8U, 255.0);
}