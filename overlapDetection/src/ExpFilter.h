#pragma once
#include "Smoothing.h"

class ExpFilter : public Smoothing {
public:
	virtual void work(cv::Mat& inputImage, cv::Mat& outputImage, int framesCounter, int framesLimit) override;

private:
	cv::Mat acc32F;
	double alpha = 0.008;
};