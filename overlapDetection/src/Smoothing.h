#pragma once
#include "opencv2/core.hpp"

class Smoothing {
public:
	virtual void work(cv::Mat& inputImage, cv::Mat& outputImage, int framesCounter, int framesLimit) = 0;

	virtual ~Smoothing() {};

	enum class SmoothingType {
		exp,
		mean
	};
};