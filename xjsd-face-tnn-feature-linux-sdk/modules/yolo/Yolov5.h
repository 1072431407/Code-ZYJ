#ifndef Yolov5_H
#define Yolov5_H

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <TNNKit.h>

namespace yolocv {
    typedef struct {
        int width;
        int height;
    } YoloSize;
}

typedef struct {
    std::string name;
    int stride;
    std::vector<yolocv::YoloSize> anchors;
} YoloLayerData;

typedef struct BoxInfo {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
} BoxInfo;


class Yolov5 {
public:
	Yolov5();

	~Yolov5();

	bool init(const std::string& proto_content, const std::string& model_content);

    std::vector<BoxInfo> detect(const cv::Mat& img, float threshold, float nms_threshold);

    void generateDetectResult(const std::vector<std::shared_ptr<TNN_NS::Mat>>& outputs, std::vector<BoxInfo> &detecs,
                              float threshold, float nms_threshold);

    void nms(std::vector<BoxInfo> &input_boxes, float nms_thresh);

    std::vector<std::string> labels{"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
                                    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
                                    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
                                    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
                                    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
                                    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
                                    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
                                    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
                                    "hair drier", "toothbrush"};
private:


    TNNKit m_tnnKit;

    std::vector<YoloLayerData> layers{
            {"output", 32, {{116, 90}, {156, 198}, {373, 326}}},
            {"413", 16, {{30,  61}, {62,  45},  {59,  119}}},
            {"431", 8,  {{10,  13}, {16,  30},  {33,  23}}},
    };
};

#endif //Yolov5S_H
