#pragma once
#include "VideoPlayer.h"
#include "EdgesDetector.h"
#include "DFTFilter.h"
#include "Laplacian.h"


using namespace std;

int main() {
    string inputName = "E:\\Downloads\\dumps\\resized_short1.avi";
    EdgesDetector* dft = new DFTFilter();
    EdgesDetector* lapl = new Laplacian();
    //VideoPlayer v(inputName, dft);
    VideoPlayer v(inputName, lapl);
    v.playVideo();
    delete dft;
    delete lapl;
    return 0;
}