#ifndef RETINA_FACE_DETECTOR_H_
#define RETINA_FACE_DETECTOR_H_

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <TNNKit.h>
#include "detector.h"


class RetinaFaceDetector : public Detector {
public:
    explicit RetinaFaceDetector();
    virtual ~RetinaFaceDetector();

protected:
    virtual void generateDetectResult(const std::vector<std::shared_ptr<TNN_NS::Mat>>& outputs, 
                                      std::vector<RecogInfo>& detecs,
                                      float score_threshold) override;
    virtual void resizeBox2Ori(std::vector<RecogInfo>& boxes, int net_image_width, int net_image_height) override;

private:
	void generatePriors(int num_featuremap, int net_input_width, int net_input_height);

private:
    int num_anchors;
    const float center_variance = 0.1;
    const float size_variance = 0.2;
    const std::vector<std::vector<float>> min_boxes = {
            {16.0f,  32.0f},
            {64.0f,  128.0f},
            {256.0f, 512.0f}};
    const std::vector<float> strides = {8.0, 16.0, 32.0};
    std::vector<std::vector<float>> featuremap_size;
    std::vector<std::vector<float>> shrinkage_size;
    std::vector<std::vector<float>> priors;
    int input_width;
    int input_height;

};


#endif // RETINA_FACE_DETECTOR_H_
