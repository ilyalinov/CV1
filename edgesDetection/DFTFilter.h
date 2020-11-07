#pragma once
#include "EdgesDetector.h"

class DFTFilter : public EdgesDetector {
	public:	
		DFTFilter() { filterSize = 60; };

		DFTFilter(int filterSize) : filterSize(filterSize) {};

		virtual void detect(cv::Mat& inputImage, cv::Mat& outputImage, int filterSize = 30);

	private:
		int filterSize = 30;

		void dftShift(cv::Mat& i, cv::Mat& outI);

		void calcMS(cv::Mat& i, cv::Mat& magI);

		void synthesizeFilter(cv::Mat& H, cv::Size size, int filterSize);

		void createComplexImage(const cv::Mat& i, cv::Mat& complexImage);
};

