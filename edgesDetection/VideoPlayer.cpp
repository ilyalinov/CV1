#pragma once
#include "VideoPlayer.h"
#include "EdgesDetector.h"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"     
#include "opencv2/videoio.hpp"  
#include <iostream>

using namespace std;
using namespace cv;

// play video, show mean frame and filtered mean frame 
void VideoPlayer::playVideo() {
    VideoCapture inputVideo(pathToVideo);
    if (!inputVideo.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return;
    }

    Size size((int)inputVideo.get(CAP_PROP_FRAME_WIDTH), (int)inputVideo.get(CAP_PROP_FRAME_HEIGHT));
    Mat frame;
    Mat m(size, CV_32F, Scalar::all(0));
    const int framesLimit = 100;
    int frameCounter = 0;
    while (true) {
        inputVideo >> frame;
        if (frame.empty()) {
            break;
        }

        imshow("new frame", frame);
        waitKey(1);
        Mat gray;
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        frame.convertTo(frame, CV_32F, 1.0 / 255.0);
        m = m + frame;
        cout << "Frame number: " << frameCounter++ << endl;

        if (frameCounter >= framesLimit) {
            int oldFrameNumber = (frameCounter - framesLimit);
            inputVideo.set(CAP_PROP_POS_FRAMES, oldFrameNumber);
            inputVideo >> frame;
            cvtColor(frame, frame, COLOR_BGR2GRAY);
            frame.convertTo(frame, CV_32F, 1.0 / 255.0);
            m = m - frame;
            inputVideo.set(CAP_PROP_POS_FRAMES, frameCounter);
            Mat mean;
            m.convertTo(mean, CV_8U, 255.0 * 1.0 / framesLimit);
            imshow("Mean frame", mean);
            waitKey(1);
            Mat outputImage;
            detector->detect(mean, outputImage);
            imshow("Filtered mean frame", outputImage);
            waitKey(1);
        }
    }

    inputVideo.release();
    destroyAllWindows();
}
