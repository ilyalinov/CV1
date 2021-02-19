#pragma once
#include <opencv2/videoio.hpp>

#include "EdgesDetector.h"

class VideoPlayer {
	public:
		VideoPlayer(std::string& inputName, EdgesDetector* e) : pathToVideo(inputName), detector(e) {};

		void processVideo();

	private:
		void initializeVideoWriter(cv::VideoWriter& v, cv::VideoCapture& cap, int compressionFactor);

		cv::Size getOutputSize(cv::VideoCapture& cap, int compressionFactor);

		void calculateMean(cv::Mat& mean, cv::Mat& meanCV8U, const cv::Mat& inputFrame, int frameCounter);

		void calculateMeanStandardDeviationMedian(const cv::Mat& mean, const cv::Mat& outputImage);
		
		void saveResults(const cv::Mat& inputFrame, const cv::Mat& mean, const cv::Mat& outputImage, int frameCounter);
		
		void showResults(const cv::Mat& inputFrame, const cv::Mat& mean, const cv::Mat& outputImage);

		void draw_components(cv::Mat& src, cv::Mat& dst, cv::Mat& labelImage, std::vector<cv::Vec3b> colors);

		const bool videoRecordingFlag = false;
		const bool meanStandardDeviationMedianRecording = false;
		const bool saveResultsFlag = false;
		const bool showResultsFlag = true;
		const int compressionFactor = 2;
		const int framesLimit = 500;
		std::string pathToVideo;
		EdgesDetector* detector;
};