#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include<alignment/alignment.h>

using namespace std;

void drawMark(cv::Mat& img,vector<Point2f>& landmarks) {
	for (const Point2f& point : landmarks) {
		cv::drawMarker(img, point, cv::Scalar(255, 0, 0), 2);
	}
}

int main() {
	cout << "alignment" << endl;
	cv::Mat imgs[10];
	std::vector<cv::Point2f> landmarks[10];

	imgs[0] = cv::imread(std::string(TEST_DATA_PATH) + "/Alignment/test_face04.jpg", cv::IMREAD_UNCHANGED);
	landmarks[0] = {Point2f(145.26517, 161.15865), \
		Point2f(192.50267, 143.54042), \
		Point2f(184.4904, 179.20639), \
		Point2f(178.36884, 209.08125), \
		Point2f(218.14134, 194.67761) };


	imgs[1] = cv::imread(std::string(TEST_DATA_PATH) + "/Alignment/oblique2.jpg", cv::IMREAD_UNCHANGED);
	landmarks[1] = { Point2f(875, 371), \
		Point2f(929, 394), \
		Point2f(887, 415), \
		Point2f(863, 429), \
		Point2f(898, 444) };

	Alignment alignImage = Alignment();
	for (int i = 0; i < 2;i++) {
		Mat cropImage;
		int ret = alignImage.align(imgs[i], landmarks[i], cropImage);
		int* p = alignImage.getBBox();
#ifndef SYS_ANDROID
		cout << p[0] << " " << p[1] << " " << p[0] + p[2] << " " << p[1] + p[3] << endl;
		drawMark(imgs[i], landmarks[i]);
		cv::imshow("img", imgs[i]);
		cv::imshow("cropimg", cropImage);
		cv::waitKey(3000);
#endif // SYS_ANDROID
	}
	
}
