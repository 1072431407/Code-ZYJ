//
// Created by chengdu on 2022/8/18.
//
#include <cmath>
#include <cstring>
#include <numeric>
#include <xlog/XLog.h>
#include "ssan_r_spoof_judger.h"
// #include "sample_timer.h"


constexpr char* LOG_TAG = "SSAN_SpoofJudger";

SsanRSpoofJudger::SsanRSpoofJudger() {
	this->label_names = { "person", "spoof" };
	this->output_layer_names = { "output0" };
	this->input_layer_names = { "input0", "input1" };
	this->score_threshold = new float(0.6f);
	this->convert_flag = true;
}

SsanRSpoofJudger::SsanRSpoofJudger(const std::vector<std::string>& label_names) {
	this->label_names = label_names;
	this->input_layer_names = { "input0", "input1" };
	this->score_threshold = new float(0.6f);
	this->convert_flag = true;
}

SsanRSpoofJudger::SsanRSpoofJudger(const std::vector<std::string>& label_names, const std::vector<std::string>& output_layer_names) {
	this->label_names = label_names;
	this->output_layer_names = output_layer_names;
	this->input_layer_names = { "input0", "input1" };
	this->score_threshold = new float(0.6f);
	this->convert_flag = true;
}

SsanRSpoofJudger::~SsanRSpoofJudger() {}

std::unordered_map<std::string, float> SsanRSpoofJudger::getName2Score(const std::vector<std::shared_ptr<TNN_NS::Mat>>& outputs, float score_threshold) {
	if (outputs.empty()) {
        xlogE(LOG_TAG, "Spoof Judger output size is 0");
        return {};
	}
	float* spoof_data = (float*)outputs[0]->GetData();
	std::vector<float> score_list(this->label_names.size());
	for (int i=0; i < this->label_names.size(); i++) {
		score_list[i] = spoof_data[i];
	}
	float softmax_sum = 0;
	std::for_each(score_list.begin(), score_list.end(), [&softmax_sum](float& score) {softmax_sum += exp(score);});
	std::for_each(score_list.begin(), score_list.end(), [&softmax_sum](float& score) {score = exp(score) / softmax_sum;});
	std::unordered_map<std::string, float> name2score;
	auto max_score_it = std::max_element(score_list.begin(), score_list.end());
	if (max_score_it != score_list.end() && *max_score_it > score_threshold) {
		for (int i=0; i < score_list.size(); i++) {
			name2score[this->label_names[i]] = score_list[i];
		}
	}
	return name2score;
}
