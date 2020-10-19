#include <iostream> 
#include <string>   

#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>     
#include <opencv2/videoio.hpp>  
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

int main()
{
    string inputName = "E:\\Downloads\\DR081944.mp4";
    VideoCapture inputVideo(inputName);
    if (!inputVideo.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    Size size((int)inputVideo.get(CAP_PROP_FRAME_WIDTH), (int)inputVideo.get(CAP_PROP_FRAME_HEIGHT));
    Mat frame;
    Mat m(size, CV_64FC3, Scalar::all(0));
    const int framesLimit = 100;
    int frameCounter = 0;
    while (true) {
        inputVideo >> frame;
        if (frame.empty()) {
            break;
        }

        imshow("new frame", frame);
        waitKey(1);
        frame.convertTo(frame, CV_64FC3, 1.0 / 255.0);
        m = m + frame;
        cout << "Frame number: " << frameCounter++ << endl;
        
        if (frameCounter >= framesLimit) {
            int oldFrameNumber = (frameCounter - framesLimit);
            inputVideo.set(CAP_PROP_POS_FRAMES, oldFrameNumber);
            inputVideo >> frame;
            frame.convertTo(frame, CV_64FC3, 1.0 / 255.0);
            m = m - frame;
            inputVideo.set(CAP_PROP_POS_FRAMES, frameCounter);
            Mat mean;
            m.convertTo(mean, CV_8UC3, 255.0 * 1.0 / framesLimit);
            imshow("mean", mean);
            waitKey(1);
        }
    }

    inputVideo.release();
    destroyAllWindows();
    return 0;
}