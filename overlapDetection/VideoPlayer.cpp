#pragma once
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
    VideoCapture oldFrameCap(pathToVideo);
    if (!newFrameCap.isOpened() || !oldFrameCap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return;
    }

    const int framesLimit = 500;
    deque<Mat> frames;

    Size size((int)newFrameCap.get(CAP_PROP_FRAME_WIDTH), (int)newFrameCap.get(CAP_PROP_FRAME_HEIGHT));
    Mat frame, oldFrame;
    Mat mean(size, CV_32F, Scalar::all(0));
    int frameCounter = 0;
    mutex mtx, mDeque;
    thread tNew, tOld;
    Timer t1, t2;

    while (true) {
        Mat gray1;
        frameCounter++;
        cout << "frameCounter = " << frameCounter << "\n";

        if (!tNew.joinable()) {
            tNew = std::thread([&]() {
                //t1.saveTimePoint();
                newFrameCap >> frame;
                t1.saveTimePoint();
                {
                    lock_guard<mutex> lock(mDeque);
                    frames.push_front(frame.clone());
                }
                cv::cvtColor(frame, gray1, COLOR_BGR2GRAY);
                gray1.convertTo(gray1, CV_32F, 1.0 / 255.0);
                {
                    lock_guard<mutex> lock(mtx);
                    mean = mean + gray1;
                }
                t1.saveTimePoint();
            });
        }

        if (frameCounter >= framesLimit) {
            if (!tOld.joinable()) {
                tOld = std::thread([&]() {
                    t2.saveTimePoint();
                    //oldFrameCap >> oldFrame;
                    Mat oldFrame;
                    {
                        lock_guard<mutex> lock(mDeque);
                        oldFrame = frames.back();
                        frames.pop_back();
                    }
                    Mat gray2;
                    cvtColor(oldFrame, gray2, COLOR_BGR2GRAY);
                    gray2.convertTo(gray2, CV_32F, 1.0 / 255.0);
                    {
                        lock_guard<mutex> lock(mtx);
                        mean = mean - gray2;
                    }
                    t2.saveTimePoint();
                });
            }

            tNew.join();
            tOld.join();

            cout << "1 thread: ";
            t1.printLastDuration();
            cout << "2 thread: ";
            t2.printLastDuration();

            Mat meanCV8U;
            mean.convertTo(meanCV8U, CV_8U, 255.0 * 1.0 / framesLimit);
            //imshow("Mean frame", meanCV8U);
            // write mean to file
            //imwrite("E:\\Downloads\\dumps\\2lapl100\\m" + to_string(frameCounter) + ".jpg", meanCV8U);

            t1.saveTimePoint();
            Mat outputImage;
            detector->detect(meanCV8U, outputImage);
            t1.saveTimePoint();
            cout << "detector: ";
            t1.printLastDuration();
            t1.printFullDuration();
            imshow("Filtered mean frame", outputImage); 
            // write result to file
            //imwrite("E:\\Downloads\\dumps\\2lapl100\\r" + to_string(frameCounter) + ".jpg", outputImage);
        }
        else {
            tNew.join();
            //t1.printLastDuration();
        }
        
        imshow("New frame", frame);
        waitKey(1);
        t1.reset();
        t2.reset();
    }

    newFrameCap.release();
    oldFrameCap.release();
    destroyAllWindows();
}