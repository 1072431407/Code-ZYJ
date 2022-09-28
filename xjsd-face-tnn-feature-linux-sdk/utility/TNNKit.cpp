#include "TNNKit.h"
#include <fstream>
#include <sstream>
#include <xlog/XLog.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

static const char* TAG = "TNNKit";

TNNKit::TNNKit() {

}

TNNKit::~TNNKit() {

}

TNN_NS::Status TNNKit::init(const string& proto_content
	, const string& model_content
	, const NetInputShape& net_input_shape
	, const TNN_NS::MatConvertParam& input_cvt_param
	, const string& network_cache_path) {
	m_net_input_shape = net_input_shape;
	m_input_cvt_param = input_cvt_param;

	TNN_NS::ModelConfig config;
	config.model_type = TNN_NS::MODEL_TYPE_TNN;
	if (proto_content.empty()) {
		return TNN_NS::Status(TNN_NS::TNNERR_MODEL_ERR, "proto content invaild!");
	}
	else {
		config.params.push_back(proto_content);
	}

	if (model_content.empty()) {
		return TNN_NS::Status(TNN_NS::TNNERR_MODEL_ERR, "model content invaild!");
	}
	else {
		config.params.push_back(model_content);
	}
	m_net = make_shared<TNN_NS::TNN>();
	TNN_NS::Status status = m_net->Init(config);
	if (status != TNN_NS::TNN_OK) {
		return status;
	}
	TNN_NS::InputShapesMap shapeMap;
	TNN_NS::NetworkConfig network_config;
	network_config.library_path = { "" };
	TNN_NS::DeviceType dt;
#ifdef SYS_ANDROID
#if 1
	dt = TNN_NS::DEVICE_OPENCL;
	network_config.device_type = dt;
	m_instance = m_net->CreateInst(network_config, status, shapeMap);
	if (status != TNN_NS::TNN_OK || !m_instance) {
		// 如果出现GPU加载失败，切换到CPU
		dt = TNN_NS::DEVICE_ARM;
		network_config.device_type = dt;
		m_instance = m_net->CreateInst(network_config, status, shapeMap);
		if (status != TNN_NS::TNN_OK) {
			return status;
		}
	}
#else
	dt = TNN_NS::DEVICE_ARM;
	network_config.device_type = dt;
	m_instance = m_net->CreateInst(network_config, status, shapeMap);
	if (status != TNN_NS::TNN_OK) {
		return status;
	}
#endif

#else
	dt = TNN_NS::DEVICE_X86;
	network_config.device_type = dt;
	network_config.cache_path = network_cache_path;
	m_instance = m_net->CreateInst(network_config, status, shapeMap);
	if (status != TNN_NS::TNN_OK) {
		return status;
	}
#endif
	return TNN_NS::TNN_OK;
}

TNN_NS::Status TNNKit::forward(const cv::Mat& img) {
	auto time1 = std::chrono::high_resolution_clock::now();
	TNN_NS::Status status;
	if (img.empty()) {
		return TNN_NS::Status(TNN_NS::TNNERR_INVALID_INPUT, "error! input image empty");
	}
	//TODO:不同的模型使用不同的数据格式，在调用测完成需要的颜色空间
	//bgr转rgb颜色
	//cv::Mat data_img = img.clone();
	//cv::cvtColor(data_img, data_img, cv::COLOR_BGR2RGB);

	int image_w = img.cols;
	int image_h = img.rows;
	int channel = img.channels();
	TNN_NS::MatType mt = getMatType(channel);
	if (mt == TNN_NS::INVALID) {
		return TNN_NS::Status(TNN_NS::TNNERR_INVALID_INPUT, "error! unsuport input mat format!");
	}

	TNN_NS::DeviceType dt;
#ifdef SYS_ANDROID
	dt = TNN_NS::DEVICE_ARM;
#else
	dt = TNN_NS::DEVICE_NAIVE;
#endif
	TNN_NS::DimsVector image_dims = { 1, channel, image_h, image_w };
	// 模型输入
	TNN_NS::DimsVector target_dims = { 1, m_net_input_shape.channel, m_net_input_shape.height, m_net_input_shape.width };
	std::shared_ptr<TNN_NS::Mat> input_mat = std::make_shared<TNN_NS::Mat>(dt, mt, image_dims, img.data);
	std::shared_ptr<TNN_NS::Mat> resize_mat = std::make_shared<TNN_NS::Mat>(dt, mt, target_dims);
	// OPENCL需要设置queue
	void* command_queue = nullptr;
	status = m_instance->GetCommandQueue(&command_queue);
	if (status != TNN_NS::TNN_OK) {
		return status;
	}
	// 转换大小
	m_resize_param.type = TNN_NS::InterpType::INTERP_TYPE_NEAREST;
	m_resize_param.scale_w = float(m_net_input_shape.width) / float(image_w);
	m_resize_param.scale_h = float(m_net_input_shape.height) / float(image_h);
	status = TNN_NS::MatUtils::Resize(*input_mat, *resize_mat, m_resize_param, command_queue);
	if (status != TNN_NS::TNN_OK) {
		return status;
	}

	// 输入数据
	status = m_instance->SetInputMat(resize_mat, m_input_cvt_param);
	if (status != TNN_NS::TNN_OK) {
		return status;
	}
	auto time2 = std::chrono::high_resolution_clock::now();
	//% (PreprocessorDefinitions)
	status =  m_instance->Forward();
	auto time3 = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> preprocess_cost = time2 - time1;	// 毫秒
	std::chrono::duration<double, std::milli> forward_cost = time3 - time2;		// 毫秒
	xlogI(TAG, "preprocess cost %.2f millisecond", preprocess_cost.count());
	xlogI(TAG, "forward cost %.2f millisecond", forward_cost.count());
	input_mat.reset();
	resize_mat.reset();
	return status;
}

TNN_NS::Status TNNKit::forward(const std::unordered_map<std::string, cv::Mat>& name2imgs) {

	auto time1 = std::chrono::high_resolution_clock::now();
	TNN_NS::Status status;
	const auto& data_img = name2imgs.begin()->second;
	int image_w = data_img.cols;
	int image_h = data_img.rows;
	shared_ptr<TNN_NS::Mat> input_mat;
	int channel = data_img.channels();
	TNN_NS::MatType mt = getMatType(channel);
	if (mt == TNN_NS::INVALID) {
		return TNN_NS::Status(TNN_NS::TNNERR_INVALID_INPUT, "error! unsuport input mat format!");
	}

	TNN_NS::DeviceType dt;
#ifdef SYS_ANDROID
	dt = TNN_NS::DEVICE_ARM;
#else
	dt = TNN_NS::DEVICE_NAIVE;
#endif
	TNN_NS::DimsVector image_dims = { 1, channel, image_h, image_w };
	input_mat = make_shared<TNN_NS::Mat>(dt, mt, image_dims, data_img.data);
	// 模型输入
	TNN_NS::DimsVector target_dims = { 1, m_net_input_shape.channel, m_net_input_shape.height, m_net_input_shape.width };
	auto resize_mat = make_shared<TNN_NS::Mat>(dt, mt, target_dims);
	// OPENCL需要设置queue
	void* command_queue = nullptr;
	status = m_instance->GetCommandQueue(&command_queue);
	if (status != TNN_NS::TNN_OK) {
		return status;
	}
	// 转换大小
	//m_resize_param.type = TNN_NS::InterpType::INTERP_TYPE_NEAREST;
	m_resize_param.scale_w = float(m_net_input_shape.width) / float(image_w);
	m_resize_param.scale_h = float(m_net_input_shape.height) / float(image_h);
	status = TNN_NS::MatUtils::Resize(*input_mat, *resize_mat, m_resize_param, command_queue);
	if (status != TNN_NS::TNN_OK) {
		return status;
	}

	// 输入数据
	for (const auto& name2img : name2imgs) {
		status = m_instance->SetInputMat(resize_mat, m_input_cvt_param, name2img.first);
		if (status != TNN_NS::TNN_OK) {
			return status;
		}
	}
	auto time2 = std::chrono::high_resolution_clock::now();
	//% (PreprocessorDefinitions)
	status =  m_instance->Forward();
	auto time3 = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> preprocess_cost = time2 - time1;	// 毫秒
	std::chrono::duration<double, std::milli> forward_cost = time3 - time2;	// 毫秒
	xlogI(TAG, "preprocess cost %f millisecond", preprocess_cost.count());
	xlogI(TAG, "forward cost %f millisecond", forward_cost.count());
	return status;
}

TNN_NS::Status TNNKit::getOutputMat(shared_ptr<TNN_NS::Mat>& output_mat, const TNN_NS::MatConvertParam& convert_param, const string& layout_name) {
	TNN_NS::DeviceType dt;
#ifdef SYS_ANDROID
	dt = TNN_NS::DEVICE_ARM;
#else
	dt = TNN_NS::DEVICE_NAIVE;
#endif
		return m_instance->GetOutputMat(output_mat, TNN_NS::MatConvertParam(), layout_name, dt);
}

string TNNKit::readProtoContent(const string& proto_path) {
	ifstream proto_stream(proto_path);
	if (!proto_stream.is_open() || !proto_stream.good()) {
		return "";
	}
	else {
		return string((istreambuf_iterator<char>(proto_stream)), istreambuf_iterator<char>());
	}
}

string TNNKit::readModelContent(const string& model_path) {
	ifstream model_stream(model_path, ios::binary);
	if (!model_stream.is_open() || !model_stream.good()) {
		return "";
	}
	else {
		stringstream model_content;
		model_content << model_stream.rdbuf();
		return model_content.str();
	}
}


TNN_NS::ResizeParam TNNKit::getResizeParam() {
	return m_resize_param;
}

TNN_NS::MatType TNNKit::getMatType(int channel) {
	switch (channel)
	{
	case 1:
		return TNN_NS::MatType::NGRAY;
	case 3:
		return TNN_NS::MatType::N8UC3;
	case 4:
		return TNN_NS::MatType::N8UC4;
	default:
		return TNN_NS::MatType::INVALID;
	}
}

