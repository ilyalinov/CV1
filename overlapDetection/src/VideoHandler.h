#pragma once
#include <opencv2/videoio.hpp>

#include "EdgesDetectorCreator.h"
#include "Configuration.h"
#include "SmoothingCreator.h"
#include "EdgesDetector.h"
#include "Smoothing.h"
#include "OpticalFlow.h"

class VideoHandler {
	public:
		VideoHandler(Configuration* c);

		void processVideo();

		~VideoHandler();

	private:
		void initializeVideoWriter();

		cv::Size getOutputSize();

		void calculateMean(cv::Mat& mean, cv::Mat& meanCV8U, const cv::Mat& inputFrame, int frameCounter);

		void calculateMeanStandardDeviationMedian(const cv::Mat& mean, const cv::Mat& outputImage);
		
		void saveResults(const cv::Mat& inputFrame, const cv::Mat& mean, const cv::Mat& outputImage, int frameCounter);
		
		void showResults(const cv::Mat& inputFrame
						, const cv::Mat& mean
						, const cv::Mat& outputImage
						, const cv::Mat& clusters
						, const cv::Mat& overlap
						, const cv::Mat& result);

		void draw_components(cv::Mat& src, cv::Mat& dst, cv::Mat& labelImage, std::vector<cv::Vec3b>& colors);

		cv::Point2f getPoint(int row, int col, const cv::Mat& labelImage, int label, int ksize);

		Configuration* configuration;
		EdgesDetector* detector;
		Smoothing* smoothing;
		OpticalFlow* optFlow;
		cv::VideoWriter v;
		cv::VideoCapture newFrameCap;
};