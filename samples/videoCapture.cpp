#include "opencv2/opencv.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
	VideoCapture cap(0);

	if (!cap.isOpened()) {
		cout << "error opening video" << endl;
		return -1;
	}

	while (true) {
		Mat frame;
		cap >> frame;
		if (frame.empty()) {
			break;
		}

		imshow("Video", frame);
		char c = (char)waitKey(25);
		if (c == 27) {
			break;
		}
	}

	cap.release();
	destroyAllWindows();
	return 0;
}