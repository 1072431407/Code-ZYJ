//
// Created by chengdu on 2022/8/18.
//
#ifndef SSAN_R_SPOOF_JUDGER_H
#define SSAN_R_SPOOF_JUDGER_H

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <TNNKit.h>

#include "classifier.h"


class SsanRSpoofJudger : public Classifier {
public:
	explicit SsanRSpoofJudger();
	explicit SsanRSpoofJudger(const std::vector<std::string>& label_names);
	explicit SsanRSpoofJudger(const std::vector<std::string>& label_names, const std::vector<std::string>& output_layer_names);
	virtual ~SsanRSpoofJudger();

protected:
	virtual std::unordered_map<std::string, float> getName2Score(const std::vector<std::shared_ptr<TNN_NS::Mat>>& outputs, float score_threshold) override;

//private:
//	float score_threshold;
};

#endif //SSAN_R_SPOOF_JUDGER_H
