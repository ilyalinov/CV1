#include <iostream>
#include <string>

#include "VideoHandler.h"
#include "EdgesDetector.h"
#include "DFTFilter.h"
#include "Laplacian.h"
#include "Configuration.h"

int main(int argc, char** argv) {
    Configuration c("config.cfg");
    VideoHandler v(&c);
    v.processVideo();
    return 0;
}