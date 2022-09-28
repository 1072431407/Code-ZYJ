#include <stdio.h>
#include <fstream>
#include <string>
#include <list>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/hal/interface.h>
#include <TNNKit.h>
#include <XJFace.h>

using namespace std;
using namespace cv;


static list<vector<float>> s_features;
XJFace xjFace;

void drawBox(cv::Mat& img, const vector<XJFace::FaceBoxInfo>& boxs) {
	int w = img.cols;
	int h = img.rows;
	for (const XJFace::FaceBoxInfo& box : boxs) {
		cv::Rect2f r = cv::Rect2f(box.rect.x * w, box.rect.y * h, box.rect.width * w, box.rect.height * h);
		cv::rectangle(img , r, cv::Scalar(255, 0, 0), 2);
		for (const XJFace::FacePoint& p : box.landmarks) {
			cv::Point src_p(p.x * w , p.y*h);
			//cv::drawMarker(img, src_p, cv::Scalar(255, 0, 0), 2);
		}
		cv::putText(img, string("id:") + to_string(box.trackId), cv::Point(int(box.rect.x * w), int(box.rect.y * h + 30)), 4, 1, cv::Scalar(0, 255, 0));
		//cv::putText(img, string("score:") + to_string(box.score), cv::Point(int(box.rect.x), int(box.rect.y + 30)), 4, 1, cv::Scalar(0, 255, 0));
	}
}

void test1(const cv::Size& video_sln) {
	//faceManager
	string face_detector_proto_path = std::string(TEST_DATA_PATH) + "/RetinaFace/FaceDetector_mobile0.25_320x320.opt.tnnproto";
	string face_detector_model_path = std::string(TEST_DATA_PATH) + "/RetinaFace/FaceDetector_mobile0.25_320x320.opt.tnnmodel";

	string mobileface_proto_path = std::string(TEST_DATA_PATH) + "/MobileFace/mobileface_128x128_512_v0.6.opt.tnnproto";
	string mobileface_model_path = std::string(TEST_DATA_PATH) + "/MobileFace/mobileface_128x128_512_v0.6.opt.tnnmodel";

	// string ssan_proto_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_320x320.opt.tnnproto";
	// string ssan_model_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_320x320.opt.tnnmodel";
	string ssan_proto_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_256x256.opt.tnnproto";
	string ssan_model_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_256x256.opt.tnnmodel";
	string yuv_img_path = std::string(TEST_DATA_PATH) + "/yuv/2022-07-31-131551_W1280_H720.YUV_420_888.yuv";

	int ret = xjFace.init(TNNKit::readProtoContent(face_detector_proto_path)
		, TNNKit::readModelContent(face_detector_model_path)
		, TNNKit::readProtoContent(mobileface_proto_path)
		, TNNKit::readModelContent(mobileface_model_path)
		, TNNKit::readProtoContent(ssan_proto_path)
		, TNNKit::readModelContent(ssan_model_path)
	);
	cout << "xjFace.init ret=" << ret << endl;
	//camera
	VideoCapture capture(0);
	//capture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, 1280);
	//capture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, 720);
	capture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, video_sln.width);
	capture.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, video_sln.height);

	Mat frame;
	//capture >> frame;
	int key;
	while ((key = waitKey(10)) != 27)
	{
		vector<XJFace::FaceBoxInfo> results;
		capture >> frame;
		xjFace.detect(frame.data, frame.cols, frame.rows, 0, XJFace::IMAGE_TYPE_BGR, results);
		switch (key)
		{
		case 'e': {
			for (XJFace::FaceBoxInfo& box : results) {
				vector<float> features;
				if (!xjFace.extrackFeature(box.trackId, features)) {
					cout << "features size:" << features.size() << endl;
					stringstream ss;
					for_each(features.begin(), features.end(), [&ss](float v) {
						ss << v << ",";
					});
					cout << "feature = " << "[" << ss.str() << "]" << endl;
					cout << "=================================" << endl;
					waitKey();
				}
			}
		}break;
		case 'j': {
			for (XJFace::FaceBoxInfo& box : results) {
				float score;
				if (!xjFace.antiSpoofing(box.trackId, score)) {
					cout << "score:" << score << endl;
					waitKey();
				}
			}
		}break;
		default:
			break;
		}
		drawBox(frame, results);
		cv::imshow("frame",frame);
	}
}

void test2() {
	string face_detector_proto_path = std::string(TEST_DATA_PATH) + "/RetinaFace/FaceDetector_mobile0.25_320x320.opt.tnnproto";
	string face_detector_model_path = std::string(TEST_DATA_PATH) + "/RetinaFace/FaceDetector_mobile0.25_320x320.opt.tnnmodel";

	string mobileface_proto_path = std::string(TEST_DATA_PATH) + "/MobileFace/mobileface_128x128_512_v0.6.opt.tnnproto";
	string mobileface_model_path = std::string(TEST_DATA_PATH) + "/MobileFace/mobileface_128x128_512_v0.6.opt.tnnmodel";

	// string ssan_proto_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_320x320.opt.tnnproto";
	// string ssan_model_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_320x320.opt.tnnmodel";
	string ssan_proto_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_256x256.opt.tnnproto";
	string ssan_model_path = std::string(TEST_DATA_PATH) + "/SpoofJudger/SSAN_R_256x256.opt.tnnmodel";
	string img_path = std::string(TEST_DATA_PATH) + "/yuv/record.yuv";

	cout << "xjFace.VersionName=" << xjFace.getVersionName() << endl;
	int ret = xjFace.init(TNNKit::readProtoContent(face_detector_proto_path)
		, TNNKit::readModelContent(face_detector_model_path)
		, TNNKit::readProtoContent(mobileface_proto_path)
		, TNNKit::readModelContent(mobileface_model_path)
		, TNNKit::readProtoContent(ssan_proto_path)
		, TNNKit::readModelContent(ssan_model_path)
	);
	cout << "xjFace.init ret=" << ret << endl;
	ifstream img_stream(img_path, ios::binary);
	stringstream ss;
	ss << img_stream.rdbuf();
	string img_content = ss.str();
	vector<XJFace::FaceBoxInfo> results;
	ret = xjFace.detect((void*)img_content.data(), 640, 480, 0, XJFace::IMAGE_TYPE_NV21, results, false);
	cout << "xjFace.detect ret=" << ret << endl;
	cout << "xjFace.detect results.size=" << results.size() << endl;
	if (results.size() == 1) {
		vector<float> feature;
		xjFace.extrackFeature(results.front().trackId, feature);
		cout << "xjFace.extrackFeature ret=" << ret << endl;
	}
}

int main()
{
	xjFace.setLogLevel(XJFace::XJFaceLogError | XJFace::XJFaceLogWarning | XJFace::XJFaceLogInfo);
	test1(cv::Size(1280,720));
	//test2();
	return 0;

}
