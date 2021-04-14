#include "OpticalFlow.h"

#include <iostream>

#include <opencv2/video/tracking.hpp>
#include <opencv2/highgui.hpp>

#include "Clustering.h"

using namespace cv;
using namespace std;

void OpticalFlow::setCurrentFrame(const cv::Mat& frame) {
    frame.copyTo(image);
}

void OpticalFlow::initialize(const Mat& labelImage, const Mat& stats, int nLabels, bool isNewLabelImage) {
    if (isNewLabelImage) {
        iteration++;
        goodLabels.clear();

        for (int i = 0; i < files.size(); i++) {
            files[i].close();
        }
        files.clear();
        
        for (int label = 1; label < nLabels; label++) {
            goodLabels.push_back(label);
        }

        componentShiftdx.clear();
        componentShiftdy.clear();
    }

    colors.resize(nLabels);
    createColors(colors, nLabels);

    Mat src = (labelImage != 0);
    Mat pointsImage(labelImage.size(), CV_8U, Scalar::all(0));
    this->nLabels = nLabels;

    for (int label = 1; label < nLabels; label++) {
        int width = stats.at<int>(label, CC_STAT_WIDTH);
        int height = stats.at<int>(label, CC_STAT_HEIGHT);
        Point topLeft(stats.at<int>(label, CC_STAT_LEFT), stats.at<int>(label, CC_STAT_TOP));
        int ksize = (stats.at<int>(label, CC_STAT_AREA) > 500) ? 7 : 5;
        pointsMap.insert(pair<int, vector<Point2f>>(label, vector<Point2f>()));
        statusMap.insert(pair<int, vector<uchar>>(label, vector<uchar>()));
        int pos = 0;
        pointsCounter = 0;

        for (int r = topLeft.y; r <= topLeft.y + height; r += ksize) {
            for (int c = topLeft.x; c <= topLeft.x + width; c += ksize) {
                //cout << stats.at<int>(label, CC_STAT_AREA) << endl;
                Point2f p = getPoint(r, c, labelImage, label, ksize);
                if (p != Point2f(-1, -1)) {
                    pointsImage.at<uchar>(p) = 255;
                    points[0].push_back(p);
                    positions.push_back(pos++);
                    labels.push_back(label);
                    pointsMap[label].push_back(p);
                    statusMap[label].push_back(1);
                    pointsCounter++;
                }
            }
        }

        //if (isNewLabelImage) {
        //    string filename("i" + to_string(iteration) + "_" 
        //        + "c" + to_string(label) + "_"
        //        + "p" + to_string(statusMap[label].size())
        //        + ".txt"
        //    );

        //    files.emplace_back(filename);
        //}
        
        //cout << "comp = " << label << "; " << " number of points = " << pointsMap[label].size() << "\n";
    }

    //cout << "number of points for tracking: " << pointsCounter << "\n";
    //imshow("points for tracking", pointsImage);
    //cv::waitKey(0);
}

void OpticalFlow::track(const Mat& prevGray, const Mat& gray) {
    vector<uchar> status;
    vector<float> err;
    Size winSize(11, 11);
    TermCriteria termcrit(TermCriteria::COUNT | TermCriteria::EPS, 30, 0.03);
    calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize, 2, termcrit, 0, 0.001);

    int j = 0;
    int k = 0;
    int label = 1;

    // update points[0], positions, labels; find components shift
    for (j = k = 0; j < points[0].size(); j++) {
        if (!dx.empty() && (j == points[0].size() - 1 || labels[j + 1] != label)) {
            if (!status[j]) {
                statusMap[labels[j]][positions[j]] = 0;
            }

            long long numberOfGoodPoints = std::count(statusMap[labels[j]].begin(), statusMap[labels[j]].end(), 1);
            bool goodComponent = std::find(goodLabels.begin(), goodLabels.end(), label) != goodLabels.end();
            if (goodComponent && numberOfGoodPoints > 0) {
                findComponentShift(label);
            }
            else {
                goodLabels.erase(std::remove(goodLabels.begin(), goodLabels.end(), label), goodLabels.end());
            }

            label++;
        }

        if (!status[j]) {
            statusMap[labels[j]][positions[j]] = 0;
            continue;
        }

        dy.push_back(((int)round((points[1][j].y - points[0][j].y) * 10)));
        dx.push_back(((int)round((points[1][j].x - points[0][j].x) * 10)));

        points[1][k] = points[1][j];
        labels[k] = labels[j];
        positions[k] = positions[j];
        k++;
        circle(image, points[1][j], 2, colors[labels[j]], -1, 8);
    }

    points[1].resize(k);
    positions.resize(k);
    labels.resize(k);
    std::swap(points[1], points[0]);
}

void OpticalFlow::reset() {
    pointsMap.clear();
    statusMap.clear();
    positions.clear();
    labels.clear();
    points[0].clear();
    //componentShiftdx.clear();
    //componentShiftdy.clear();
    int pointsCounter = 0;
}

const std::vector<int>& OpticalFlow::getResult() const {
    return goodLabels;
}

void OpticalFlow::printNumberOfSuccessfulPoints() {
    // log how many points are successfully tracked for each component
    for (int j = 1; j < nLabels; j++) {
        long long result = std::count(statusMap[j].begin(), statusMap[j].end(), 1);
        cout << "comp number = " << j << "; successful points number = " << result << "\n";
    }
}

void OpticalFlow::showTrackedPoints() {
    // draw successfully tracked points
    imshow("points", image);
    //imwrite("E:\\Downloads\\dumps\\points\\" + to_string(frameCounter) + ".jpg", image);
    cv::waitKey(0);
}

void OpticalFlow::findComponentShift(int label) {
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

    const float eps = 3.0;
    componentShiftdy[label] += (mostCommondy) / 10.0;
    componentShiftdx[label] += (mostCommondx) / 10.0;
    if (abs(componentShiftdy[label]) > eps || abs(componentShiftdx[label]) > eps + 3) {
        goodLabels.erase(std::remove(goodLabels.begin(), goodLabels.end(), label), goodLabels.end());
    }

    //cout << "comp number: " << label << "; shift dy: " << mostCommondy << "\n";
    //cout << "comp number: " << label << "; shift dx: " << mostCommondx << "\n";

    cout << "comp number: " << label << "; full shift dy: " << componentShiftdy[label] << "\n";
    cout << "comp number: " << label << "; full shift dx: " << componentShiftdx[label] << "\n";

    //files[label - 1] << componentShiftdy[label] << "\n";

    dx.clear();
    dy.clear();
}

Point2f OpticalFlow::getPoint(int row, int col, const Mat& labelImage, int label, int ksize) {
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