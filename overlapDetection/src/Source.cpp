#include <iostream>

#include <opencv2/core/ocl.hpp>

#include "VideoHandler.h"
#include "Configuration.h"

int main(int argc, char** argv) {
    cv::ocl::setUseOpenCL(false);
    Configuration c(argv[1]);
    VideoHandler v(&c);
    VideoHandler v1(&c);
    VideoHandler v2(&c);
    v.processVideo();
    v1.processVideo();
    v2.processVideo();
    return 0;
}