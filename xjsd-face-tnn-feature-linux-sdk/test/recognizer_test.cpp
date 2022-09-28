#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <detector/retina_face_detector.h>
#include <recognizer/MobileFace.h>

using namespace std;



int main()
{
	//auto start0 = std::chrono::high_resolution_clock::now();
	cout << "extract" << endl;
	
	string proto_path = std::string(TEST_DATA_PATH) + "/MobileFace/mobileface_128x128_512_v0.6.opt.tnnproto";
	string model_path = std::string(TEST_DATA_PATH) + "/MobileFace/mobileface_128x128_512_v0.6.opt.tnnmodel";
	cv::Mat img1 = cv::imread("D:/source/face/linux-face-tnn/records/extrack-1661848762423.png", cv::IMREAD_COLOR);
	cv::Mat img2 = cv::imread(std::string(TEST_DATA_PATH) + "/MobileFace/tnn_crop_image2.jpg", cv::IMREAD_COLOR);

	MobileFace mobileFace;
	mobileFace.init(TNNKit::readProtoContent(proto_path), TNNKit::readModelContent(model_path));
	std::vector<float>features1;
	std::vector<float>features2;
	mobileFace.extract(img1, features1);
	mobileFace.extract(img2, features2);
	float score = mobileFace.featureContrast(features1, features2);
	cout << "featureContrast score:" << score << endl;

	cout << "=====================================================" << endl;
	stringstream ss1;
	ss1 << "features1=";
	for_each(features1.begin(), features1.end(), [&ss1](float f) {
		ss1 << f << ",";
	});
	cout << ss1.str() << endl;

	cout << "=====================================================" << endl;
	stringstream ss2;
	ss2 << "features2=";
	for_each(features2.begin(), features2.end(), [&ss2](float f) {
		ss2 << f << ",";
	});
	cout << ss2.str() << endl;


	//string proto_path = std::string(TEST_DATA_PATH) + "/MobileFace/yolov5s_sim_opt.tnnproto";
	//string model_path = std::string(TEST_DATA_PATH) + "/MobileFace/yolov5s_sim_opt.tnnmodel";
	//string img_path = std::string(TEST_DATA_PATH) + "/MobileFace/head.png";
	//cv::Mat img = cv::imread(img_path,cv::IMREAD_UNCHANGED);
	//cv::imshow("img", img);
	//cv::waitKey();
	return getchar();
}
