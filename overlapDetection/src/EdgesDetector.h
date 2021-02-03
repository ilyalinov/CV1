#pragma once
#include "opencv2/core.hpp"

class EdgesDetector {
	public:
		virtual void detect(cv::Mat& inputImage, cv::Mat& outputImage) = 0;

		virtual ~EdgesDetector() {};
};