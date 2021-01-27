#include "VideoPlayer.h"

#include <iostream>
#include <deque>

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

    const int framesLimit = 500;
    deque<Mat> frames;

    Size size((int)newFrameCap.get(CAP_PROP_FRAME_WIDTH), (int)newFrameCap.get(CAP_PROP_FRAME_HEIGHT));
    Mat frame, oldFrame;
    Mat mean(size, CV_32F, Scalar::all(0));
    Mat outputImage;
    Mat meanCV8U;
    int frameCounter = 0;
    Timer t1;

    while (true) {
        Mat gray;
        frameCounter++;
        cout << "frameCounter = " << frameCounter << "\n";

        //t1.saveTimePoint();
        newFrameCap >> frame;
        //t1.saveTimePoint();
        if (frame.empty()) {
            return;
        }

        cv::cvtColor(frame, gray, COLOR_BGR2GRAY);
        gray.convertTo(gray, CV_32F, 1.0 / 255.0);
        mean = mean + gray;
        //t1.saveTimePoint();
        //t1.printLastDuration();
        
        if (frameCounter % framesLimit == 0) {
            mean.convertTo(meanCV8U, CV_8U, 255.0 * 1.0 / framesLimit);
            // write mean to file
            //imwrite("E:\\Downloads\\dumps\\2lapl100\\m" + to_string(frameCounter) + ".jpg", meanCV8U);

            //t1.saveTimePoint();
            detector->detect(meanCV8U, outputImage);
            mean.setTo(0);
            //t1.saveTimePoint();
            //cout << "detector: ";
            //t1.printLastDuration();
        }

        t1.saveTimePoint();
        t1.printFullDuration();

        if (frameCounter >= framesLimit) {
            imshow("Mean frame", meanCV8U);
            imshow("Filtered mean frame", outputImage);
        }

        // write result to file
        //imwrite("E:\\Downloads\\dumps\\2lapl100\\r" + to_string(frameCounter) + ".jpg", outputImage);

        imshow("New frame", frame);
        waitKey(1);
        t1.reset();
    }

    newFrameCap.release();
    destroyAllWindows();
}