#include "Laplacian.h"
#include <opencv2/imgproc.hpp>

void Laplacian::detect(cv::Mat& inputImage, cv::Mat& outputImage, int filterSize)
{
	cv::Mat dst;
	int kernel_size = 3;
	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;

	cv::Laplacian(inputImage, dst, ddepth, kernel_size, scale, delta, cv::BORDER_DEFAULT);
	convertScaleAbs(dst, outputImage);
}