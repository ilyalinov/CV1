#pragma once
#include "EdgesDetector.h"

class DFTFilter : public EdgesDetector {
	public:	
		DFTFilter() { filterSize_ = 32; };

		DFTFilter(int filterSize) : filterSize_(filterSize) {};

		virtual void detect(cv::Mat& inputImage, cv::Mat& outputImage);

	private:
		int filterSize_;

		void dftShift(cv::Mat& i, cv::Mat& outI);

		void calcMS(cv::Mat& i, cv::Mat& magI);

		void synthesizeFilter(cv::Mat& H, cv::Size size, int filterSize);

		void createComplexImage(const cv::Mat& i, cv::Mat& complexImage);
};