#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <detector/retina_face_detector.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


static std::string gBenchResultStr = "";

void setBenchResult(std::string result) {
	gBenchResultStr = result;
}

void drawBox(cv::Mat& img, const std::vector<RecogInfo>& boxs) {
	int w = img.cols;
	int h = img.rows;
	for (const RecogInfo& box : boxs) {
		cv::Rect2f r = cv::Rect2f(box.x1 * w, box.y1 * h, (box.x2 - box.x1)* w, (box.y2 - box.y1) * h);
		cv::rectangle(img, r , cv::Scalar(255, 0, 0), 2);
		for (const cv::Point2f& p : box.landmarks) {
			cv::drawMarker(img, cv::Point(p.x*w,p.y*h), cv::Scalar(255, 0, 0), 2);
		}
		cv::putText(img, string("score:") + to_string(box.score), cv::Point(int(r.x), int(r.y + 30)), 4, 1, cv::Scalar(0, 255, 0));
	}
}

int main() {
	auto start0 = std::chrono::high_resolution_clock::now();
	string proto_path = std::string(TEST_DATA_PATH) + "/RetinaFace/FaceDetector_mobile0.25_320x320.opt.tnnproto";
	string model_path = std::string(TEST_DATA_PATH) + "/RetinaFace/FaceDetector_mobile0.25_320x320.opt.tnnmodel";
	vector<string> image_paths = { 
			std::string(TEST_DATA_PATH) + "/RetinaFace/big.jpg",
			std::string(TEST_DATA_PATH) + "/RetinaFace/crop.jpg",
			std::string(TEST_DATA_PATH) + "/RetinaFace/meeting0.jpg",
			std::string(TEST_DATA_PATH) + "/RetinaFace/meeting1.jpg",
			std::string(TEST_DATA_PATH) + "/RetinaFace/meeting2.jpg",
			std::string(TEST_DATA_PATH) + "/RetinaFace/meeting3.jpg",
			std::string(TEST_DATA_PATH) + "/RetinaFace/meeting4.jpg",
			std::string(TEST_DATA_PATH) + "/RetinaFace/meeting5.jpg",
	};

	Detector* retina_face_detector = new RetinaFaceDetector();
	int ret = retina_face_detector->init(TNNKit::readProtoContent(proto_path), TNNKit::readModelContent(model_path));
	if (ret != 0) {
		std::cout << "detector_init failed: " << ret << endl;
		return -1;
	}
	cv::Mat img;
	std::vector<RecogInfo> output;
	for (string path : image_paths) {
		img = cv::imread(path, cv::IMREAD_COLOR);
		output = retina_face_detector->detect(img, 0.8, 0.5f);
		stringstream ss;
		std::for_each(output.begin(), output.end(), [&ss](const RecogInfo& box) {
			ss << box.label << "," << box.score << ",(" << box.x1 << "," << box.y1 << "),(" << box.x2 << "," << box.y2 << ")" << "\n";
		});
		std::cout << ss.str() << endl;
#ifndef SYS_ANDROID
		drawBox(img, output);
		cv::imshow("img", img);
		cv::waitKey();
#endif // SYS_ANDROID
	}
	return 0;
}
