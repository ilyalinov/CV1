#pragma once
#include <opencv2/videoio.hpp>

#include "EdgesDetectorCreator.h"
#include "Configuration.h"
#include "SmoothingCreator.h"
#include "EdgesDetector.h"
#include "Smoothing.h"

class VideoHandler {
	public:
		VideoHandler(Configuration* c, EdgesDetectorCreator* e, SmoothingCreator* s);

		void processVideo();

		~VideoHandler();

	private:
		void initializeVideoWriter();

		cv::Size getOutputSize();

		void calculateMean(cv::Mat& mean, cv::Mat& meanCV8U, const cv::Mat& inputFrame, int frameCounter);

		void calculateMeanStandardDeviationMedian(const cv::Mat& mean, const cv::Mat& outputImage);
		
		void saveResults(const cv::Mat& inputFrame, const cv::Mat& mean, const cv::Mat& outputImage, int frameCounter);
		
		void showResults(const cv::Mat& inputFrame, const cv::Mat& mean, const cv::Mat& outputImage);

		void draw_components(cv::Mat& src, cv::Mat& dst, cv::Mat& labelImage, std::vector<cv::Vec3b>& colors);

		Configuration* configuration;
		EdgesDetector* detector;
		Smoothing* smoothing;
		cv::VideoWriter v;
		cv::VideoCapture newFrameCap;
};