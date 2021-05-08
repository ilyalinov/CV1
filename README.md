### Overlap detection app for ADAS

[![Build Status](https://travis-ci.com/ilyalinov/CV1.svg?branch=dev)](https://travis-ci.com/ilyalinov/CV1)

#### Prerequisites:
* GCC or Visual C++ compiler
* OpenCV
* CMake

#### How to build:
* `cd overlapDetection`
* `mkdir build && cd build`
* `cmake ..`
* `cmake --build .`

#### How to run:
* `overlapDetection <path_to_configuration_file>`

#### Configuration file example:
  ```ini
show.results=true
size.factor=2
frames.limit=200
path.to.video=С:\MySpecialDir\cityTraffic.mp4
detector=laplacian
video.recording=true
  ````
#### Parameters: 
* show.results -- show the proccess of overlap finding;
* size.factor -- input frame compression parameter;
  * set it to 2 for a more precise search;
* frames.limit -- the length of frame sequence for smoothing;
  * set it to 200 for a more precise search; 
* path.to.video -- path to input video or videostream;
* detector -- laplacian and dft (DFT high pass filter) are supported. DFT filter doesn't work yet;  
* video.recording -- set it to true if you want to record a video with found zones of overlap.
