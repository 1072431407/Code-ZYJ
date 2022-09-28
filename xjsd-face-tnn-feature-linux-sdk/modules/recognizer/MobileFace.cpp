#include "MobileFace.h"
#include "omp.h"
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <xlog/XLog.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

static const char* TAG = "MobileFace";

MobileFace::MobileFace() {
}

MobileFace::~MobileFace() {

}

int MobileFace::init(const std::string& proto_content,const std::string& model_content) {


	TNNKit::NetInputShape input_shape = {128,128,3};
	TNN_NS::MatConvertParam input_cvt_param;
	input_cvt_param.scale = { 1.0 / 128, 1.0 / 128, 1.0 / 128 };
	input_cvt_param.bias = { -127.0 / 128, -127.0 / 128, -127.0 / 128 };

	TNN_NS::Status status = m_tnnKit.init(proto_content, model_content, input_shape, input_cvt_param);
	if(status != TNN_NS::TNN_OK) {
		xlogE(TAG,"tnnkit init failed! %s",status.description().c_str());
	}
	return (int)status;
}

int MobileFace::extract(const cv::Mat& img, std::vector<float>& features) {
	//bgr转rgb颜色
	cv::Mat data_img = img.clone();
	cv::cvtColor(data_img, data_img, cv::COLOR_BGR2RGB);
	TNN_NS::Status status = m_tnnKit.forward(data_img);
	if (status != TNN_NS::TNN_OK) {
		xlogE(TAG, "tnnkit forward failed! %s", status.description().c_str());
		return (int)status;
	}
	// 获取数据
	std::shared_ptr<TNN_NS::Mat> output_mat;
	TNN_NS::MatConvertParam convertParam;
	status = m_tnnKit.getOutputMat(output_mat, convertParam, "");
	if (status != TNN_NS::TNN_OK) {
		xlogE(TAG, "tnnkit getOutputMat Error: %s", status.description().c_str());
		return (int)status;
	}
	auto* data = static_cast<float*>(output_mat->GetData());
	for (int ii = 0; ii < DIMENSION; ii++) {
		features.push_back(data[ii]);
	}
	return TNN_NS::TNN_OK;
}

float MobileFace::featureContrast(const std::vector<float>& feature1, const std::vector<float>& feature2) {
	float mul = 0;
	float featureNum = 0;
	float featureTargetNum = 0;
	for (int i = 0; i < feature1.size(); i++) {
		mul += feature1[i] * feature2[i];
		featureNum += feature1[i] * feature1[i];
		featureTargetNum += feature2[i] * feature2[i];
	}
	auto score = mul / (float)(sqrt(featureNum * featureTargetNum));
	// 将result从 （-1，1） 归一化到 （0，1）
	score = (score + (float)1.0) * (float)0.5;
	return score;
}

