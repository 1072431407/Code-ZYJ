#include "Yolov5.h"
#include "omp.h"
#include <fstream>
#include <xlog/XLog.h>


static const char* LOG_TAG = "Yolov5";


Yolov5::Yolov5() {
}

Yolov5::~Yolov5() {

}

bool Yolov5::init(const std::string& proto_content, const std::string& model_content) {
	TNNKit::NetInputShape net_input_shape = { 640,448,3 };
	TNN_NS::MatConvertParam input_cvt_param;
	input_cvt_param.scale = { 1.0 / 255, 1.0 / 255, 1.0 / 255, 0.0 };
	input_cvt_param.bias = { 0.0, 0.0, 0.0, 0.0 };

	TNN_NS::Status status = m_tnnKit.init(proto_content, model_content, net_input_shape, input_cvt_param);
	if (status != TNN_NS::TNN_OK) {
		xlogE(LOG_TAG, "TNN init failed!%s(%d)", status.description().c_str(), (int)status);
        return true;
    }
    else {
        return false;
    }
}

std::vector<BoxInfo> Yolov5::detect(const cv::Mat& img, float threshold, float nms_threshold) {

    TNN_NS::Status status = m_tnnKit.forward(img);
    if (status != TNN_NS::TNN_OK) {
        xlogE(LOG_TAG, "instance.Forward Error: %s", status.description().c_str());
    }

    // 获取数据
    std::vector<std::shared_ptr<TNN_NS::Mat>> output_mats;
    for (const YoloLayerData &layerData : Yolov5::layers) {
        std::shared_ptr<TNN_NS::Mat> output_mat;
        TNN_NS::MatConvertParam param;
        status = m_tnnKit.getOutputMat(output_mat, param, layerData.name);
        if (status != TNN_NS::TNN_OK) {
            xlogE(LOG_TAG, "instance.GetOutputMat Error: %s", status.description().c_str());
        }
        output_mats.push_back(output_mat);
        xlogI(LOG_TAG, "===============> %d %d %d %d", output_mat->GetDims()[0], output_mat->GetDims()[1], output_mat->GetDims()[2], output_mat->GetDims()[3]);
    }

    
    std::vector<BoxInfo> results;
    generateDetectResult(output_mats, results, threshold, nms_threshold);
    
    TNN_NS::ResizeParam rp = m_tnnKit.getResizeParam();
    for (BoxInfo &boxInfo : results) {
        boxInfo.x1 = boxInfo.x1 / rp.scale_w;
        boxInfo.x2 = boxInfo.x2 / rp.scale_w;
        boxInfo.y1 = boxInfo.y1 / rp.scale_h;
        boxInfo.y2 = boxInfo.y2 / rp.scale_h;
    }
    xlogI(LOG_TAG, "object size:%ld", results.size());
    return results;
}

void Yolov5::generateDetectResult(const std::vector<std::shared_ptr<TNN_NS::Mat>>& outputs
    , std::vector<BoxInfo> &detecs
    , float threshold
    , float nms_threshold) {

    int blob_index = 0;
    int num_anchor = Yolov5::layers[0].anchors.size(); // 3
    int detect_dim = 0;  // 85
    for (auto &output:outputs) {
        auto dim = output->GetDims();

        detect_dim = dim[3] / num_anchor;
        if (dim[3] != num_anchor * detect_dim) {
            xlogE(LOG_TAG, "Invalid detect output, the size of last dimension is: %d\n", dim[3]);
            return;
        }
        auto *data = static_cast<float *>(output->GetData());

        int num_potential_detecs = dim[1] * dim[2] * num_anchor;
        for (int i = 0; i < num_potential_detecs; ++i) {
            float x = data[i * detect_dim + 0];
            float y = data[i * detect_dim + 1];
            float width = data[i * detect_dim + 2];
            float height = data[i * detect_dim + 3];

            float objectness = data[i * detect_dim + 4];
            if (objectness < threshold) {
                continue;
            }

            // 计算坐标
            x = (float) (x * 2 - 0.5 + ((i / num_anchor) % dim[2])) * Yolov5::layers[blob_index].stride;
            y = (float) (y * 2 - 0.5 + ((i / num_anchor) / dim[2]) % dim[1]) * Yolov5::layers[blob_index].stride;
            width = pow((width * 2), 2) * Yolov5::layers[blob_index].anchors[i % num_anchor].width;
            height = pow((height * 2), 2) * Yolov5::layers[blob_index].anchors[i % num_anchor].height;
            // 坐标格式转换
            float x1 = x - width / 2;
            float y1 = y - height / 2;
            float x2 = x + width / 2;
            float y2 = y + height / 2;
            
            auto conf_start = data + i * detect_dim + 5;
            auto conf_end = data + (i + 1) * detect_dim;
            auto max_conf_iter = std::max_element(conf_start, conf_end);
            int conf_idx = static_cast<int>(std::distance(conf_start, max_conf_iter));
            float score = (*max_conf_iter) * objectness;

            BoxInfo obj_info;
            obj_info.x1 = x1;
            obj_info.y1 = y1;
            obj_info.x2 = x2;
            obj_info.y2 = y2;
            obj_info.score = score;
            obj_info.label = conf_idx;
            detecs.push_back(obj_info);
        }
        blob_index += 1;
    }
    nms(detecs, nms_threshold);
}

void Yolov5::nms(std::vector<BoxInfo> &input_boxes, float nms_thresh) {
    std::sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
                   * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        for (int j = i + 1; j < int(input_boxes.size());) {
            float xx1 = std::max(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = std::max(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = std::min(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = std::min(input_boxes[i].y2, input_boxes[j].y2);
            float w = std::max(float(0), xx2 - xx1 + 1);
            float h = std::max(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= nms_thresh) {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            } else {
                j++;
            }
        }
    }
}
