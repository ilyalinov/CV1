#include "VideoHandler.h"

#include <iostream>
#include <fstream>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>     
#include <opencv2/videoio.hpp>  

#include "EdgesDetector.h"
#include "Laplacian.h"
#include "DFTFilter.h"
#include "Timer.h"

using namespace std;
using namespace cv;

VideoHandler::VideoHandler(Configuration* c) {
    configuration = c;
    EdgesDetector::DetectorType t = configuration->getEdgesDetectorType();
    detector = nullptr;
    if (t == EdgesDetector::DetectorType::laplacian) {
        detector = new ::Laplacian();
    }
    else if (t == EdgesDetector::DetectorType::dft) {
        detector = new DFTFilter();
    }

    string pathToVideo = configuration->getPathToVideo();
    newFrameCap.open(pathToVideo);
    if (!newFrameCap.isOpened()) {
        // throw exception? 
        cout << "Error opening video stream or file" << endl;
        return;
    }

    if (configuration->hasVideoRecording()) {
        initializeVideoWriter();
    }

}

// process video, calculate mean frame, filtered mean frame and detect overlap 
void VideoHandler::processVideo() {
    Mat frame;
    Size outputSize = getOutputSize();
    Mat mean32F(outputSize, CV_32F, Scalar::all(0));
    Mat mask(outputSize, CV_8U, Scalar::all(0));
    Mat meanCV8U;
    Mat filteredImage;
    int frameCounter = 0;
    Timer t1;
    std::vector<Mat> masks;
    while (true) {
        frameCounter++;
        cout << "frameCounter = " << frameCounter << "\n";
        
        // read input frame
        newFrameCap >> frame;

        if (frame.empty()) {
            break;
        }
        
        Timer t;
        t.saveTimePoint();

        // resize image by compressionFactor times vertically and horizontally  
        resize(frame, frame, outputSize);
        
        // calculate mean frame as CV_8U and CV_32F Mat types
        calculateMean(mean32F, meanCV8U, frame, frameCounter);
        
        // apply laplacian or DFT high pass filter
        detector->detect(meanCV8U, filteredImage);

        // find clusters
        Mat i = filteredImage;
        Mat i1;
        GaussianBlur(i, i1, Size(9, 9), 0);
        morphologyEx(i1, i1, MORPH_CLOSE, Mat::ones(15, 15, CV_8U));
        threshold(i1, i1, 40, 255, THRESH_BINARY);
        Mat stats, centroids, labelImage;
        int nLabels = connectedComponentsWithStats(i1, labelImage, stats, centroids, 8, CV_32S);
        
        
        // find masks overlap
        if (frameCounter % configuration->getFramesLimit() == 0) {
            masks.push_back(labelImage);
            Mat tmp = (labelImage != 0);
            threshold(tmp, tmp, 0, 1, THRESH_BINARY);
            mask = mask + tmp;
            Mat cmp(outputSize, CV_8U, Scalar::all((int)(masks.size() / 2)));
            cmp = mask > cmp;
            imshow("mask", cmp);
            //imwrite("mask" + to_string(frameCounter) + ".jpg", cmp);
        }
        

        //Mat mask(outputSize, CV_8U, Scalar::all(0));
        //if (frameCounter % framesLimit == 0) {
        //    masks.push_back(labelImage);
        //    for (int i = 0; i < labelImage.rows; i++) {
        //        for (int j = 0; j < labelImage.cols; j++) {
        //            int counter = 0;
        //            for (int k = 0; k < masks.size(); k++) {
        //                Scalar s = masks[k].at<int>(i, j);
        //                int intensity = s.val[0];
        //                if (intensity != 0) {
        //                    counter++;
        //                }
        //            }

        //            if (counter > masks.size() / 2) {
        //                mask.at<uchar>(i, j, 0) = 255;
        //            }
        //        }
        //    }

        //    //imshow("mask", mask);
        //    //imwrite("mask" + to_string(frameCounter) + ".jpg", mask);
        //}

        
        // draw colored connected components image
        std::vector<Vec3b> colors(nLabels);
        colors[0] = Vec3b(0, 0, 0);
        std::vector<int> trueLabels;
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

        if (configuration->hasMeanStandardDeviationMedianRecording()) {
            calculateMeanStandardDeviationMedian(meanCV8U, filteredImage);
        }

        if (configuration->hasVideoRecording()) {
            v << filteredImage;
        }

        if (configuration->hasSaveResultsFlag()) {
            saveResults(frame, meanCV8U, filteredImage, frameCounter);
        }

        if (configuration->hasShowResultsFlag()) {
            showResults(frame, meanCV8U, filteredImage);
        }
        t.reset();
        t1.reset();
    }

    destroyAllWindows();
}

VideoHandler::~VideoHandler() {
    newFrameCap.release();
    delete this->detector;
}

void VideoHandler::initializeVideoWriter() {
    const int compressionFactor = configuration->getCompressionFactor();
    Size outputSize((int)newFrameCap.get(CAP_PROP_FRAME_WIDTH) / compressionFactor, (int)newFrameCap.get(CAP_PROP_FRAME_HEIGHT) / compressionFactor);
    //int ex = (int)(cap.get(CAP_PROP_FOURCC));
    int fps = (int)(newFrameCap.get(CAP_PROP_FPS));
    string videoName = "E:\\Downloads\\dumps\\xxxxx_" + to_string(compressionFactor) + ".avi";
    int codec = VideoWriter::fourcc('M', 'J', 'P', 'G');
    v.open(videoName, codec, fps, outputSize, false);
}

cv::Size VideoHandler::getOutputSize() {
    const int compressionFactor = configuration->getCompressionFactor();
    Size outputSize((int)newFrameCap.get(CAP_PROP_FRAME_WIDTH) / compressionFactor, (int)newFrameCap.get(CAP_PROP_FRAME_HEIGHT) / compressionFactor);
    return outputSize;
}

void VideoHandler::calculateMean(cv::Mat& mean32F, cv::Mat& meanCV8U, const cv::Mat& inputFrame, int frameCounter) {
    Mat gray;
    cv::cvtColor(inputFrame, gray, COLOR_BGR2GRAY);
    gray.convertTo(gray, CV_32F, 1.0 / 255.0);
    mean32F = mean32F + gray;

    const int framesLimit = configuration->getFramesLimit();
    int divider = (frameCounter % framesLimit == 0) ? framesLimit : (frameCounter % framesLimit);
    mean32F.convertTo(meanCV8U, CV_8U, 255.0 * 1.0 / divider);
    if (frameCounter % framesLimit == 0) {
        mean32F.setTo(0);
    }
}

void VideoHandler::draw_components(Mat& src, Mat& dst, Mat& labelImage, std::vector<Vec3b>& colors) {
    for (int r = 0; r < dst.rows; ++r) {
        for (int c = 0; c < dst.cols; ++c) {
            int label = labelImage.at<int>(r, c);
            Vec3b& pixel = dst.at<Vec3b>(r, c);
            pixel = colors[label];
        }
    }
}

void VideoHandler::calculateMeanStandardDeviationMedian(const cv::Mat& mean, const cv::Mat& outputImage) {
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

void VideoHandler::showResults(const cv::Mat& inputFrame, const cv::Mat& mean, const cv::Mat& filteredImage) {
    imshow("Mean frame", mean);
    imshow("Filtered mean frame", filteredImage);
    imshow("New frame", inputFrame);
    waitKey(1);
}

void VideoHandler::saveResults(const cv::Mat& inputFrame, const cv::Mat& mean, const cv::Mat& outputImage, int frameCounter) {
    string path = "E:\\Downloads\\dumps\\lapl2_" + to_string(configuration->getCompressionFactor()) + "\\";
    // write input frame to file
    imwrite(path + "s\\" + "s" + to_string(frameCounter) + ".jpg", inputFrame);
    // write mean to file
    imwrite(path + "m\\" + "m" + to_string(frameCounter) + ".jpg", mean);
    // write result to file
    imwrite(path + "r\\" + "r" + to_string(frameCounter) + ".jpg", outputImage);
}