#include <iostream>

#include "VideoHandler.h"
#include "Configuration.h"

int main(int argc, char** argv) {
    Configuration c(argv[1]);
    VideoHandler v(&c);
    v.processVideo();
    return 0;
}