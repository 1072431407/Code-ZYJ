#include "retina_face_detector.h"
#include "sample_timer.h"
#include <cmath>
#include <cstring>
#include <xlog/XLog.h>


#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))
constexpr char* TAG = "RetinaFaceDetector";

RetinaFaceDetector::RetinaFaceDetector() {
    this->output_layer_names = {"conf", "loc", "landms"};
    this->input_cvt_param_ptr = new TNN_NS::MatConvertParam();
	this->input_cvt_param_ptr->scale = { 1.0, 1.0, 1.0, 0.0 };
	this->input_cvt_param_ptr->bias = { -104, -117, -123, 0.0 };
    this->net_input_shape_ptr = new TNNKit::NetInputShape(320, 320, 3);
	this->net_input_width = this->net_input_shape_ptr->width;
	this->net_input_height = this->net_input_shape_ptr->height;
	int num_featuremap = 3;
	this->generatePriors(num_featuremap, this->net_input_width, this->net_input_height);
}

RetinaFaceDetector::~RetinaFaceDetector() {
    delete this->input_cvt_param_ptr;
    delete this->net_input_shape_ptr;
}

void RetinaFaceDetector::generatePriors(int num_featuremap, int net_input_width, int net_input_height) {
    auto w_h_list = {net_input_width, net_input_height};
    for (auto size : w_h_list) {
        std::vector<float> fm_item;
        for (float stride : this->strides) {
            fm_item.push_back(ceil(size / stride));
        }
        this->featuremap_size.push_back(fm_item);
    }
    for (auto size : w_h_list) {
        this->shrinkage_size.push_back(strides);
    }
    /* generate prior anchors */
    for (int index = 0; index < num_featuremap; index++) {
        float scale_w = net_input_width / shrinkage_size[0][index];
        float scale_h = net_input_height / shrinkage_size[1][index];
        for (int j = 0; j < featuremap_size[1][index]; j++) {
            for (int i = 0; i < featuremap_size[0][index]; i++) {
                float x_center = (i + 0.5) / scale_w;
                float y_center = (j + 0.5) / scale_h;

                for (float k : min_boxes[index]) {
                    float w = k / net_input_width;
                    float h = k / net_input_height;
                    this->priors.push_back({clip(x_center, 1), clip(y_center, 1), clip(w, 1), clip(h, 1)});
                }
            }
        }
    }
    this->num_anchors = this->priors.size();
    return;
}

/*
 * Generating Bbox from output blobs
 */
void RetinaFaceDetector::generateDetectResult(const std::vector<std::shared_ptr<TNN_NS::Mat>>& outputs,
                                              std::vector<RecogInfo>& detecs,
                                              float score_threshold) {
    float* conf_data = (float*)outputs[0]->GetData();
    float* loc_data  = (float*)outputs[1]->GetData();
    float* landms_data = (float*)outputs[2]->GetData();

    for (int i = 0; i < this->num_anchors; i++) {
        if (conf_data[i * 2 + 1] > score_threshold) {
            float x_center = loc_data[i * 4] * center_variance * priors[i][2] + priors[i][0];
            float y_center = loc_data[i * 4 + 1] * center_variance * priors[i][3] + priors[i][1];
            float w        = exp(loc_data[i * 4 + 2] * size_variance) * priors[i][2];
            float h        = exp(loc_data[i * 4 + 3] * size_variance) * priors[i][3];
            std::vector<cv::Point2f> landmarks;
            for (int k=0; k<5; k++) {
                float landms_x = priors[i][0] + landms_data[10*i+2*k] * center_variance * priors[i][2];
                float landms_y = priors[i][1] + landms_data[10*i+2*k+1] * center_variance * priors[i][3];
                landmarks.emplace_back(cv::Point2f(landms_x, landms_y));
            }

            RecogInfo rects;
            rects.x1    = clip(x_center - w / 2.0, 1);
            rects.y1    = clip(y_center - h / 2.0, 1);
            rects.x2    = clip(x_center + w / 2.0, 1);
            rects.y2    = clip(y_center + h / 2.0, 1);
            rects.score = clip(conf_data[i * 2 + 1], 1);
            rects.landmarks = landmarks;
            rects.label = 0;
            detecs.emplace_back(rects);
        }
    }
}

void RetinaFaceDetector::resizeBox2Ori(std::vector<RecogInfo>& boxes, int net_image_width, int net_image_height) {
	std::for_each(boxes.begin(), boxes.end(), [net_image_width, net_image_height](RecogInfo& box) {
					box.x1 *= net_image_width;
					box.y1 *= net_image_height;
					box.x2 *= net_image_width;
					box.y2 *= net_image_height;
					for (auto& landm : box.landmarks) {
						landm.x *= net_image_width;
						landm.y *= net_image_height;
					}
				 });
    return;
}
