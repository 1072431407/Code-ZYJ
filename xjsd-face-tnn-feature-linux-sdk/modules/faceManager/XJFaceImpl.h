#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>
#include <thread>
#include <functional>
#include <algorithm>
#include <atomic>
#include <opencv2/core.hpp>
#include "detector/retina_face_detector.h"
#include "kfTrack/track.h"
#include "recognizer/MobileFace.h"
#include "spoofjudger/ssan_r_spoof_judger.h"
#include <XJFace.h>
#include <condition_variable>

using namespace std;

class FACE_EXPORT XJFaceImpl
{
public:
	XJFaceImpl();
	virtual ~XJFaceImpl();

	/**
	*初始化
	* @param detector_proto 人脸检测模型proto文件
	* @param detector_model 人脸检测模型文件
	* @param recognizer_proto 人脸识别模型proto文件
	* @param recognizer_model 人脸识别模型文件
	* @param spoofjudger_proto 活体检测模型proto文件
	* @param spoofjudger_model 活体检测模型文件
	* @return 初始化状态码
	*/
	int init(const string& detector_proto
		, const string& detector_model
		, const string& recognizer_proto
		, const string& recognizer_model
		, const string& spoofjudger_proto
		, const string& spoofjudger_model);


	/**
	*获取人脸检测版本号
	*/
	string getVersionName();

	/**
	*获取检测器模型版本
	*/
	string getDetectorModelVersion();

	/**
	*获取识别器模型版本
	*/
	string getRecognizerModelVersion();

	/**
	*获取活检器模型版本
	*/
	string getSpoofJudgerModelVersion();

	/**
	*跟踪模式下设置检测间隔时间
	*/
	void setTrackDetectInterval(int intervalMS);


	/**
	*设置log输出等级
	*/
	void setLogLevel(int level);

	/**
	*人脸检测
	*/
	int detect(void* data, int width, int height,int rotation, XJFace::FaceImageType image_type, vector<XJFace::FaceBoxInfo>& face_boxs, bool is_tracking_mode = true);
	int detect(const cv::Mat& img, vector<XJFace::FaceBoxInfo>& face_boxs, bool is_tracking_mode = true);

	/**
	*人脸提特
	*/
	int extrackFeature(int trackId, vector<float>& features);
	int extrackFeature(void* data, int width, int height,int rotation, XJFace::FaceImageType image_type, vector<float>& features);
	int extrackFeature(const cv::Mat& img, vector<float>& features);

	/**
	*活体检测
	*/
	int antiSpoofing(int trackId, float& score);
	int antiSpoofing(void* data, int width, int height,int rotation, XJFace::FaceImageType image_type, float& score);
	int antiSpoofing(const cv::Mat& img, float& score);

	/**
	*人脸对比
	*/
	float faceCompare(const vector<float>& feature1, const vector<float>& feature2);
private:
	void detectTask();						//检测任务
	int generateImage(cv::Mat& dst,void* data, int width, int height,int rotation, XJFace::FaceImageType image_type);
	bool findBoxImageByTrackId(int trackId, cv::Mat& dst, float output_scale=1.0);
	cv::Rect2f faceRectTocvRect(const XJFace::FaceRect& rect);
	vector<cv::Point2f> facePointsTocvPoints(const vector<XJFace::FacePoint>& points);
	double laplacianStdValue(const cv::Mat& img);					//拉普拉斯变换方差值
	bool isLandMarksVaild(const vector<cv::Point2f>& points);		//判断landmark是否有效
	cv::Rect2f getLandMarksBundingBox(const vector<cv::Point2f>& points);	//获取landmarks围成的bunding box
private:
	atomic_bool m_running;					//主程序是否运行中
	atomic_bool m_sleeping;					//检测线程是否休眠中
	atomic_bool m_waiting;					//检测线程休眠结束
	thread m_detect_thread;					//检测线程句柄
	mutex m_correction_mtx;					//跟踪校准锁
	mutex m_last_face_boxs_mtx;				//人脸信息框锁
	mutex m_task_mtx;						//主线程锁
	std::condition_variable m_sleep_cv;		//休眠唤醒条件变量
	std::condition_variable m_wait_cv;		//检测等待条件变量
	vector<XJFace::FaceBoxInfo> m_last_face_boxs;	//最后一帧人脸框
	std::vector<RecogInfo> m_detect_boxs;			//人脸检测框
	std::list<float> m_laplacian_std_values;		//图片清晰度分值列表
	cv::Mat m_detect_img;							//检测图片
	cv::Mat m_prepare_img;							//通过检测的备选图片
	mutex m_prepare_mtx;							//备选图片锁
	int m_detect_interval;							//检测间隔时间
private:
	ckfManager m_tracker;					//跟踪器
	RetinaFaceDetector m_detector;			//检测器

	mutex m_recognizer_mtx;					//识别器锁
	MobileFace m_recognizer;				//识别器
	mutex m_spoofjudger_mtx;				//活检器锁
	SsanRSpoofJudger m_spoofjudger;			//活检器
	float m_detector_threshold = 0.8f;
	float m_detector_nms = 0.5f;
};
