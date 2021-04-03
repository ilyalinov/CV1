#pragma once

#include <opencv2/core.hpp>

int findClusters(const cv::Mat& inputImage, cv::Mat& labelImage, cv::Mat& centroids, cv::Mat& stats);

void createColors(std::vector<cv::Vec3b>& v, int labelsNumber);

void drawComponents(cv::Mat& dst, const cv::Mat& labelImage, std::vector<cv::Vec3b>& colors);

void findMasksOverlap(cv::Mat& mask, const cv::Mat& labelImage, const std::vector<cv::Mat>& masks, cv::Mat& result);