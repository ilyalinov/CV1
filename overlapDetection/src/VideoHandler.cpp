#include "VideoHandler.h"

#include <iostream>
#include <fstream>
#include <map>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>     
#include <opencv2/videoio.hpp>  
#include <opencv2/video/tracking.hpp>

#include "Clustering.h"
#include "Timer.h"

using namespace std;
using namespace cv;

VideoHandler::VideoHandler(Configuration* c) {
    configuration = c;
    EdgesDetector::DetectorType t = configuration->getEdgesDetectorType();
    detector = c->getDetector();
    smoothing = c->getSmoothing();
    optFlow = new OpticalFlow();

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

// process video, calculate smoothed image, filtered smoothed frame and detect overlap 
void VideoHandler::processVideo() {
    Mat frame;
    Size outputSize = getOutputSize();
    Mat mask(outputSize, CV_8U, Scalar::all(0));
    Mat smoothedCV8U;
    Mat filteredImage;
    Mat clusters;
    Mat overlap;
    int frameCounter = 0;
    Timer t1;
    std::vector<Mat> masks;
    Mat gray, prevGray, prevLabelImage;
    Mat stats, centroids, labelImage;
    int nLabels = 0;

    while (true) {
        frameCounter++;
        cout << "frameCounter = " << frameCounter << "\n";
        
        // read input frame
        newFrameCap >> frame;

        if (frame.empty()) {
            break;
        }
        
        Timer t1;
        t1.saveTimePoint();
 
        resize(frame, frame, outputSize);        
        cv::cvtColor(frame, gray, COLOR_BGR2GRAY);

        // calculate smoothed image 
        smoothing->work(gray, smoothedCV8U, frameCounter, configuration->getFramesLimit());

        if (frameCounter % configuration->getFramesLimit() == 0) {
            // apply laplacian or DFT high pass filter
            detector->detect(smoothedCV8U, filteredImage);

            // find clusters
            nLabels = findClusters(filteredImage, labelImage, centroids, stats);
            masks.push_back(labelImage.clone());

            // draw colored connected components image
            std::vector<Vec3b> colors(nLabels);
            createColors(colors, nLabels);
            clusters = Mat(outputSize, CV_8UC3, Scalar::all(0));
            drawComponents(clusters, labelImage, colors);
            imwrite("c" + to_string(frameCounter) + ".png", clusters);
        }
        
        // OPTICAL FLOW
        if (frameCounter > configuration->getFramesLimit()) {
            optFlow->setCurrentFrame(frame);
            optFlow->track(prevGray, gray);
            optFlow->printNumberOfSuccessfulPoints();
            optFlow->showTrackedPoints();
        }

        if (frameCounter == configuration->getFramesLimit()) {
            optFlow->initialize(labelImage, stats, nLabels, true);
        }
        else if (frameCounter % configuration->getFramesLimit() == 0) {
            vector<int> goodLabels = optFlow->getResult();

            overlap = Mat(outputSize, CV_8U, Scalar::all(0));
            for (int i = 0; i < goodLabels.size(); i++) {
                overlap += prevLabelImage == goodLabels[i];
                threshold(overlap, overlap, 0, 1, THRESH_BINARY);
            }
            
            threshold(overlap, overlap, 0, 255, THRESH_BINARY);
            imwrite("o" + to_string(frameCounter) + ".png", overlap);

            optFlow->reset();
            optFlow->initialize(labelImage, stats, nLabels, true);
        }
        else if (frameCounter > configuration->getFramesLimit() && frameCounter % configuration->getFramesLimit() != 0 && frameCounter % 40 == 0) {
            optFlow->reset();
            optFlow->initialize(labelImage, stats, nLabels, false);
        }

        cv::swap(prevGray, gray);
        labelImage.copyTo(prevLabelImage);

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
            calculateMeanStandardDeviationMedian(smoothedCV8U, filteredImage);
        }

        if (configuration->hasVideoRecording()) {
            v << filteredImage;
        }

        if (configuration->hasSaveResultsFlag()) {
            saveResults(frame, smoothedCV8U, filteredImage, frameCounter);
        }

        if (configuration->hasShowResultsFlag()) {
            showResults(frame, smoothedCV8U, filteredImage, clusters, overlap);
        }

        t1.reset();
    }

    destroyAllWindows();
}

VideoHandler::~VideoHandler() {
    newFrameCap.release();
    delete this->optFlow;
}

void VideoHandler::initializeVideoWriter() {
    const int sizeFactor = configuration->getSizeFactor();
    Size outputSize = getOutputSize();
    //int ex = (int)(cap.get(CAP_PROP_FOURCC));
    int fps = (int)(newFrameCap.get(CAP_PROP_FPS));
    string videoName = "E:\\Downloads\\dumps\\xxxxx_" + to_string(sizeFactor) + ".avi";
    int codec = VideoWriter::fourcc('M', 'J', 'P', 'G');
    v.open(videoName, codec, fps, outputSize, false);
}

cv::Size VideoHandler::getOutputSize() {
    const int sizeFactor = configuration->getSizeFactor();
    Size outputSize(160 * sizeFactor, 90 * sizeFactor);
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

void VideoHandler::calculateMeanStandardDeviationMedian(const cv::Mat& smoothed, const cv::Mat& outputImage) {
    //ofstream f1("med_mean.txt", ios::app);
    //ofstream f3("med_filter.txt", ios::app);
    ofstream f2("disp_mean.txt", ios::app);
    ofstream f4("disp_filter.txt", ios::app);
    ofstream f5("mean_filter.txt", ios::app);
    Mat m, disp;
    
    meanStdDev(smoothed, m, disp);
    // for variance instead of standard deviation use:
    //disp *= disp;
    int med = -1;
    std::vector<int> array;
    array.assign(smoothed.begin<uchar>(), smoothed.end<uchar>());
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

void VideoHandler::showResults(const cv::Mat& inputFrame
                                , const cv::Mat& smoothed
                                , const cv::Mat& filteredImage
                                , const cv::Mat& clusters
                                , const cv::Mat& overlap) {
    if (!smoothed.empty()) {
        imshow("Smoothed frame", smoothed);
    }
    if (!filteredImage.empty()) {
        imshow("Filtered smoothed image", filteredImage);
    }
    if (!inputFrame.empty()) {
        imshow("New frame", inputFrame);
    }
    if (!clusters.empty()) {
        imshow("Clusters", clusters);
    }
    if (!overlap.empty()) {
        imshow("Overlap", overlap);
    }
    cv::waitKey(1);
}

void VideoHandler::saveResults(const cv::Mat& inputFrame, const cv::Mat& smoothed, const cv::Mat& outputImage, int frameCounter) {
    string path = "E:\\Downloads\\dumps\\lapl2_" + to_string(configuration->getSizeFactor()) + "\\";
    // write input frame to file
    imwrite(path + "s\\" + "s" + to_string(frameCounter) + ".jpg", inputFrame);
    // write mean to file
    imwrite(path + "m\\" + "m" + to_string(frameCounter) + ".jpg", smoothed);
    // write result to file
    imwrite(path + "r\\" + "r" + to_string(frameCounter) + ".jpg", outputImage);
}

Point2f VideoHandler::getPoint(int row, int col, const Mat& labelImage, int label, int ksize) {
    Point2f center((float)(col + (ksize / 2)), (float)(row + ksize / 2));
    Point2f best(-1, -1);
    int d = INT_MAX;
    for (int r = row; (r < (row + ksize)) && (r < labelImage.rows); r++) {
        for (int c = col; (c < (col + ksize)) && (c < labelImage.cols); c++) {
            if (labelImage.at<int>(r, c) == label) {
                if (d >= (c - (int)center.x) * (c - (int)center.x) + (r - (int)center.y) * (r - (int)center.x)) {
                    best = Point2f((float)c, (float)r);
                    d = (c - (int)center.x) * (c - (int)center.x) + (r - (int)center.y) * (r - (int)center.y);
                }
                if (d == 0) {
                    break;
                }
            }
        }

        if (d == 0) {
            break;
        }
    }

    return best;
}