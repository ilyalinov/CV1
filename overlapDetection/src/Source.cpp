#include <iostream>
#include <string>

#include "VideoHandler.h"
#include "EdgesDetector.h"
#include "Configuration.h"
#include "EdgesDetectorCreator.h"
#include "SmoothingCreator.h"

int main(int argc, char** argv) {
    Configuration c("config.cfg");
    EdgesDetectorCreator* e = nullptr; 
    LaplacianCreator l;
    e = &l;
    SmoothingCreator* s = nullptr;
    MeanCreator m;
    ExpFilterCreator m1;
    s = &m;
    VideoHandler v(&c, e, s);
    v.processVideo();
    return 0;
}