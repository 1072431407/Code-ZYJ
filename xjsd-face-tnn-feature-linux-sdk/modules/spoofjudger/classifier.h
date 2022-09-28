#ifndef Classifier_H
#define Classifier_H

#include <string>
#include <vector>
#include <unordered_map>
#include <opencv2/core.hpp>
#include <TNNKit.h>


class Classifier {
public:
	Classifier();
	virtual ~Classifier();

	virtual int init(const std::string& proto_path, 
					 const std::string& model_path, 
					 const TNNKit::NetInputShape& net_input_shape, 
					 const TNN_NS::MatConvertParam& input_cvt_param) final;

    virtual std::unordered_map<std::string, float> predict(const cv::Mat& img) final;
    virtual std::unordered_map<std::string, float> predict(const std::unordered_map<std::string, cv::Mat>& name2imgs, float conf_threshold) final;

protected:
    virtual std::unordered_map<std::string, float> getName2Score(const std::vector<std::shared_ptr<TNN_NS::Mat>>& outputs, float score_threshold) = 0;

protected:
    TNNKit* m_tnnKit = nullptr;
    std::vector<std::string> input_layer_names;
    std::vector<std::string> output_layer_names;
    std::vector<std::string> label_names;
    int net_input_width = 0;
    int net_input_height = 0;
    float* score_threshold = nullptr;
    bool convert_flag = false;
};

#endif //Classifier_H
