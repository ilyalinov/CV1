#include <iostream>

#include "VideoPlayer.h"
#include "EdgesDetector.h"
#include "DFTFilter.h"
#include "Laplacian.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: overlapDetection <Video_Path>" << "\n";
        return -1;
    }

    string inputName = argv[1];
    EdgesDetector* dft = new DFTFilter();
    EdgesDetector* lapl = new Laplacian();
    //VideoPlayer v(inputName, dft);
    VideoPlayer v(inputName, lapl);
    v.playVideo();
    delete dft;
    delete lapl;
    return 0;
}