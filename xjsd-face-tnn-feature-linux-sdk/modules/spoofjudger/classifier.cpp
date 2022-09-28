#include <fstream>
#include <xlog/XLog.h>
#include <opencv2/imgproc.hpp>
#include "omp.h"
#include "classifier.h"


constexpr char* LOG_TAG = "Classifier";

Classifier::Classifier() {
    m_tnnKit = new TNNKit();
}

Classifier::~Classifier() {
    delete m_tnnKit;
}

int Classifier::init(const std::string& proto_path, 
					 const std::string& model_path, 
					 const TNNKit::NetInputShape& net_input_shape, 
					 const TNN_NS::MatConvertParam& input_cvt_param) {
    this->net_input_width = net_input_shape.width;
    this->net_input_height = net_input_shape.height;
	TNN_NS::Status status = m_tnnKit->init(proto_path, model_path, net_input_shape, input_cvt_param);
	if (status != TNN_NS::TNN_OK) {
		xlogE(LOG_TAG, "TNN init failed %s(%d)",status.description().c_str(), (int)status);
    } 
	return (int)status;
}

std::unordered_map<std::string, float> Classifier::predict(const cv::Mat& image) {
    cv::Mat img = image.clone();
    if (this->convert_flag) {
		cv::cvtColor(image, img, cv::COLOR_BGR2RGB);
    }
    if (this->input_layer_names.empty()) {
        xlogE(LOG_TAG, "input layer names not set");
        return {};
    }
    if (nullptr == this->score_threshold) {
        xlogE(LOG_TAG, "score threshold not set");
        return {};
    }
    std::unordered_map<std::string, cv::Mat> name2imgs;
    for (const auto& name : this->input_layer_names) {
        name2imgs[name] = img;
    }
    return this->predict(name2imgs, *this->score_threshold);
}

std::unordered_map<std::string, float> Classifier::predict(const std::unordered_map<std::string, cv::Mat>& name2imgs, float score_threshold) {
    TNN_NS::Status status = m_tnnKit->forward(name2imgs);
    if (status != TNN_NS::TNN_OK) {
        xlogE(LOG_TAG, "instance.Forward Error: %s", status.description().c_str());
        return {};
    }
    // 获取数据
    std::vector<std::shared_ptr<TNN_NS::Mat>> output_mats;
    for (const std::string& layer_name : this->output_layer_names) {
        std::shared_ptr<TNN_NS::Mat> output_mat;
        TNN_NS::MatConvertParam param;
        status = m_tnnKit->getOutputMat(output_mat, param, layer_name);
        if (status != TNN_NS::TNN_OK) {
            xlogE(LOG_TAG, "instance.GetOutputMat Error: %s", status.description().c_str());
            return {};
        }
        output_mats.push_back(output_mat);
        //xlogI(LOG_TAG, "===============> %d %d", output_mat->GetDims()[0], output_mat->GetDims()[1]);
    }

    return this->getName2Score(output_mats, score_threshold);
}

