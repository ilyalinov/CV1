#pragma once
#include "Smoothing.h"

class Mean : public Smoothing {
public:
	virtual void work(cv::Mat& inputImage, cv::Mat& outputImage, int framesCounter, int framesLimit) override;

private:
	cv::Mat mean32F;
};