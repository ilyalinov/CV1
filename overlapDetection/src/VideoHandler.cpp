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

#include "EdgesDetector.h"
#include "Timer.h"

using namespace std;
using namespace cv;

VideoHandler::VideoHandler(Configuration* c, EdgesDetectorCreator* e, SmoothingCreator* s) {
    configuration = c;
    EdgesDetector::DetectorType t = configuration->getEdgesDetectorType();
    detector = e->create();
    smoothing = s->create();

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
    Mat mask(outputSize, CV_8U, Scalar::all(0));
    Mat smoothedCV8U;
    Mat filteredImage;
    int frameCounter = 0;
    Timer t1;
    std::vector<Mat> masks;
    Mat prevGray;
    Mat gray;
    vector<Point2f> points[2];
    map<int, vector<Point2f>> pointsMap;
    map<int, vector<uchar>> statusMap;
    vector<int> labels;
    vector<int> positions;
    Mat stats, centroids, labelImage;
    int nnLabels = 0;
    std::map<int, int> componentShiftdx;
    std::map<int, int> componentShiftdy;
    vector<int> dy;
    vector<int> dx;
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

        // change image size 
        resize(frame, frame, outputSize);
        
        // calculate mean frame as CV_8U and CV_32F Mat types
        //calculateMean(mean32F, meanCV8U, frame, frameCounter);
        smoothing->work(frame, smoothedCV8U, frameCounter, configuration->getFramesLimit());

        // apply laplacian or DFT high pass filter
        detector->detect(smoothedCV8U, filteredImage);

        // find clusters
        Mat i = filteredImage;
        Mat i1;
        GaussianBlur(i, i1, Size(9, 9), 0);
        morphologyEx(i1, i1, MORPH_CLOSE, Mat::ones(15, 15, CV_8U));
        threshold(i1, i1, 40, 255, THRESH_BINARY);
        //int nLabels = connectedComponentsWithStats(i1, labelImage, stats, centroids, 8, CV_32S);
        
        if (frameCounter % configuration->getFramesLimit() == 0) {
            int nLabels = connectedComponentsWithStats(i1, labelImage, stats, centroids, 8, CV_32S);
            nnLabels = nLabels;

            // fill colors vector to draw components
            std::vector<Vec3b> colors(nLabels);
            colors[0] = Vec3b(0, 0, 0);
            std::vector<int> trueLabels;
            for (int label1 = 1; label1 < nLabels; ++label1) {
                colors[label1] = Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
            }

            // find masks overlap
            masks.push_back(labelImage);
            Mat dst1(i1.size(), CV_8UC3);
            draw_components(i1, dst1, labelImage, colors);
            Mat tmp = (labelImage != 0);
            threshold(tmp, tmp, 0, 1, THRESH_BINARY);
            mask = mask + tmp;
            Mat cmp(outputSize, CV_8U, Scalar::all((int)(masks.size() / 2)));
            cmp = mask > cmp;
            //imshow("mask", cmp);
            //imwrite("mask" + to_string(frameCounter) + ".jpg", cmp);
        }

        // draw colored connected components image
        //Mat dst(i1.size(), CV_8UC3);
        //draw_components(i1, dst, labelImage, colors);
        //imshow("components", dst);
        

        // OPTICAL FLOW
        cv::cvtColor(frame, gray, COLOR_BGR2GRAY);
        Mat image;
        frame.copyTo(image);

        t1.saveTimePoint();

        if (frameCounter >= configuration->getFramesLimit() && frameCounter % 10 == 0) {
            pointsMap.clear();
            statusMap.clear();
            positions.clear();
            labels.clear();
            points[0].clear();
            componentShiftdx.clear();
            componentShiftdy.clear();
            int counter = 0;
            Mat src = (labelImage != 0);
            Mat pointsImage(labelImage.size(), CV_8U, Scalar::all(0));

            for (int label1 = 1; label1 < nnLabels; label1++) {
                int width = stats.at<int>(label1, CC_STAT_WIDTH);
                int height = stats.at<int>(label1, CC_STAT_HEIGHT);
                Point topLeft(stats.at<int>(label1, CC_STAT_LEFT), stats.at<int>(label1, CC_STAT_TOP));
                int ksize = (stats.at<int>(label1, CC_STAT_AREA) > 500) ? 7 : 5;
                pointsMap.insert(pair<int, vector<Point2f>>(label1, vector<Point2f>()));
                statusMap.insert(pair<int, vector<uchar>>(label1, vector<uchar>()));
                int pos = 0;

                for (int r = topLeft.y; r <= topLeft.y + height; r += ksize) {
                    for (int c = topLeft.x; c <= topLeft.x + width; c += ksize) {
                        //cout << stats.at<int>(label, CC_STAT_AREA) << endl;
                        Point2f p = getPoint(r, c, labelImage, label1, ksize);
                        if (p != Point2f(-1, -1)) {
                            pointsImage.at<uchar>(p) = 255;
                            points[0].push_back(p);
                            positions.push_back(pos++);
                            labels.push_back(label1);
                            pointsMap[label1].push_back(p);
                            statusMap[label1].push_back(1);
                            counter++;
                        }
                    }
                }

                cout << "comp = " << label1 << "; " << " number of points = " << pointsMap[label1].size() << "\n";
            }

            //cout << "number of points for tracking: " << counter << "\n";
            imshow("points for tracking", pointsImage);
            cv::waitKey(0);
        }


        if (frameCounter > configuration->getFramesLimit() && (frameCounter % 10) != 0) {
            vector<uchar> status;
            vector<float> err;
            Size winSize(5, 5);
            TermCriteria termcrit(TermCriteria::COUNT | TermCriteria::EPS, 20, 0.03);
            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize, 2, termcrit, 0, 0.001);

            int j = 0;
            int k = 0;
            int label = 1;
            //vector<int> dy;
            //vector<int> dx;
            // update points[0], positions, labels
            for (j = k = 0; j < points[0].size(); j++) {
                if (!dx.empty() && (j == points[0].size() - 1 || labels[j + 1] != label)) {
                    std::map<int, int> countdy;
                    std::map<int, int> countdx;
                    for (int t = 0; t < dy.size(); t++) {
                        ++countdx[dx[t]];
                        ++countdy[dy[t]];
                    }

                    int occurencesdy = 0;
                    int mostCommondy = 0;
                    int occurencesdx = 0;
                    int mostCommondx = 0;
                    for (auto it = countdy.cbegin(); it != countdy.cend(); ++it) {
                        if (it->second > occurencesdy) {
                            mostCommondy = it->first;
                            occurencesdy = it->second;
                        }
                    }
                    for (auto it = countdx.cbegin(); it != countdx.cend(); ++it) {
                        if (it->second > occurencesdx) {
                            mostCommondx = it->first;
                            occurencesdx = it->second;
                        }
                    }
                    
                    componentShiftdy[label] += (mostCommondy);
                    componentShiftdx[label] += (mostCommondx);
                    cout << "comp number: " << label << "; full shift dy: " << componentShiftdy[label] << "\n";
                    cout << "comp number: " << label << "; full shift dx: " << componentShiftdx[label] << "\n";

                    label++;
                    dx.clear();
                    dy.clear();
                }

                if (!status[j]) {
                    statusMap[labels[j]][positions[j]] = 0;
                    continue;
                }

                dy.push_back((int)round(points[1][j].y - points[0][j].y));
                dx.push_back((int)round(points[1][j].x - points[0][j].x));

                points[1][k] = points[1][j];
                labels[k] = labels[j];
                positions[k] = positions[j];
                k++;
                circle(image, points[1][j], 1, Scalar(0, 255, 0), -1, 8);
            }

            // log how many points are successfully tracked for each component
            for (j = 1; j < nnLabels; j++) {
                long long count = std::count(statusMap[j].begin(), statusMap[j].end(), 1);
                cout << "comp number = " << j << "; successful points number = " << count << "\n";
            }

            points[1].resize(k);
            positions.resize(k);
            labels.resize(k);
            std::swap(points[1], points[0]);

            // draw successfully tracked points
            if ((frameCounter - configuration->getFramesLimit()) % 30 > -1) {
                imshow("goodPoints", image);
                cv::waitKey(0);
            }
        }

        cv::swap(prevGray, gray);

        t1.saveTimePoint();
        t1.printFullDuration();


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
            calculateMeanStandardDeviationMedian(smoothedCV8U, filteredImage);
        }

        if (configuration->hasVideoRecording()) {
            v << filteredImage;
        }

        if (configuration->hasSaveResultsFlag()) {
            saveResults(frame, smoothedCV8U, filteredImage, frameCounter);
        }

        if (configuration->hasShowResultsFlag()) {
            showResults(frame, smoothedCV8U, filteredImage);
        }

        t1.reset();
    }

    destroyAllWindows();
}

VideoHandler::~VideoHandler() {
    newFrameCap.release();
    delete this->detector;
    delete this->smoothing;
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

void VideoHandler::showResults(const cv::Mat& inputFrame, const cv::Mat& smoothed, const cv::Mat& filteredImage) {
    imshow("Smoothed frame", smoothed);
    imshow("Filtered mean frame", filteredImage);
    imshow("New frame", inputFrame);
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