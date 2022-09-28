#include "alignment.h"
#include <math.h>


using namespace cv;

static const float PI = 3.1415926f;

Alignment::Alignment()
{
}

Alignment::~Alignment()
{
}

int Alignment::align(const Mat& image, const std::vector<cv::Point2f>& landmarks, Mat& dst)
{
    if (image.empty()) {
        return -1;
    }
	if (landmarks.size() < 5) {
		return -2;
	}
    Mat tarlandmarks = (Mat_<float>(5, 2) << landmarks[0].x, landmarks[0].y, \
                                            landmarks[1].x, landmarks[1].y, \
                                            landmarks[2].x, landmarks[2].y, \
                                            landmarks[3].x, landmarks[3].y, \
                                            landmarks[4].x, landmarks[4].y);
    // get warp matric
    Mat rM = cv::estimateAffine2D(this->m_std_landmarks, landmarks);
    float tanA = rM.at<double>(1, 0) / rM.at<double>(1, 1);
    float tanB = -rM.at<double>(0, 1) / rM.at<double>(0, 0);
    float tanAB = (tanA + tanB) / 2;
    double angle = atan(tanAB) * 180 / PI;
    Point2f center = landmarks[2];
    Mat warpM = cv::getRotationMatrix2D(center, angle, 1.0f);

    // warp image
    int rows = image.rows;
    int cols = image.cols;
    Mat rotImage = Mat::zeros(rows, cols, CV_32FC2);
    cv::warpAffine(image, rotImage, warpM, Size2i(cols,rows));

    // warp point
    std::vector<Point2f> newlandmarks;
    for (int ii = 0; ii < landmarks.size(); ii++) {
        Mat p = (Mat_ <double> (3, 1) << (double)landmarks[ii].x, (double)landmarks[ii].y, (double)1.0);
        Mat pp = warpM * p;
        cv::Point2f point = Point2f((float)pp.at<double>(0, 0), (float)pp.at<double>(1, 0));
		newlandmarks.push_back(point);
		//cv::drawMarker(rotImage, point, cv::Scalar(255, 0, 0), 2);
    }
    // get bbox
    this->pointToBBox(newlandmarks, this->m_newBBox);
    int size[] = { cols, rows };
    this->scaleBBox(size, this->m_newBBox, this->m_scale);

    // crop image
    Rect rect(this->m_newBBox[0], this->m_newBBox[1], this->m_newBBox[2], this->m_newBBox[3]);
    if (rect.width == 0 || rect.height == 0) {
        //image empty
        return -3;
    }
    else if (rect != (rect & Rect(0, 0, rotImage.cols, rotImage.rows))) {
		//ROI boundary crossing
        return -4;
	}
	else {
        //copy image
		rotImage(rect).copyTo(dst);
		return 0;
    }
}

int* Alignment::getBBox()
{
    return this->m_newBBox;
}

void Alignment::pointToBBox(const std::vector<cv::Point2f>& landmarks, int* BBox)
{
    int min_x = 100000;
    int max_x = 0;
    int min_y = 100000;
    int max_y = 0;
    for (int ii = 0; ii < landmarks.size(); ii++) {
        Point2f p = landmarks[ii];
        min_x = min_x < p.x ? min_x : p.x;
        max_x = max_x > p.x ? max_x : p.x;
        min_y = min_y < p.y ? min_y : p.y;
        max_y = max_y > p.y ? max_y : p.y;
    }
    BBox[0] = int(min_x);
    BBox[1] = int(min_y);
    BBox[2] = int(max_x-min_x);
    BBox[3] = int(max_y-min_y);
}

void Alignment::scaleBBox(int imageSize[], int* BBox, float scale)
{
    float x_mid = (int)(BBox[0] + BBox[2] / 2.0);
    float y_mid = (int)(BBox[1] + BBox[3] / 2.0);
    float w_scale = scale * BBox[2];
    float h_scale = scale * BBox[3];
    BBox[0] = int(x_mid - w_scale / 2.0) > 0 ? int(x_mid - w_scale / 2.0) : 0;
    BBox[1] = int(y_mid - h_scale * 0.6) > 0 ? int(y_mid - h_scale * 0.6) : 0;
    BBox[2] = int(x_mid + w_scale / 2.0) > imageSize[0] - 1 ? imageSize[0] - 1 - BBox[0] : int(x_mid + w_scale / 2.0f) - BBox[0];
    BBox[3] = int(y_mid + h_scale * 0.4) > imageSize[1] - 1 ? imageSize[1] - 1 - BBox[1] : int(y_mid + h_scale * 0.4f) - BBox[1];
}
