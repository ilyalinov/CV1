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

// process video, calculate mean frame, filtered mean frame and detect overlap 
void VideoPlayer::processVideo() {
    VideoCapture newFrameCap(pathToVideo);
    if (!newFrameCap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return;
    }

    VideoWriter v;
    if (videoRecordingFlag) {
        initializeVideoWriter(v, newFrameCap, compressionFactor);
    }

    //ofstream f("f.txt");
    //ofstream tr("t.txt");

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
        resize(frame, frame, outputSize);
        //t1.saveTimePoint();
        //t1.printLastDuration();
        //t1.saveTimePoint();
        
        calculateMean(mean32F, meanCV8U, frame, frameCounter);
        //t1.saveTimePoint();
        //t1.printLastDuration();
        t1.saveTimePoint();
        
        detector->detect(meanCV8U, outputImage);
        t1.saveTimePoint();
        cout << "detector: ";
        t1.printLastDuration();

        Mat i = outputImage;
        //imshow("filtered", i);
        //waitKey(0);

        Mat i1;
        Timer t;
        t.saveTimePoint();
        
        //cv::blur(i, i1, Size(15, 15));
        GaussianBlur(i, i1, Size(9, 9), 0);
        //imshow("gaussian blur", i1);
        //waitKey(0);

        morphologyEx(i1, i1, MORPH_CLOSE, Mat::ones(15, 15, CV_8U));
        //imshow("closing", i1);
        //waitKey(0);

        threshold(i1, i1, 40, 255, THRESH_BINARY);
        //imshow("threshold", i1);
        //waitKey(0);
        //ofstream f(to_string(k) + ".txt");
        //f << i1;
        //f.close();
        Mat stats, centroids, labelImage;
        //t.saveTimePoint();
        int nLabels = connectedComponentsWithStats(i1, labelImage, stats, centroids, 8, CV_32S);
        t.saveTimePoint();
        //t.printLastDuration();

        std::vector<Vec3b> colors(nLabels);
        colors[0] = Vec3b(0, 0, 0);
        std::vector<int> trueLabels;
        std::vector<Mat> trueMasks;
        std::vector<Mat> masks;

        for (int label = 1; label < nLabels; ++label) {
            colors[label] = Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
        }

        Mat dst(i1.size(), CV_8UC3);
        draw_components(i1, dst, labelImage, colors);
        imshow("components", dst);
        //imwrite("E:\\Downloads\\dumps\\1_components\\" + to_string(frameCounter) + ".jpg", dst);

        // points intensity chart
        //Point p1(277, 37);
        //Point p2(165, 162);
        //Vec3b v1 = frame.at<Vec3b>(p1.y, p1.x);
        //Vec3b v2 = frame.at<Vec3b>(p2.y, p2.x);
        //circle(frame, Point(p1), 5, Scalar(0, 255, 0));
        //circle(frame, Point(p2), 5, Scalar(0, 255, 0));
        //int in1 = ((int)(v1.val[0]) + (int)(v1.val[1]) + (int)(v1.val[2])) / 3;
        //int in2 = ((int)(v2.val[0]) + (int)(v2.val[1]) + (int)(v2.val[2])) / 3;
        //f << in1 << "\n";
        //tr << in2 << "\n";

        if (meanStandardDeviationMedianRecording) {
            calculateMeanStandardDeviationMedian(meanCV8U, outputImage);
        }

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

void VideoPlayer::draw_components(Mat& src, Mat& dst, Mat& labelImage, std::vector<Vec3b> colors) {
    for (int r = 0; r < dst.rows; ++r) {
        for (int c = 0; c < dst.cols; ++c) {
            int label = labelImage.at<int>(r, c);
            Vec3b& pixel = dst.at<Vec3b>(r, c);
            pixel = colors[label];
        }
    }
}

void VideoPlayer::calculateMeanStandardDeviationMedian(const cv::Mat& mean, const cv::Mat& outputImage) {
    //ofstream f1("med_mean.txt", ios::app);
    //ofstream f3("med_filter.txt", ios::app);
    ofstream f2("disp_mean.txt", ios::app);
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

    //f1 << med << "\n";
    f2 << disp.at<double>(0, 0) << "\n";

    meanStdDev(outputImage, m, disp);
    // for variance instead of standard deviation use:
    //disp *= disp;
    array.assign(outputImage.begin<uchar>(), outputImage.end<uchar>());
    std::nth_element(array.begin(), array.begin() + array.size() / 2, array.end());
    med = array[array.size() / 2];

    //f3 << med << "\n";
    f4 << disp.at<double>(0, 0) << "\n";
    f5 << m.at<double>(0, 0) << "\n";
    
    //f1.close();
    f2.close();
    //f3.close();
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
    string path = "E:\\Downloads\\dumps\\lapl2_" + to_string(compressionFactor) + "\\";
    // write input frame to file
    imwrite(path + "s\\" + "s" + to_string(frameCounter) + ".jpg", inputFrame);
    // write mean to file
    imwrite(path + "m\\" + "m" + to_string(frameCounter) + ".jpg", mean);
    // write result to file
    imwrite(path + "r\\" + "r" + to_string(frameCounter) + ".jpg", outputImage);
}