#pragma once
#include "opencv2/core.hpp"

class EdgesDetector {
	public:
		virtual void detect(cv::Mat& inputImage, cv::Mat& outputImage, int filterSize = 30) = 0;

		~EdgesDetector() {};
};