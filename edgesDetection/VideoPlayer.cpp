#pragma once
#include "VideoPlayer.h"
#include "EdgesDetector.h"
#include "Timer.h"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"     
#include "opencv2/videoio.hpp"  
#include <iostream>

using namespace std;
using namespace cv;

// play video, show mean frame and filtered mean frame 
void VideoPlayer::playVideo() {
    VideoCapture newFrameCap(pathToVideo);
    VideoCapture oldFrameCap(pathToVideo);
    if (!newFrameCap.isOpened() || !oldFrameCap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return;
    }

    const int framesLimit = 100;

    Size size((int)newFrameCap.get(CAP_PROP_FRAME_WIDTH), (int)newFrameCap.get(CAP_PROP_FRAME_HEIGHT));
    Mat frame, oldFrame;
    Mat mean(size, CV_32F, Scalar::all(0));
    int frameCounter = 0;

    while (true) {
        cout << "New frame" << '\n';
        newFrameCap >> frame;
        if (frame.empty()) {
            break;
        }

        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        gray.convertTo(gray, CV_32F, 1.0 / 255.0);        
        mean = mean + gray;

        cout << "Frame number: " << frameCounter << "\n";
        frameCounter++;

        if (frameCounter >= framesLimit) {
            oldFrameCap >> oldFrame;
            if (oldFrame.empty()) {
                break;
            }
            
            Mat gray1;
            cvtColor(oldFrame, gray1, COLOR_BGR2GRAY);
            gray1.convertTo(gray1, CV_32F, 1.0 / 255.0);            
            mean = mean - gray1;

            Mat meanCV8U;
            mean.convertTo(meanCV8U, CV_8U, 255.0 * 1.0 / framesLimit);
            imshow("Mean frame", meanCV8U);
            
            Mat outputImage;
            detector->detect(meanCV8U, outputImage);
            imshow("Filtered mean frame", outputImage);
        }

        imshow("New frame", frame);
        waitKey(1);
    }

    newFrameCap.release();
    oldFrameCap.release();
    destroyAllWindows();
}