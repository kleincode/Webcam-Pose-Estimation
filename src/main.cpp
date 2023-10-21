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

cv::Mat resizeFrame(cv::Mat &frame, cv::Mat &resized, int targetWidth, int targetHeight) {
	/* 
		Resizes the image to targetWidth x targetHeight, such that it keeps its aspect ratio.
		The resized image is saved in resized.
		The returned Mat is a cropped version of frame, with the same aspect ratio of resized
		but the original resolution of frame.
	*/

	int height = frame.size().height;
	int width = frame.size().width;
	double xScale = targetWidth / (double)width;
	double yScale = targetHeight / (double)height;

	cv::Mat cropped;

	if (yScale > xScale) {
		// targetWidth / targetHeight = sourceWidth / height (keep aspect ratio)
		int sourceWidth = cvCeil(targetWidth / yScale);
		// pad image left and right before resize to keep aspect ratio
		int sourcePad = (width - sourceWidth) / 2;

		cropped = frame(cv::Rect(sourcePad, 0, sourceWidth, height));
		cv::resize(cropped, resized, cv::Size(targetWidth, targetHeight));
	}
	else {
		// targetWidth / targetHeight = width / sourceHeight (keep aspect ratio)
		int sourceHeight = cvCeil(targetHeight / xScale);
		// pad image top and bottom before resize to keep aspect ratio
		int sourcePad = (height - sourceHeight) / 2;

		cropped = frame(cv::Rect(0, sourcePad, width, sourceHeight));
		cv::resize(cropped, resized, cv::Size(targetWidth, targetHeight));
	}
	return cropped;
}

float* runInterpreter(unique_ptr<tflite::Interpreter> &interpreter, cv::Mat &input) {
	memcpy(interpreter->typed_input_tensor<unsigned char>(0), input.data, input.total() * input.elemSize());

	// inference
	chrono::steady_clock::time_point start, end;
	start = chrono::steady_clock::now();
	if (interpreter->Invoke() != kTfLiteOk) {
		cerr << "Inference failed" << endl;
	}
	end = chrono::steady_clock::now();
	auto processing_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

	cout << "processing time: " << processing_time << " ms" << endl;

	float* results = interpreter->typed_output_tensor<float>(0);
	return results;
}

const float poseThreshold = 0.2f, keypointThreshold = 0.2f;
const auto red = cv::Scalar(50, 50, 255);
const std::vector<std::pair<int, int>> connections = {
	{0, 1}, {0, 2}, {1, 3}, {2, 4}, {5, 6}, {5, 7}, {7, 9}, {6, 8}, {8, 10}, {5, 11}, {6, 12}, {11, 12}, {11, 13}, {13, 15}, {12, 14}, {14, 16} };

void drawKeypoints(cv::Mat& target, float* output, int poses, double frameMs) {

	int width = target.size().width;
	int height = target.size().height;

	// if poses == 1:
	//	Output is expected to be [1, 1, 17, 3]
	// if poses > 1:
	//	Output is expected to be [1, poses, 56]
	//	Each pose is described by 17*3 keypoint locations + scores [y, x, s]
	//	Elements 51..55 are [ymin, xmin, ymax, xmax, score]

	for (int p = 0; p < poses; p++) {
		float* pose = output + (56 * p);

		if (poses > 1) {
			float ymin = pose[51], xmin = pose[52], ymax = pose[53], xmax = pose[54], score = pose[55];
			if (score < poseThreshold) continue;

			cv::Rect bbox = cv::Rect(
				static_cast<int>(width * xmin),
				static_cast<int>(height * ymin),
				static_cast<int>(width * (xmax - xmin)),
				static_cast<int>(height * (ymax - ymin))
			);

			cv::rectangle(target, bbox, red, 2);
			cv::putText(target, to_string(cvRound(100 * score)) + "%", cv::Point(static_cast<int>(width * xmin), static_cast<int>(height * ymin - 5)), cv::FONT_HERSHEY_SIMPLEX, 0.5, red);
		}

		// draw keypoints
		for (int k = 0; k < 17; k++) {
			float* keypoint = pose + 3 * k;
			float y = keypoint[0], x = keypoint[1], conf = keypoint[2];
			if (conf < keypointThreshold) continue;

			cv::circle(target, cv::Point(static_cast<int>(x * width), static_cast<int>(y * height)), 2, red, cv::FILLED);
		}

		// draw skeleton
		for (const auto& connection : connections) {
			float* keypoint1 = pose + 3 * connection.first;
			float y1 = keypoint1[0], x1 = keypoint1[1], conf1 = keypoint1[2];

			float* keypoint2 = pose + 3 * connection.second;
			float y2 = keypoint2[0], x2 = keypoint2[1], conf2 = keypoint2[2];

			if (conf1 < keypointThreshold || conf2 < keypointThreshold) continue;

			auto p1 = cv::Point(static_cast<int>(x1 * width), static_cast<int>(y1 * height));
			auto p2 = cv::Point(static_cast<int>(x2 * width), static_cast<int>(y2 * height));

			cv::line(target, p1, p2, red, 1);
		}
	}

	// FPS Counter
	cv::putText(target, to_string(cvRound(1000.0 / frameMs)) + " FPS", cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, red);
}

int main() {
	bool multiPose;
	string modelFile, answer;
	cout << "Use multi-pose model? (y/n): " << flush;
	cin >> answer;
	if (answer.starts_with("y") || answer.starts_with("Y")) {
		multiPose = true;
		modelFile = "../assets/lite-model_movenet_multipose_lightning_tflite_float16_1.tflite";
		cout << "Using multi-pose model" << endl;
	}
	else {
		multiPose = false;
		modelFile = "../assets/lite-model_movenet_singlepose_lightning_tflite_float16_4.tflite";
		cout << "Using single-pose model" << endl;
	}

	/*
		TFLITE SETUP
	*/
	cout << "Loading interpreter" << endl;
	auto model = tflite::FlatBufferModel::BuildFromFile(modelFile.c_str());

	if (!model) {
		throw runtime_error("Failed to load model from " + modelFile);
	}
	cout << "Model loaded: " << modelFile << endl;

	tflite::ops::builtin::BuiltinOpResolver op_resolver;
	unique_ptr<tflite::Interpreter> interpreter;
	tflite::InterpreterBuilder(*model, op_resolver)(&interpreter);

	if (interpreter->AllocateTensors() != kTfLiteOk) {
		throw runtime_error("Failed to allocate tensors");
	}

	// For debug purposes:
	// tflite::PrintInterpreterState(interpreter.get());

	auto input = interpreter->inputs()[0];
	auto output = interpreter->outputs()[0];

	auto output_dims_size = interpreter->tensor(output)->dims->size;
	auto output_dims = interpreter->tensor(output)->dims->data;

	auto input_dims_size = interpreter->tensor(input)->dims->size;
	auto input_dims = interpreter->tensor(input)->dims->data;

	cout << "The input tensor has the following dimensions: [" << input_dims;
	for (int d = 1; d < input_dims_size; d++) {
		cout << ", " << input_dims[d];
	}
	cout << "]" << endl;

	if (input_dims_size < 3) {
		throw runtime_error("Input dims should be at least 3d.");
	}

	int inputWidth = input_dims[2];
	int inputHeight = input_dims[1];

	if (multiPose) {
		cout << "Multi-pose: Please provide an input image width and height (must be multiples of 32, larger side is recommended to be 256)." << endl;

		cout << "Input width (recommended: 256): " << flush;
		cin >> inputWidth;
		if (inputWidth % 32 != 0) {
			throw runtime_error("Input width must be a multiple of 32.");
		}

		cout << "Input height (recommended: 192): " << flush;
		cin >> inputHeight;
		if (inputWidth % 32 != 0) {
			throw runtime_error("Input height must be a multiple of 32.");
		}

		// this is necessary for models with dynamic input size (like MoveNet Lightning Multipose - 1 x H x W x 3)
		interpreter->ResizeInputTensor(0, { 1, inputHeight, inputWidth, 3 });

		if (interpreter->AllocateTensors() != kTfLiteOk) {
			throw runtime_error("Failed to reallocate tensors");
		}
	}

	cout << "The output tensor has the following dimensions: [" << output_dims;
	for (int d = 1; d < output_dims_size; d++) {
		cout << ", " << output_dims[d];
	}
	cout << "]" << endl;

	int poses = output_dims_size > 1 ? output_dims[1] : 1;
	cout << "Number of detected poses: " << poses << endl;

	/*
		OPENCV SETUP
	*/

	cv::Mat frame, resized;
	cv::namedWindow("Output");
	cv::VideoCapture cap(0);

	if (!cap.isOpened()) {
		cout << "No video stream detected" << endl;
		system("pause");
		return -1;
	}

	auto time = chrono::steady_clock::now();
	double avgFrameMs = 1000.0 / 30.0;

	while (true) {
		cap >> frame;
		if (frame.empty()) {
			break;
		}

		// cut contains the right aspect ratio, resized is a smaller version of cut for inference
		cv::Mat cropped = resizeFrame(frame, resized, inputWidth, inputHeight);

		float* results = runInterpreter(interpreter, resized);

		auto now = chrono::steady_clock::now();
		auto frameMs = chrono::duration_cast<chrono::milliseconds>(now - time).count();
		time = now;
		avgFrameMs += 0.05 * (frameMs - avgFrameMs);
		// draw on larger cropped matrix
		drawKeypoints(cropped, results, poses, avgFrameMs);

		cv::imshow("Output", cropped);
		if (cv::waitKey(5) == 27) {
			// ESC pressed
			break;
		}
	}

	cap.release();
	cv::destroyAllWindows();
	return 0;
}