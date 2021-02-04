#include "VideoPlayer.h"

#include <iostream>
#include <deque>
#include <fstream>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>     
#include <opencv2/videoio.hpp>  

#include "EdgesDetector.h"
#include "Timer.h"

using namespace std;
using namespace cv;

// play video, show mean frame and filtered mean frame 
void VideoPlayer::playVideo() {
    VideoCapture newFrameCap(pathToVideo);
    if (!newFrameCap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return;
    }

    VideoWriter v;
    if (videoRecordingFlag) {
        initializeVideoWriter(v, newFrameCap, compressionFactor);
    }

    Mat frame;
    Size outputSize = getOutputSize(newFrameCap, compressionFactor);
    Mat mean32F(outputSize, CV_32F, Scalar::all(0));
    Mat meanCV8U;
    Mat outputImage;
    int frameCounter = 0;
    Timer t1;
    while (true) {
        frameCounter++;
        cout << "frameCounter = " << frameCounter << "\n";

        //t1.saveTimePoint();
        newFrameCap >> frame;
        //t1.saveTimePoint();
        //t1.printLastDuration();
        if (frame.empty()) {
            return;
        }
        
        //t1.saveTimePoint();
        resize(frame, frame, Size(), 1.0 / compressionFactor, 1.0 / compressionFactor);
        //t1.saveTimePoint();
        
        calculateMean(mean32F, meanCV8U, frame, frameCounter);
        detector->detect(meanCV8U, outputImage);
        //t1.saveTimePoint();
        //cout << "detector: ";
        //t1.printLastDuration();

        calculateMeanStandardDeviationMedian(meanCV8U, outputImage);

        if (videoRecordingFlag) {
            v << outputImage;
        }

        if (saveResultsFlag) {
            saveResults(frame, meanCV8U, outputImage, frameCounter);
        }

        if (showResultsFlag) {
            showResults(frame, meanCV8U, outputImage);
        }
        t1.reset();
    }

    newFrameCap.release();
    destroyAllWindows();
}

void VideoPlayer::initializeVideoWriter(VideoWriter& v, VideoCapture& cap, int compressionFactor) {
    Size outputSize((int)cap.get(CAP_PROP_FRAME_WIDTH) / compressionFactor, (int)cap.get(CAP_PROP_FRAME_HEIGHT) / compressionFactor);
    int ex = static_cast<int>(cap.get(CAP_PROP_FOURCC));
    int fps = static_cast<int>(cap.get(CAP_PROP_FPS));
    string name = "E:\\Downloads\\dumps\\xxxxx_" + to_string(compressionFactor) + ".avi";
    v.open(name, ex, fps, outputSize, false);
}

cv::Size VideoPlayer::getOutputSize(VideoCapture& cap, int compressionFactor) {
    Size outputSize((int)cap.get(CAP_PROP_FRAME_WIDTH) / compressionFactor, (int)cap.get(CAP_PROP_FRAME_HEIGHT) / compressionFactor);
    return outputSize;
}

void VideoPlayer::calculateMean(cv::Mat& mean32F, cv::Mat& meanCV8U, const cv::Mat& inputFrame, int frameCounter) {
    Mat gray;
    cv::cvtColor(inputFrame, gray, COLOR_BGR2GRAY);
    gray.convertTo(gray, CV_32F, 1.0 / 255.0);
    mean32F = mean32F + gray;

    int divider = (frameCounter % framesLimit == 0) ? framesLimit : (frameCounter % framesLimit);
    mean32F.convertTo(meanCV8U, CV_8U, 255.0 * 1.0 / divider);
    if (frameCounter % framesLimit == 0) {
        mean32F.setTo(0);
    }
}

void VideoPlayer::calculateMeanStandardDeviationMedian(const cv::Mat& mean, const cv::Mat& outputImage) {
    ofstream f1("med_mean.txt", ios::app);
    ofstream f2("disp_mean.txt", ios::app);
    ofstream f3("med_filter.txt", ios::app);
    ofstream f4("disp_filter.txt", ios::app);
    ofstream f5("mean_filter.txt", ios::app);
    Mat m, disp;
    
    meanStdDev(mean, m, disp);
    // for variance instead of standard deviation use:
    //disp *= disp;
    int med = -1;
    std::vector<int> array;
    array.assign(mean.begin<uchar>(), mean.end<uchar>());
    std::nth_element(array.begin(), array.begin() + array.size() / 2, array.end());
    med = array[array.size() / 2];
    array.clear();

    f1 << med << "\n";
    f2 << disp.at<double>(0, 0) << "\n";

    meanStdDev(outputImage, m, disp);
    // for variance instead of standard deviation use:
    //disp *= disp;
    array.assign(outputImage.begin<uchar>(), outputImage.end<uchar>());
    std::nth_element(array.begin(), array.begin() + array.size() / 2, array.end());
    med = array[array.size() / 2];

    f3 << med << "\n";
    f4 << disp.at<double>(0, 0) << "\n";
    f5 << m.at<double>(0, 0) << "\n";
    
    f1.close();
    f2.close();
    f3.close();
    f4.close();
    f5.close();
}

void VideoPlayer::showResults(const cv::Mat& inputFrame, const cv::Mat& mean, const cv::Mat& outputImage) {
    imshow("Mean frame", mean);
    imshow("Filtered mean frame", outputImage);
    imshow("New frame", inputFrame);
    waitKey(1);
}

void VideoPlayer::saveResults(const cv::Mat& inputFrame, const cv::Mat& mean, const cv::Mat& outputImage, int frameCounter) {
    // write input frame to file
    imwrite("E:\\Downloads\\dumps\\2lapl100\\s" + to_string(frameCounter) + ".jpg", inputFrame);
    // write mean to file
    imwrite("E:\\Downloads\\dumps\\2lapl100\\m" + to_string(frameCounter) + ".jpg", mean);
    // write result to file
    imwrite("E:\\Downloads\\dumps\\2lapl100\\r" + to_string(frameCounter) + ".jpg", outputImage);
}