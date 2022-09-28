#pragma once

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <unordered_map>
//#include <tnn/core/nativelib.h>
#include <tnn/core/tnn.h>
#include <tnn/core/blob.h>
#include <tnn/utils/blob_converter.h>
#include <tnn/utils/mat_utils.h>
using namespace std;

/**
*@brief tnn工具类，方面tnn跨平台调用和部署
*/
class TNNKit {
public:
	struct NetInputShape
	{
		NetInputShape() {}
		NetInputShape(int w, int h, int c) : width(w), height(h), channel(c) {}
		int width;
		int height;
		int channel;
	};
public:
	TNNKit();
	~TNNKit();
	/**
	*加载模型，初始化TNN引擎
	*@param proto_content 文本proto文件数据
	*@param model_content 二进制模型文件数据
	*@net_input_shape 网络输入shape，对应图片宽高和channel
	*@input_cvt_param 网络输入转换参数，包含scale和bias
	*@network_cache_path 网络缓存路径，加上后能解决第一次预处理加载慢的问题
	*@return 返回状态，0为成功
	*/
	TNN_NS::Status init(const string& proto_content,
						const string& model_content,
						const NetInputShape& net_input_shape,
						const TNN_NS::MatConvertParam& input_cvt_param,
						const string& network_cache_path = "");

	/**
	*推理执行
	*@param img mat格式图片数据
	*@return 返回状态，0为成功
	*/
	TNN_NS::Status forward(const cv::Mat& img);

	TNN_NS::Status forward(const std::unordered_map<std::string, cv::Mat>& name2img);

	/**
	*获取推理输出层数据
	*@return 返回状态，0为成功
	*/
	TNN_NS::Status getOutputMat(shared_ptr<TNN_NS::Mat>& output_mat,
								const TNN_NS::MatConvertParam& convert_param = TNN_NS::MatConvertParam(),
								const string& layout_name = "");

	/**
	*获取输入图片到网络输入的resize参数，yolo系列算法需要根据anchor还原坐标在原始图片上的位置
	*@return resize参数
	*/
	TNN_NS::ResizeParam getResizeParam();

	/**
	*获取输入图片到网络输入的resize参数，yolo系列算法需要根据anchor还原坐标在原始图片上的位置
	*@return 返回resize参数
	*/

	/**
	*以文本方式读取tnnproto文件信息，并保存在string中
	*@return 文件内容
	*/
	static string readProtoContent(const string& proto_path);

	/**
	*以二进展方式读取tnnmodel文件信息，并保存在string中
	*@return 文件内容
	*/
	static string readModelContent(const string& model_path);
private:
	shared_ptr<TNN_NS::TNN> m_net;
	shared_ptr<TNN_NS::Instance> m_instance;
	NetInputShape m_net_input_shape;
	TNN_NS::MatConvertParam m_input_cvt_param;
	TNN_NS::ResizeParam m_resize_param;
	TNN_NS::MatType getMatType(int channel);
};

