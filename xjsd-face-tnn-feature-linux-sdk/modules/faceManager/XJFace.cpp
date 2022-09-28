#include "XJFaceImpl.h"

XJFace::XJFace():
	ptr(new XJFaceImpl())
{

}

XJFace::~XJFace() {

}


int XJFace::init(const string& detector_proto
	, const string& detector_model
	, const string& recognizer_proto
	, const string& recognizer_model
	, const string& spoofjudger_proto
	, const string& spoofjudger_model) {
	if (ptr) {
		return ptr->init(detector_proto
			, detector_model
			, recognizer_proto
			, recognizer_model
			, spoofjudger_proto
			, spoofjudger_model);
	}
	return -1000;
}


void XJFace::setTrackDetectInterval(int intervalMS) {
	if (ptr) {
		ptr->setTrackDetectInterval(intervalMS);
	}
}

void XJFace::setLogLevel(int level) {
	if (ptr) {
		return ptr->setLogLevel(level);
	}
}


string XJFace::getVersionName() {
	if (ptr) {
		return ptr->getVersionName();
	}
	return "";
}

string XJFace::getDetectorModelVersion() {
	if (ptr) {
		return ptr->getDetectorModelVersion();
	}
	return "";
}


string XJFace::getRecognizerModelVersion() {
	if (ptr) {
		return ptr->getRecognizerModelVersion();
	}
	return "";
}

string XJFace::getSpoofJudgerModelVersion() {
	if (ptr) {
		return ptr->getSpoofJudgerModelVersion();
	}
	return "";
}

int XJFace::detect(void* data, int width, int height,int rotation, FaceImageType image_type, vector<FaceBoxInfo>& boxs, bool is_tracking_mode) {
	if (ptr) {
		return ptr->detect(data,  width,  height, rotation,  image_type, boxs, is_tracking_mode);
	}
	return -1000;
}

int  XJFace::extrackFeature(int trackId, vector<float>& features) {
	if (ptr) {
		return ptr->extrackFeature(trackId, features);
	}
	return -1000;
}

int  XJFace::extrackFeature(void* data, int width, int height,int rotation, FaceImageType image_type, vector<float>& features) {
	if (ptr) {
		return ptr->extrackFeature(data, width, height, rotation, image_type, features);
	}
	return -1000;
}


int  XJFace::antiSpoofing(int trackId, float& score) {
	if (ptr) {
		return ptr->antiSpoofing(trackId, score);
	}
	return -1000;
}

int  XJFace::antiSpoofing(void* data, int width, int height,int rotation, FaceImageType image_type, float& score) {
	if (ptr) {
		return ptr->antiSpoofing(data, width, height, rotation, image_type, score);
	}
	return -1000;
}


float  XJFace::faceCompare(const vector<float>& feature1, const vector<float>& feature2) {
	if (ptr) {
		return ptr->faceCompare(feature1, feature2);
	}
	return -1000;
}
