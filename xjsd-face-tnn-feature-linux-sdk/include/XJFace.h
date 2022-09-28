#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
#define FACE_EXPORT   __declspec( dllexport )
#else
#define FACE_EXPORT
#endif

using namespace std;

class XJFaceImpl;
class FACE_EXPORT XJFace
{
public:
	struct FaceRect {
		float x = 0;
		float y = 0;
		float width = 0;
		float height = 0;
		FaceRect() = default;
		FaceRect(float x, float y, float w, float h) {
			this->x = x;
			this->y = y;
			this->width = w;
			this->height = h;
		}
	};
	struct FacePoint {
		float x = 0;
		float y = 0;
		FacePoint() = default;
		FacePoint(float x, float y) {
			this->x = x;
			this->y = y;
		}
	};

	struct FaceBoxInfo {
		int trackId = 0;				//人脸跟踪id，有新目标时id会增加
		float sore = 0;					//人脸跟踪分数(0-1)之间
		FaceRect rect;					//人脸框0-1归一化数据，使用的时候坐标需要乘以原始图片宽高
		vector<FacePoint> landmarks;	//一共5个点，10条数据，0-1归一化数据，使用的时候坐标需要乘以原始图片宽高
	};

	enum XJFaceLogLevel {
		XJFaceLogDebug = 0x01,
		XJFaceLogInfo = 0x02,
		XJFaceLogWarning = 0x04,
		XJFaceLogError = 0x08
	};

	enum FaceImageType
	{
		IMAGE_TYPE_UNKNOWN = 0
		, IMAGE_TYPE_BGR = 1				//opencv rgb image
		, IMAGE_TYPE_BGRA = 2				//opencv rgba image
		, IMAGE_TYPE_ARGB = 3				//bitmap argb image
		, IMAGE_TYPE_NV12 = 4				//android nv21 image
		, IMAGE_TYPE_NV21 = 5				//android nv21 image--yuv COLOR_YUV420sp2RGB==COLOR_YUV2RGB_NV21 --DEBUG_CAMERA_DATA_PREPROCESS = 0
		, IMAGE_TYPE_I420 = 6				//yuv i420 image
	};
public:
	XJFace();
	virtual ~XJFace();

	/**
	*初始化
	*/
	virtual int init(const string& detector_proto
		, const string& detector_model
		, const string& recognizer_proto
		, const string& recognizer_model
		, const string& spoofjudger_proto
		, const string& spoofjudger_model);
	
	/**
	*跟踪模式下设置检测间隔时间
	*/
	virtual void setTrackDetectInterval(int intervalMS);

	/**
	*设置log输出等级
	*/
	void setLogLevel(int level);

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
	*人脸检测
	*/
	virtual int detect(void* data, int width, int height,int rotation, FaceImageType image_type, vector<FaceBoxInfo>& boxs, bool is_tracking_mode = true);

	/**
	*人脸提特
	*/
	virtual int extrackFeature(int trackId, vector<float>& features);
	virtual int extrackFeature(void* data, int width, int height,int rotation, FaceImageType image_type, vector<float>& features);

	/**
	*活体检测
	*/
	virtual int antiSpoofing(int trackId, float& score);
	virtual int antiSpoofing(void* data, int width, int height,int rotation, FaceImageType image_type, float& score);

	/**
	*人脸对比
	*/
	virtual float faceCompare(const vector<float>& feature1, const vector<float>& feature2);

private:
	shared_ptr<XJFaceImpl> ptr;
};
