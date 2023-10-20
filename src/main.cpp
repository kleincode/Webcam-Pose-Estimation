#include <tensorflow/lite/model.h>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/optional_debug_tools.h>
#include <tensorflow/lite/string_util.h>

#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
using namespace std;

int main() {
	cv::Mat img;
	cv::namedWindow("Webcam");
	cv::VideoCapture cap(0);

	if (!cap.isOpened()) {
		cout << "No video stream detected" << endl;
		system("pause");
		return -1;
	}

	while (true) {
		cap >> img;
		if (img.empty()) {
			break;
		}
		cv::imshow("Webcam", img);
		char c = (char)cv::waitKey(25);
		if (c == 27) {
			// ESC pressed
			break;
		}
	}

	cap.release();
	return 0;
}