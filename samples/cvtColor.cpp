#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <chrono>

using namespace std;
using namespace cv;

int main() {
    Mat i = imread("E:\\Downloads\\dumps\\messi.jpg");
    Mat i1 = i.clone();
    long long duration1 = 0;
    long long duration2 = 0;

    thread t1 = std::thread([&]() {
        auto tt1 = std::chrono::high_resolution_clock::now();
        
        cvtColor(i, i, COLOR_BGR2GRAY);
        
        auto tt2 = std::chrono::high_resolution_clock::now();
        duration1 = std::chrono::duration_cast<std::chrono::microseconds>(tt2 - tt1).count();
    });
    
    thread t2 = std::thread([&]() {
        auto tt1 = std::chrono::high_resolution_clock::now();
        
        cvtColor(i1, i1, COLOR_BGR2GRAY);
        
        auto tt2 = std::chrono::high_resolution_clock::now();
        duration2 = std::chrono::duration_cast<std::chrono::microseconds>(tt2 - tt1).count();
    });

    t1.join();
    t2.join();

    std::cout << "1 thread: " << duration1 << '\n';
    std::cout << "2 thread: " << duration2 << '\n';

	return 0;
}