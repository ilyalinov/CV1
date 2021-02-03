#include "DFTFilter.h"

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

using namespace cv; 

// remove low frequencies from the image
void DFTFilter::detect(Mat& i, Mat& outputImg) {
    Rect roi = Rect(0, 0, i.cols & -2, i.rows & -2);
    i = i(roi);
    Mat magI;

    Mat H;
    synthesizeFilter(H, i.size(), filterSize_);

    Mat complexI, complexH, complexIH;
    createComplexImage(i, complexI);
    dft(complexI, complexI, DFT_SCALE);
    createComplexImage(H, complexH);
    mulSpectrums(complexI, complexH, complexIH, 0);

    idft(complexIH, complexIH);
    Mat planes[2] = { Mat_<float>(i.clone()), Mat::zeros(i.size(), CV_32F) };
    split(complexIH, planes);
    outputImg = planes[0];
    outputImg.convertTo(outputImg, CV_8U);
    normalize(outputImg, outputImg, 0, 255, NORM_MINMAX);
}

// rearrange the quadrants of Fourier image  so that the origin is at the image center
void DFTFilter::dftShift(Mat& i, Mat& outI) {
    outI = i.clone();
    int cx = outI.cols / 2;
    int cy = outI.rows / 2;

    // Top-Left - Create a ROI per quadrant
    Mat q0(outI, Rect(0, 0, cx, cy));
    // Top-Right
    Mat q1(outI, Rect(cx, 0, cx, cy));
    // Bottom-Left
    Mat q2(outI, Rect(0, cy, cx, cy));
    // Bottom-Right
    Mat q3(outI, Rect(cx, cy, cx, cy));

    // swap quadrants (Top-Left with Bottom-Right)
    Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    // swap quadrant (Top-Right with Bottom-Left)
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
}

// calculate magnitude spectrum
void DFTFilter::calcMS(Mat& i, Mat& magI) {
    Rect roi = Rect(0, 0, i.cols & -2, i.rows & -2);
    i = i(roi);
    Mat planes[] = { Mat_<float>(i), Mat::zeros(i.size(), CV_32F) };
    Mat complexI;

    // add to the expanded another plane with zeros
    merge(planes, 2, complexI);
    dft(complexI, complexI);
    split(complexI, planes);

    // planes[0] = magnitude
    magnitude(planes[0], planes[1], planes[0]);
    magI = planes[0];

    // switch to logarithmic scale
    magI += Scalar::all(1);
    log(magI, magI);

    // crop the spectrum, if it has an odd number of rows or columns
    magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

    dftShift(magI, magI);
    normalize(magI, magI, 0, 1, NORM_MINMAX);
}

// create dft high pass filter
void DFTFilter::synthesizeFilter(Mat& H, Size size, int filterSize) {
    H = Mat(size, CV_32F, Scalar(1));
    //H(Rect(H.cols / 2 - (filterSize / 2), H.rows / 2 - (filterSize / 2), filterSize, filterSize)) = 0;
    circle(H, Point(H.cols / 2, H.rows / 2), filterSize / 2, Scalar(0), FILLED);
    dftShift(H, H);
}

void DFTFilter::createComplexImage(const Mat& i, Mat& complexImage) {
    Mat planes[2] = { Mat_<float>(i.clone()), Mat::zeros(i.size(), CV_32F) };
    merge(planes, 2, complexImage);
}