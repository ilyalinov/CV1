#include "Laplacian.h"

#include <opencv2/imgproc.hpp>

void Laplacian::detect(cv::Mat& inputImage, cv::Mat& outputImage)
{
	cv::Mat dst;
	int ksize = 3;
	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;

	//cv::Mat grad_x, grad_y;
	//cv::Mat abs_grad_x, abs_grad_y;

	cv::GaussianBlur(inputImage, inputImage, cv::Size(ksize, ksize), 0, 0, cv::BORDER_DEFAULT);
	
	//cv::Sobel(dst1, grad_x, ddepth, 1, 0, ksize, scale, delta, cv::BORDER_DEFAULT);
	//cv::Sobel(dst1, grad_y, ddepth, 0, 1, ksize, scale, delta, cv::BORDER_DEFAULT);

	//convertScaleAbs(grad_x, abs_grad_x);
	//convertScaleAbs(grad_y, abs_grad_y);
	//
	//addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, outputImage);
	
	cv::Laplacian(inputImage, dst, ddepth, ksize, scale, delta, cv::BORDER_DEFAULT);
	convertScaleAbs(dst, outputImage);
}