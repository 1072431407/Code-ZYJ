#include "detector.h"
#include "omp.h"
#include <fstream>
#include <xlog/XLog.h>
#include <cmath>


constexpr char* LOG_TAG = "Detector";

Detector::Detector() {
    m_tnnKit = new TNNKit();
}

Detector::~Detector() {
    delete m_tnnKit;
}

int Detector::init(const std::string& proto_content, const std::string& model_content) {
    if (nullptr == this->net_input_shape_ptr) {
		xlogE(LOG_TAG, "TNN init has no net_input_shape");
        return -1;
    }
    if (nullptr == this->input_cvt_param_ptr) {
		xlogE(LOG_TAG, "TNN init has no input convert parameter");
        return -1;
    }
	TNN_NS::Status status = m_tnnKit->init(proto_content, model_content, *this->net_input_shape_ptr, *this->input_cvt_param_ptr);
	if (status != TNN_NS::TNN_OK) {
		xlogE(LOG_TAG, "TNN init failed %s(%d)", status.description().c_str(), (int)status);
	}
	return (int)status;
}

std::vector<RecogInfo> Detector::detect(const cv::Mat& img, float score_threshold, float nms_threshold) {
    TNN_NS::Status status = m_tnnKit->forward(img);
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
        //xlogI(LOG_TAG, "===============> %d %d %d", output_mat->GetDims()[0], output_mat->GetDims()[1], output_mat->GetDims()[2]);
    }

    std::vector<RecogInfo> results;
    this->generateDetectResult(output_mats, results, score_threshold);
    this->diouNms(results, nms_threshold);
    //this->nms(results, nms_threshold);
    
    //TNN_NS::ResizeParam rp = m_tnnKit->getResizeParam();
    //for (RecogInfo &boxInfo : results) {
    //    boxInfo.x1 /= rp.scale_w;
    //    boxInfo.x2 /= rp.scale_w;
    //    boxInfo.y1 /= rp.scale_h;
    //    boxInfo.y2 /= rp.scale_h;
    //    for (auto& landm : boxInfo.landmarks) {
    //        landm.x /= rp.scale_w;
    //        landm.y /= rp.scale_h;
    //    }
    //}
    //this->resizeBox2Ori(results, this->net_input_width, this->net_input_height);
    //xlogI(LOG_TAG, "object size:%ld", results.size());
    return results;
}

void Detector::nms(std::vector<RecogInfo> &input_boxes, float nms_thresh) {
    //box area desc sort 
    std::sort(input_boxes.begin(), input_boxes.end(), [](const RecogInfo& r1, const RecogInfo& r2) -> bool {
		float area1 = (r1.x2 - r1.x1) * (r1.y2 - r1.y1);
		float area2 = (r2.x2 - r2.x1) * (r2.y2 - r2.y1);
        return area1 > area2;
    });

  //  auto it1 = input_boxes.begin();
  //  while (it1 != input_boxes.end())
  //  {
  //      auto it2 = it1 + 1;
		//while (it2 != input_boxes.end())
		//{
		//	cv::Rect2f r1(it1->x1, it1->y1, it1->x2 - it1->x1, it1->y2 - it1->y1);
		//	cv::Rect2f r2(it2->x1, it2->y1, it2->x2 - it2->x1, it2->y2 - it2->y1);
		//	cv::Rect2f r_and = r1 & r2;
  //          float iou = r_and.area() / std::min(r1.area(), r2.area());
  //          if (iou > nms_thresh) {
		//		it2 = input_boxes.erase(it2);
  //          }
  //          else {
		//		it2++;
  //          }
		//}
  //      it1++;
  //  }
    //std::sort(input_boxes.begin(), input_boxes.end(), [](RecogInfo a, RecogInfo b) { return a.score > b.score; });
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

void Detector::diouNms(std::vector<RecogInfo> &input_boxes, float nms_thresh) {
    //std::sort(input_boxes.begin(), input_boxes.end(), [](RecogInfo a, RecogInfo b) { return a.score > b.score; });
    //box area desc sort 
    std::sort(input_boxes.begin(), input_boxes.end(), [](const RecogInfo& r1, const RecogInfo& r2) -> bool {
		float area1 = (r1.x2 - r1.x1) * (r1.y2 - r1.y1);
		float area2 = (r2.x2 - r2.x1) * (r2.y2 - r2.y1);
        return area1 > area2;
    });
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
            if (0.f != w && 0.f != h) {
                float c = w * w + h * h;
                float box1_center_x = (input_boxes[i].x1 + input_boxes[i].x2) / 2.f;
                float box1_center_y = (input_boxes[i].y1 + input_boxes[i].y2) / 2.f;
                float box2_center_x = (input_boxes[j].x1 + input_boxes[j].x2) / 2.f;
                float box2_center_y = (input_boxes[j].y1 + input_boxes[j].y2) / 2.f;
                float d = (box1_center_x - box2_center_x) * (box1_center_x - box2_center_x) + (box1_center_y - box2_center_y) * (box1_center_y - box2_center_y);
                float diou_term = std::pow(d / c, 0.6);
                ovr -= diou_term;
            }
            if (ovr >= nms_thresh) {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            } else {
                j++;
            }
        }
    }
}
