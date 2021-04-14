#pragma once

#include <vector>
#include <map>
#include <fstream>

#include <opencv2/core.hpp>

class OpticalFlow {
	public:
		void setCurrentFrame(const cv::Mat& frame);

		void initialize(const cv::Mat& labelImage, const cv::Mat& stats, int nLabels, bool isNewLabelImage);

		void track(const cv::Mat& prevGray, const cv::Mat& gray);

		void reset();

		const std::vector<int>& getResult() const;

		void printNumberOfSuccessfulPoints();

		void showTrackedPoints();

	private:
		void findComponentShift(int label);

		cv::Point2f getPoint(int row, int col, const cv::Mat& labelImage, int label, int ksize);

		std::vector<cv::Vec3b> colors;
		std::vector<int> goodLabels;
		std::vector<cv::Point2f> points[2];
		std::map<int, std::vector<cv::Point2f>> pointsMap;
		std::map<int, std::vector<uchar>> statusMap;
		std::vector<int> labels;
		std::vector<int> positions;    
		std::map<int, float> componentShiftdx;
		std::map<int, float> componentShiftdy;
		std::vector<int> dy;
		std::vector<int> dx;
		std::vector<int> result;
		cv::Mat image;
		int pointsCounter = 0;
		int nLabels = 0;
		int iteration = 0;
		std::vector<std::ofstream> files;
};