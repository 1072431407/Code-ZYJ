#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <yolo/Yolov5.h>

//using namespace std;

using namespace cv;
using namespace std;

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480

char buff[2000000];

void drawBox(cv::Mat& img, const std::vector<BoxInfo>& boxs)
{
	for (const BoxInfo& box : boxs) {
		cv::rectangle(img, cv::Point(int(box.x1), int(box.y1)), cv::Point(int(box.x2), int(box.y2)), cv::Scalar(255, 0, 0), 2);
		cv::putText(img, string("label:") + to_string(box.label), cv::Point(int(box.x1), int(box.y1 + 30)), 4, 1, cv::Scalar(0, 255, 0));
		cv::putText(img, string("score:") + to_string(box.score), cv::Point(int(box.x1), int(box.y1 + 60)), 4, 1, cv::Scalar(0, 255, 0));
	}
}

//int readNV12toRGB1()
//{
//	int width = 640;
//	int height = 480;
//	int yuvNV12_size = width * height * 3 / 2;
//	int rgb24_size = width * height;
//
//	cv::Mat yuvNV12;
//	cv::Mat rgb24;
//	string pic_dir = std::string(TEST_DATA_PATH) + "/test/";
//
//	vector<cv::String> files_yuv;
//	cv::glob(pic_dir + "*.YUV_420_888", files_yuv);
//	//cv::glob(pic_dir + " * .yuv", files_yuv);
//
//	for (size_t i = 0; i < files_yuv.size(); i++)
//	{
//		printf("image file : %s \n", files_yuv[i].c_str());
//
//		//1.read nv12 file to nv12 mat
//		FILE* f = fopen(files_yuv[i].c_str(), "r");
//
//		memset(buff, 0, 2000000);
//		fread(buff, 1, yuvNV12_size, f);
//		yuvNV12.create(height * 3 / 2, width, CV_8UC1);
//		memcpy(yuvNV12.data, buff, yuvNV12_size);
//		//2.cvt nv12 mat to rgb24 mat
//		cvtColor(yuvNV12, rgb24, COLOR_YUV2BGR_NV12);
//		//3.imwrite
//
//		std::string savePath = files_yuv[i] + "_rgb.jpg";
//		imwrite(savePath, rgb24);
//		fflush(f);
//		fclose(f);
//	}
//
//	return 0;
//}

int readNV12toRGB()
{
	int width = 640;
	int height = 480;
	int yuvNV12_size = width * height * 3 / 2;
	int rgb24_size = width * height;

	cv::Mat yuvNV12;
	cv::Mat rgb24;
	string pic_dir = std::string(TEST_DATA_PATH) + "/test/";

	vector<cv::String> files_yuv;
	//cv::glob(pic_dir + "*.YUV_420_888", files_yuv);
	cv::glob(pic_dir + "*.yuv", files_yuv);
	//cv::glob(pic_dir + " * .yuv", files_yuv);
	//cv::namedWindow("pic", cv::WINDOW_AUTOSIZE);// #创建窗口

	for (size_t i = 0; i < files_yuv.size(); i++)
	{
		printf("image file : %s \n", files_yuv[i].c_str());
		auto start = std::chrono::high_resolution_clock::now();
		//1.read nv12 file to nv12 mat
		FILE* f = fopen(files_yuv[i].c_str(), "r");

		memset(buff, 0, 2000000);
		fread(buff, 1, yuvNV12_size, f);
		yuvNV12.create(height * 3 / 2, width, CV_8UC1);
		memcpy(yuvNV12.data, buff, yuvNV12_size);
		//2.cvt nv12 mat to rgb24 mat
		//cvtColor(yuvNV12, rgb24, COLOR_YUV2BGR_NV12);
		//yuv yyyyyyyyuuvv通道是I420 facepp
		//cvtColor(yuvNV12, rgb24, COLOR_YUV2BGR_I420);
		//nv12 from andord 5100
		//cvtColor(yuvNV12, rgb24, COLOR_YUV2BGR_NV12);
		//3.imwrite
		//cvtColor(yuvNV12, rgb24, COLOR_GRAY2BGR);
		//DEBUG_CAMERA_DATA_PREPROCESS = 2
		cvtColor(yuvNV12, rgb24, COLOR_YUV420sp2RGB);
		auto end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double, std::milli> tm = end - start;	// 毫秒
		auto timeDif = tm.count();
		std::string savePath = files_yuv[i] + "_rgb.jpg";
		imwrite(savePath, rgb24);
		fflush(f);
		fclose(f);

		auto img = cv::imread(savePath);//#读取这个路径的图片
		cv::waitKey(0);//#窗口显示时间，单位：毫秒
		cv::putText(img, "time: " + std::to_string(timeDif) + "ms", { img.rows - 20,int(img.cols * 0.1) }, 4, 1, cv::Scalar(0, 255, 0));
		cv::imshow("image", img); //#显示图片窗口
		cv::waitKey(0);
	}
	//cv::destroyAllWindows();// #删除建立的全部窗口，释放资源

	return 0;
}

int showImg(cv::Mat& img)
{
	//ifstream fd_yuv("in.yuv");

	////yuv Mat
	//cv::Mat yuv_img(IMAGE_HEIGHT * 1.5, IMAGE_WIDTH, CV_8UC1);

	////read file to Mat
	//fd_yuv.read((char*)yuv_img.data, IMAGE_WIDTH* IMAGE_HEIGHT);

	//jpg Mat
	/*cv::Mat jpg_img;
	cv::cvtColor(yuv_img, jpg_img, CV_YUV2RGB_NV12);
	imwrite("out.jpg", jpg_img);*/
	/*cv::putText(img, "time: "+std::to_string(timeDif)+ "ms", { img.rows-50,int(img.cols*0.7) }, 4, 1, cv::Scalar(0, 255, 0));
	cv::imshow("image", img);*/
	return 0;
}
 

int main()
{

	readNV12toRGB();
	return -2;
	cout << "yolo_test" << endl;
	string proto_path = std::string(TEST_DATA_PATH) + "/Yolo/yolov5s_sim_opt.tnnproto";
	string model_path = std::string(TEST_DATA_PATH) + "/Yolo/yolov5s_sim_opt.tnnmodel";
	string img_path = std::string(TEST_DATA_PATH) + "/Yolo/head.png";
	cv::Mat img = cv::imread(img_path, cv::IMREAD_COLOR);
	cout << "img " << img.cols << "," << img.rows << endl;
	Yolov5 yolov5;
	yolov5.init(TNNKit::readProtoContent(proto_path), TNNKit::readModelContent(model_path));
	std::vector<BoxInfo> boxs = yolov5.detect(img, 0.7f, 0.3f);
	stringstream ss;
	std::for_each(boxs.begin(), boxs.end(), [&ss](const BoxInfo& box) {
		ss << box.label << "," << box.score << ",(" << box.x1 << "," << box.y1 << "),(" << box.x2 << "," << box.y2 << ")" << "\n";
	});
	cout << ss.str() << endl;
	drawBox(img, boxs);
	cv::imshow("img", img);
	cv::waitKey();
	return getchar();
}


 