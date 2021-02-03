#pragma once
#include "EdgesDetector.h"

class Laplacian : public EdgesDetector {
	public:
		void detect(cv::Mat& inputImage, cv::Mat& outputImage);
};