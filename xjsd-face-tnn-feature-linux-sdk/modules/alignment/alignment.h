//
// Created by hww on 2022/8/11.
//
#ifndef ALIGNMENT_H
#define ALIGNMENT_H
#include <string>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/imgproc/types_c.h>

using namespace cv;

class Alignment {
public:
	Alignment();
	virtual ~Alignment();

	float m_scale = 2.7f;
	

	//Mat Align(Mat croppedImage, const Rect box, const Point2f landmarks[], int landmarks_size);
	//Mat Align(const Mat& image, float box[], int box_size, float landmarks[], int landmarks_size);

	int align(const Mat& image, const std::vector<cv::Point2f>& landmarkss,Mat& dst);
	int* getBBox();

private:
	Mat m_std_landmarks = (Mat_<float>(5, 2) << 30.2946, 51.6963, 30.2946, 51.6963, 48.0252, 71.7366, 33.5493, 92.3655, 62.7299, 92.20411);
	int m_newBBox[4];   // x, y, w, h

	void pointToBBox(const std::vector<cv::Point2f>& landmarks, int* BBox);
	void scaleBBox(int imageSize[], int* BBox, float scale);
};
#endif //OPENCV_STATICLIB_DEMO_FACEPREPROCESS_H
