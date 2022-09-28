#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <spoofjudger/ssan_r_spoof_judger.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


static std::string gBenchResultStr = "";

void setBenchResult(std::string result) {
	gBenchResultStr = result;
}

int main() {
	auto start0 = std::chrono::high_resolution_clock::now();
	string image_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/real13.jpg";
	//string proto_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_320x320.opt.tnnproto";
	//string model_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_320x320.opt.tnnmodel";
	//const TNNKit::NetInputShape& net_input_shape = {320, 320, 3}; 
	string proto_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_256x256.opt.tnnproto";
	string model_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_256x256.opt.tnnmodel";
	const TNNKit::NetInputShape& net_input_shape = {256, 256, 3}; 
	TNN_NS::MatConvertParam input_cvt_param;
	input_cvt_param.scale = { 1.0 / 128.0, 1.0 / 128.0, 1.0 / 128.0, 0.0 };
	input_cvt_param.bias = { -127.5 / 128.0, -127.5 / 128.0, -127.5 / 128.0, 0.0 };
	Classifier* spoof_judger = new SsanRSpoofJudger();

	int ret = spoof_judger->init(TNNKit::readProtoContent(proto_path), TNNKit::readModelContent(model_path), net_input_shape, input_cvt_param);
	if (ret != 0) {
		cout << "Spoof Judger init failed: " << ret << endl;
		return -1;
	}
	cv::Mat img = cv::imread(image_path, cv::IMREAD_COLOR);
	//cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
	//cv::resize(img, img, cv::Size(320, 320));
	std::unordered_map<std::string, float> name2score = spoof_judger->predict(img);

	int face_number = 0;
	std::for_each(name2score.begin(), name2score.end(), [&face_number](const std::pair<std::string, float> n2s) {
																		face_number = n2s.second > 0.5f ? face_number + 1 : face_number;
																		});
	std::cout << "face number: " << face_number << std::endl;

	stringstream ss;
	std::for_each(name2score.begin(), name2score.end(), [&ss](const std::pair<std::string, float>& name2score) {
		ss << name2score.first << ": " << name2score.second << "\n";
	});
	cout << ss.str() << endl;
	cv::imshow("img", img);
	cv::waitKey();
	return getchar();
}
