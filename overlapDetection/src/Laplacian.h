#pragma once
#include "EdgesDetector.h"

class Laplacian : public EdgesDetector {
	public:
		Laplacian() {};

		void detect(cv::Mat& inputImage, cv::Mat& outputImage);
};