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

    const int framesLimit = 500;

    int c = 4;
    Size size((int)newFrameCap.get(CAP_PROP_FRAME_WIDTH), (int)newFrameCap.get(CAP_PROP_FRAME_HEIGHT));
    Size outputSize((int)newFrameCap.get(CAP_PROP_FRAME_WIDTH) / c, (int)newFrameCap.get(CAP_PROP_FRAME_HEIGHT) / c);
    int ex = static_cast<int>(newFrameCap.get(CAP_PROP_FOURCC));
    int fps = static_cast<int>(newFrameCap.get(CAP_PROP_FPS));
    string name = "E:\\Downloads\\dumps\\dft1_" + to_string(c) + ".avi";
    VideoWriter v(name, ex, fps, outputSize, false);
    Mat frame, oldFrame;
    Mat mean(outputSize, CV_32F, Scalar::all(0));
    Mat outputImage;
    Mat meanCV8U;
    int frameCounter = 0;
    Timer t1;
    ofstream f1("med_mean.txt"), f2("disp_mean.txt"), f3("med_filter.txt"), f4("disp_filter.txt"), f5("mean_filter.txt");

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
        
        //t1.saveTimePoint();
        resize(frame, frame, Size(), 1. / c, 1. / c);

        //t1.saveTimePoint();
        //t1.printLastDuration();
        cv::cvtColor(frame, gray, COLOR_BGR2GRAY);
        gray.convertTo(gray, CV_32F, 1.0 / 255.0);
        mean = mean + gray;
        //t1.saveTimePoint();
        //t1.printLastDuration();
        
        int divider = (frameCounter % framesLimit == 0) ? framesLimit : (frameCounter % framesLimit);
        mean.convertTo(meanCV8U, CV_8U, 255.0 * 1.0 / divider);
        // write mean to file
        //imwrite("E:\\Downloads\\dumps\\2lapl100\\m" + to_string(frameCounter) + ".jpg", meanCV8U);

        
        detector->detect(meanCV8U, outputImage);
        //t1.saveTimePoint();
        //cout << "detector: ";
        //t1.printLastDuration();
        v << outputImage;
        if (frameCounter % framesLimit == 0) {
            mean.setTo(0);
        }

        //t1.saveTimePoint();
        //t1.printFullDuration();

        imshow("Mean frame", meanCV8U);
        imshow("Filtered mean frame", outputImage);

        //Mat m, disp;
        //meanStdDev(meanCV8U, m, disp);
        ////disp *= disp;
        //int med = -1;
        //std::vector<int> array;
        //array.assign(meanCV8U.begin<uchar>(), meanCV8U.end<uchar>());
        //std::nth_element(array.begin(), array.begin() + array.size() / 2, array.end());
        //med = array[array.size() / 2];
        //array.clear();

        //f1 << med << "\n";
        //f2 << disp.at<double>(0, 0) << "\n";

        //meanStdDev(outputImage, m, disp);
        ////disp *= disp;
        //array.assign(outputImage.begin<uchar>(), outputImage.end<uchar>());
        //std::nth_element(array.begin(), array.begin() + array.size() / 2, array.end());
        //med = array[array.size() / 2];

        //f3 << med << "\n";
        //f4 << disp.at<double>(0, 0) << "\n";
        //f5 << m.at<double>(0, 0) << "\n";

        // write result to file
        //imwrite("E:\\Downloads\\dumps\\2lapl100\\r" + to_string(frameCounter) + ".jpg", outputImage);
        // write input frame to file
        //imwrite("E:\\Downloads\\dumps\\1lapl100\\s" + to_string(frameCounter) + ".jpg", frame);

        imshow("New frame", frame);
        waitKey(1);
        t1.reset();
    }

    newFrameCap.release();
    f1.close(); 
    f2.close();
    f3.close();
    f4.close();
    f5.close();
    destroyAllWindows();
}