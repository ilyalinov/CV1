#include <iostream>

#include <opencv2/core/ocl.hpp>

#include "VideoHandler.h"
#include "Configuration.h"

int main(int argc, char** argv) {
    cv::ocl::setUseOpenCL(false);
    Configuration c(argv[1]);
    VideoHandler v(&c);
    v.processVideo();
    return 0;
}