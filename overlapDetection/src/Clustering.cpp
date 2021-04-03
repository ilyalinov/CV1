#include "Clustering.h"

#include <vector>

#include <opencv2/imgproc.hpp>

using namespace cv;

int findClusters(const cv::Mat& inputImage, cv::Mat& labelImage, cv::Mat& centroids, cv::Mat& stats) {
    Mat i1;
    GaussianBlur(inputImage, i1, Size(9, 9), 0);
    morphologyEx(i1, i1, MORPH_CLOSE, Mat::ones(15, 15, CV_8U));
    threshold(i1, i1, 40, 255, THRESH_BINARY);
    int nLabels = connectedComponentsWithStats(i1, labelImage, stats, centroids, 8, CV_32S);
    return nLabels;
}

void createColors(std::vector<Vec3b>& colors, int labelsNumber) {
    colors[0] = Vec3b(0, 0, 0);
    std::vector<int> trueLabels;
    for (int label = 1; label < labelsNumber; label++) {
        colors[label] = Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
    }
}

void drawComponents(Mat& dst, const Mat& labelImage, std::vector<Vec3b>& colors) {
    for (int r = 0; r < dst.rows; ++r) {
        for (int c = 0; c < dst.cols; ++c) {
            int label = labelImage.at<int>(r, c);
            Vec3b& pixel = dst.at<Vec3b>(r, c);
            pixel = colors[label];
        }
    }
}

void findMasksOverlap(Mat& mask, const Mat& labelImage, const std::vector<Mat>& masks, Mat& result) {
    Mat allClusters = (labelImage != 0);
    threshold(allClusters, allClusters, 0, 1, THRESH_BINARY);
    mask = mask + allClusters;
    Mat temp(labelImage.size(), CV_8U, Scalar::all((int)(masks.size() / 2)));
    result = mask > temp;
}