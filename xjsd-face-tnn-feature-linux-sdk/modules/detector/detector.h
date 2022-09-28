#ifndef Detector_H
#define Detector_H

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <TNNKit.h>

#include <tnn/core/blob.h>
#include <tnn/utils/blob_converter.h>
#include <tnn/utils/mat_utils.h>


typedef struct RecogInfo {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
    std::vector<cv::Point2f> landmarks;
} RecogInfo;


class Detector {
public:
	Detector();
	virtual ~Detector();

	virtual int init(const std::string& proto_content, const std::string& model_content) final;
    virtual std::vector<RecogInfo> detect(const cv::Mat& img, float threshold, float nms_threshold) final;

protected:
    virtual void generateDetectResult(const std::vector<std::shared_ptr<TNN_NS::Mat>>& outputs, 
                                      std::vector<RecogInfo> &detecs,
                                      float threshold) = 0;
    virtual void resizeBox2Ori(std::vector<RecogInfo>& boxes, int net_image_width, int net_image_height) = 0;

private:
    virtual void nms(std::vector<RecogInfo> &input_boxes, float nms_thresh) final;
    virtual void diouNms(std::vector<RecogInfo> &input_boxes, float nms_thresh) final;

protected:
    TNNKit* m_tnnKit = nullptr;
	TNNKit::NetInputShape* net_input_shape_ptr = nullptr;
	TNN_NS::MatConvertParam* input_cvt_param_ptr = nullptr;
    std::vector<std::string> output_layer_names;
    std::vector<std::string> label_names;
    int net_input_width;
    int net_input_height;

};


#endif //Detector_H
