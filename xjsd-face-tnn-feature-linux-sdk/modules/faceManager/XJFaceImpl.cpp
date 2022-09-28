#include "XJFaceImpl.h"
#include <alignment/alignment.h>
#include <xlog/XLog.h>
#include <FileUtil.h>

#define  RECORD_RAW_DATA					//是否缓存原始数据，用于算法debug	
#define LAPLACIAN_STD_THRESHOLD	15			//拉普拉斯标准差通过阈值

static const char* TAG = "XJFaceImpl";


XJFaceImpl::XJFaceImpl():
	m_detect_interval(50)
	,m_running(false)
	, m_sleeping(false)
	, m_waiting(true)
{
	m_detect_thread = std::thread(std::mem_fn(&XJFaceImpl::detectTask), this);
	m_detect_thread.detach();
}

XJFaceImpl::~XJFaceImpl()
{
	m_running = false;
	m_sleeping = false;
	m_waiting = false;
	m_sleep_cv.notify_all();
	m_wait_cv.notify_all();
	m_task_mtx.lock();
	m_task_mtx.unlock();
}

int XJFaceImpl::init(const string& detector_proto, const string& detector_model, const string& recognizer_proto, const string& recognizer_model, const string& spoofjudger_proto, const string& spoofjudger_model) {
	int ret;
	xlogI(TAG, "face init");
	xlogI(TAG, "version %s", getVersionName().c_str());
	ret = m_detector.init(detector_proto, detector_model);
	if (ret) {
		return ret;
	}
	ret = m_recognizer.init(recognizer_proto, recognizer_model);
	if (ret) {
		return ret;
	}
	//TNNKit::NetInputShape spoofjudger_input_shape = { 320, 320, 3 };
	TNNKit::NetInputShape spoofjudger_input_shape = { 256, 256, 3 };
	TNN_NS::MatConvertParam spoofjudger_input_cvt_param;
	spoofjudger_input_cvt_param.scale = { 1.0 / 128.0, 1.0 / 128.0, 1.0 / 128.0, 0.0 };
	spoofjudger_input_cvt_param.bias = { -127.5 / 128.0, -127.5 / 128.0, -127.5 / 128.0, 0.0 };
	ret = m_spoofjudger.init(spoofjudger_proto, spoofjudger_model, spoofjudger_input_shape, spoofjudger_input_cvt_param);
	if (ret) {
		return ret;
	}
	return ret;
}


string XJFaceImpl::getVersionName() {
	return "v0.9";
}


string XJFaceImpl::getDetectorModelVersion() {
	return "v0.25";
}

string XJFaceImpl::getRecognizerModelVersion() {
	return "v0.6";
}

string XJFaceImpl::getSpoofJudgerModelVersion() {
	return "v0.6";
}

void XJFaceImpl::setLogLevel(int level) {
	tinykit::XLog::getInstance()->setLogLevel(level);
}

void XJFaceImpl::setTrackDetectInterval(int intervalMS) {
	m_detect_interval = intervalMS;
}

int XJFaceImpl::generateImage(cv::Mat& dst, void* data, int width, int height,int rotation, XJFace::FaceImageType image_type) {
	if (!data) {
		return -1;
	}
	switch (image_type)
	{
	case XJFace::IMAGE_TYPE_BGR:
		dst = cv::Mat(height, width, CV_8UC3, data);
		break;
	case XJFace::IMAGE_TYPE_BGRA:
		dst = cv::Mat(height, width, CV_8UC4, data);
		break;
	case XJFace::IMAGE_TYPE_ARGB:
		dst = cv::Mat(height, width, CV_8UC4, data);
		break;
	case XJFace::IMAGE_TYPE_NV12: {
		cv::Mat yuv_mat(height * 3 / 2, width, CV_8UC1, data);
		cvtColor(yuv_mat, dst, cv::COLOR_YUV2BGR_NV12);
	}break;
	case XJFace::IMAGE_TYPE_NV21: {
		cv::Mat yuv_mat(height * 3 / 2, width, CV_8UC1, data);
		cvtColor(yuv_mat, dst, cv::COLOR_YUV2BGR_NV21);
	}break;
	case XJFace::IMAGE_TYPE_I420: {
		cv::Mat yuv_mat(height * 3 / 2, width, CV_8UC1, data);
		cvtColor(yuv_mat, dst, cv::COLOR_YUV2BGR_I420);
	}break;
	default:
		return -2;
	}
	if (!dst.empty()) {
		switch (rotation)
		{
		case -270:
		case 90:
			cv::transpose(dst, dst);
			flip(dst, dst, 1);
			break;
		case -180:
		case 180:
			cv::flip(dst, dst, 0);
			cv::flip(dst, dst, 1);
			break;
		case -90:
		case 270:
			cv::transpose(dst, dst);
			flip(dst, dst, 0);
			break;
		default:
			break;
		}
	}
	return 0;
}

cv::Rect2f XJFaceImpl::faceRectTocvRect(const XJFace::FaceRect& rect) {
	return cv::Rect2f(rect.x, rect.y, rect.width, rect.height);
}

vector<cv::Point2f> XJFaceImpl::facePointsTocvPoints(const vector<XJFace::FacePoint>& points) {
	vector<cv::Point2f> results;
	for (const XJFace::FacePoint& p : points)
	{
		results.emplace_back(cv::Point2f(p.x,p.y));
	}
	return results;
}

bool XJFaceImpl::findBoxImageByTrackId(int trackId, cv::Mat& dst, float output_scale) {
	//查找trackId对应的检测框
	XJFace::FaceBoxInfo fbox;
	m_last_face_boxs_mtx.lock();	
	if (m_last_face_boxs.size() > 0) {
		auto it = std::find_if(m_last_face_boxs.begin(), m_last_face_boxs.end(), [trackId](const XJFace::FaceBoxInfo& box) {
			return box.trackId == trackId;
		});
		if (it != m_last_face_boxs.end()) {
			fbox = *it;
		}
	}
	m_last_face_boxs_mtx.unlock();
	//image roi
	if (fbox.trackId == trackId) {
		int img_w = m_prepare_img.cols;
		int img_h = m_prepare_img.rows;
		std::vector<cv::Point2f> landmarks;
		unique_lock<mutex> lck(m_prepare_mtx);
		//查找重合检测框,匹配landmarks
		if (m_detect_boxs.size() > 0) {
			auto it = std::find_if(m_detect_boxs.begin(), m_detect_boxs.end(), [&fbox](const RecogInfo& dbox)->bool {
				cv::Rect2f r1(dbox.x1, dbox.y1, dbox.x2 - dbox.x1, dbox.y2 - dbox.y1);
				cv::Rect2f r2(fbox.rect.x, fbox.rect.y, fbox.rect.width, fbox.rect.height);
				cv::Rect2f box_and = r1 & r2;
				cv::Rect2f box_or = r1 | r2;
				float iou = box_and.area() * 1.0f / box_or.area();
				return iou > 0.5f;
			});
			if (it != m_detect_boxs.end()) {
				landmarks = it->landmarks;
			}
		}
		if (isLandMarksVaild(landmarks)) {
			//匹配到landmarks
			//face alignment
			Alignment align;
			//还原landmarks成绝对坐标
			std::for_each(landmarks.begin(), landmarks.end(), [&img_w, &img_h](cv::Point2f& p) {
				p.x *= img_w;
				p.y *= img_h;
			});
			int ret = align.align(m_prepare_img, landmarks, dst);
			if (ret) {
				xlogE(TAG, "face align error!ret=%d", ret);
				return false;
			}
			if (1.0 != output_scale) {
				XJFace::FaceRect find_rect;
				find_rect = fbox.rect;
				find_rect.x -= find_rect.width * (output_scale-1.f)/2.f;
				find_rect.y -= find_rect.height * (output_scale-1.f)/2.f;
				find_rect.width *= output_scale;
				find_rect.height *= output_scale;
				cv::Rect spoof_judger_box(find_rect.x * img_w, find_rect.y * img_h, find_rect.width * img_w, find_rect.height * img_h);
				//防止roi越界
				spoof_judger_box = spoof_judger_box & cv::Rect(0, 0, img_w, img_h);
				dst = m_prepare_img(spoof_judger_box);
				return true;
			}
		}
	}
	return false;
}

double XJFaceImpl::laplacianStdValue(const cv::Mat& img) {
	double val = 0;
	if (!img.empty()) {
		cv::Mat gray;
		cv::Mat lap;
		cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
		cv::Laplacian(gray, lap, CV_64F);
		cv::Scalar     mean;
		cv::Scalar     dev;
		cv::meanStdDev(lap, mean, dev);
		val = dev.val[0];
	}
	return val;
}

bool XJFaceImpl::isLandMarksVaild(const vector<cv::Point2f>& points) {
	if (points.size() != 5) {
		return false;
	}
	else {
		for (const cv::Point2f& p : points) {
			if (std::min(p.x, p.y) < 0 || std::max(p.x, p.y) > 1) {
				return false;
			}
		}
		return true;
	}
}


cv::Rect2f  XJFaceImpl::getLandMarksBundingBox(const vector<cv::Point2f>& points) {
		cv::Rect2f box;
		float left = 1;
		float top = 1;
		float right = 0;
		float bottom = 0;
		for (const cv::Point2f& p : points) {
			//x
			if (p.x < left) {
				left = p.x;
			}
			else if(p.x > right) {
				right = p.x;
			}
			//y
			if (p.y < top) {
				top = p.y;
			}
			else if (p.y > bottom) {
				bottom = p.y;
			}
		}
		box.x = left;
		box.y = top;
		box.width = right - left;
		box.height = bottom - top;
		return box;
}


int XJFaceImpl::detect(void* data, int width, int height,int rotation, XJFace::FaceImageType image_type, vector<XJFace::FaceBoxInfo>& face_boxs, bool is_tracking_mode) {
	cv::Mat img;
	generateImage(img,data, width, height, rotation, image_type);
	return detect(img, face_boxs,is_tracking_mode);
}


int XJFaceImpl::detect(const cv::Mat& img, vector<XJFace::FaceBoxInfo>& face_boxs, bool is_track_mode) {
	if (img.empty()) {
		return -2000;
	}
	auto time1 = std::chrono::high_resolution_clock::now();
	face_boxs.clear();
	if (is_track_mode) {
		//跟踪模式
		int img_w = img.cols;
		int img_h = img.rows;
		if (m_correction_mtx.try_lock()) {
			//开始跟踪
			auto time1 = std::chrono::high_resolution_clock::now();
			m_tracker.run(img);
			auto time2 = std::chrono::high_resolution_clock::now();
			//获取跟踪结果
			for (const RsltInfo& tbox : m_tracker.rsltOut) {
				//收集跟踪框
				XJFace::FaceBoxInfo face;
				//TODO:跟踪模块先还原成实际rect，等待后面跟踪器修改成归一化坐标
				cv::Rect2f fr(tbox.rect.x * 1.0f / img_w, tbox.rect.y * 1.0f / img_h, tbox.rect.width * 1.0f / img_w, tbox.rect.height * 1.0f / img_h);
				face.rect = XJFace::FaceRect(fr.x, fr.y, fr.width, fr.height);
				face.trackId = tbox.id;
				face_boxs.emplace_back(face);
			}
			//保存跟踪人脸信息
			m_last_face_boxs_mtx.lock();
			m_last_face_boxs = face_boxs;
			m_last_face_boxs_mtx.unlock();
			m_correction_mtx.unlock();
			std::chrono::duration<double, std::milli> track_cost = time2 - time1;
			xlogD(TAG, "track cost time:%f millisecond", track_cost.count());
		}
		else
		{
			//跟踪校准中，直接返回上一帧人脸框
			m_last_face_boxs_mtx.lock();
			face_boxs = m_last_face_boxs;
			m_last_face_boxs_mtx.unlock();
		}
		if (m_waiting) {
			img.copyTo(m_detect_img);
			m_wait_cv.notify_all();
		}
	}
	else {
		//直接检测模式
		img.copyTo(m_detect_img);
		std::vector<RecogInfo> detect_infos = m_detector.detect(m_detect_img, m_detector_threshold, m_detector_nms);
		static int direct_track_id = 0;
		for (const RecogInfo& rinfo : detect_infos) {
			direct_track_id++;
			//////face box///////
			XJFace::FaceBoxInfo face;
			face.trackId = direct_track_id;
			face.rect = XJFace::FaceRect(rinfo.x1, rinfo.y1, rinfo.x2 - rinfo.x1, rinfo.y2 - rinfo.y1);
			std::for_each(rinfo.landmarks.begin(), rinfo.landmarks.end(), [&face](const cv::Point2f& p) {
				face.landmarks.emplace_back(XJFace::FacePoint(p.x,p.y));
			});
			face_boxs.emplace_back(face);
		}
		m_last_face_boxs_mtx.lock();
		m_last_face_boxs = face_boxs;
		m_last_face_boxs_mtx.unlock();
	}
	auto time2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> extrack_cost = time2 - time1;
	xlogD(TAG, "detect cost time:%f millisecond", extrack_cost.count());
	return 0;
}


int XJFaceImpl::extrackFeature(void* data, int width, int height,int rotation, XJFace::FaceImageType image_type, vector<float>& features) {
	cv::Mat img;
	generateImage(img, data, width, height, rotation, image_type);
	return extrackFeature(img,features);
}

int XJFaceImpl::extrackFeature(const cv::Mat& img, vector<float>& features) {
	unique_lock<mutex> lck(m_recognizer_mtx);
	 auto time1 = std::chrono::high_resolution_clock::now();
	 if (img.empty()) {
		 xlogE(TAG, "extrackFeature error! img empty");
		 return -2000;
	 }
	 int ret = m_recognizer.extract(img, features);
	 auto time2 = std::chrono::high_resolution_clock::now();
	 std::chrono::duration<double, std::milli> extrack_cost = time2 - time1;
	 xlogD(TAG, "extrackFeature cost time:%f millisecond", extrack_cost.count());
	 if (ret != TNN_NS::TNN_OK) {
		 xlogE(TAG, "extrackFeature error! ret=%d",ret);
	 }
	 return ret;
}


int XJFaceImpl::extrackFeature(int trackId, vector<float>& features) {
	Mat img;
	int ret = -1;
	if (findBoxImageByTrackId(trackId, img)) {
		ret = extrackFeature(img,features);
#ifdef RECORD_RAW_DATA 
		if (tinykit::FileUtil::isExists(RECORD_DATA_PATH)) {
			int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			stringstream ss;
			ss << RECORD_DATA_PATH;
			ss << "/";
			ss << "extrack-";
			ss << now;
			ss << "-";
			ss << trackId;
			ss << ".png";
			string path = ss.str();
			cv::imwrite(path, img);
			xlogD(TAG, "save raw image %s", path.c_str());
		}
#endif
	}
	return ret;
}

float XJFaceImpl::faceCompare(const vector<float>& feature1, const vector<float>& feature2) {
	return m_recognizer.featureContrast(feature1, feature2);
}

int XJFaceImpl::antiSpoofing(void* data, int width, int height,int rotation, XJFace::FaceImageType image_type, float& score) {
	cv::Mat img;
	generateImage(img, data, width, height, rotation, image_type);
	return antiSpoofing(img, score);
}

int XJFaceImpl::antiSpoofing(const cv::Mat& img, float& score) {
	unique_lock<mutex> lck(m_spoofjudger_mtx);
	int ret = -1;
	if (img.empty()) {
		return -2000;
	}
	auto time1 = std::chrono::high_resolution_clock::now();
	std::unordered_map<std::string, float> name2score = m_spoofjudger.predict(img);
	auto time2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> cost = time2 - time1;
	xlogD(TAG, "antiSpoofing cost time:%f millisecond", cost.count());
	auto it = name2score.find("person");
	if (it != name2score.end()) {
		score = it->second;
		ret = 0;
	}
#ifdef RECORD_RAW_DATA 
	if (tinykit::FileUtil::isExists(RECORD_DATA_PATH)) {
		int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		stringstream ss;
		ss << RECORD_DATA_PATH;
		ss << "/";
		ss << "antiSpoofing-";
		ss << now;
		ss << "-";
		ss << score;
		ss << ".png";
		string path = ss.str();
		cv::imwrite(path, img);
		xlogD(TAG, "save raw image %s", path.c_str());
	}
#endif
	return ret;
}

int XJFaceImpl::antiSpoofing(int trackId, float& score) {
	Mat img;
	if (findBoxImageByTrackId(trackId, img, 1.5)) {
		return antiSpoofing(img, score);
	}
	return -1;
}

void XJFaceImpl::detectTask() {
	unique_lock<mutex> lck(m_task_mtx);
	mutex sleep_mtx;
	unique_lock<mutex> sleep_lck(sleep_mtx);
	mutex wait_mtx;
	unique_lock<mutex> wait_lck(wait_mtx);
	m_running = true;
	while (m_running)
	{
		m_sleeping = true;
		m_sleep_cv.wait_for(sleep_lck, std::chrono::milliseconds(m_detect_interval));
		m_sleeping = false;
		if (m_running) {
			m_waiting = true;
			m_wait_cv.wait(wait_lck);
			m_waiting = false;
			if (!m_detect_img.empty()) {
				int img_w = m_detect_img.cols;
				int img_h = m_detect_img.rows;
				std::vector<RecogInfo> recogInfos = m_detector.detect(m_detect_img, m_detector_threshold, m_detector_nms);
				if (recogInfos.size() > 0) {
					//检测到人脸
					const RecogInfo& rinfo = recogInfos.front();
					//判断landmark是否送检有效
					if (isLandMarksVaild(rinfo.landmarks)) {
						//判断人脸图片清晰度
						cv::Rect2f rb = getLandMarksBundingBox(rinfo.landmarks);
						cv::Rect face_rect(rb.x * img_w, rb.y * img_h, rb.width * img_w, rb.height * img_h);
						face_rect = face_rect & cv::Rect(0, 0, img_w, img_h);
						cv::Mat face_img = m_detect_img(face_rect);
						auto time1 = std::chrono::high_resolution_clock::now();
						double lapstd = laplacianStdValue(face_img);
						auto time2 = std::chrono::high_resolution_clock::now();
						std::chrono::duration<double, std::milli> laplacian_std_cost = time2 - time1;
						xlogD(TAG, "laplacianStdValue %f,cost time:%f millisecond", lapstd, laplacian_std_cost.count());
						m_laplacian_std_values.push_back(lapstd);
						if (m_laplacian_std_values.size() > 10) {
							m_laplacian_std_values.pop_front();
						}
						float mean = 0;
						std::for_each(m_laplacian_std_values.begin(), m_laplacian_std_values.end(), [&mean](float v) {
							mean += v;
						});
						mean /= m_laplacian_std_values.size();
						if (lapstd > mean/2) {
							//图片清晰度通过阈值，保存到备选
							m_prepare_mtx.lock();
							m_detect_boxs = recogInfos;
							m_detect_img.copyTo(m_prepare_img);
							m_prepare_mtx.unlock();
						}
					}
					
				}
				//推送检测框到跟踪器
				for (const RecogInfo& rbox : recogInfos) {
					//TODO:跟踪模块先还原成实际rect，等待后面跟踪器修改成归一化坐标
					cv::Rect2f r(rbox.x1 * img_w, rbox.y1 * img_h, (rbox.x2 - rbox.x1) * img_w, (rbox.y2 - rbox.y1) * img_h);
					m_tracker.boxIn.push_back(r);
				}
				//跟踪校准
				m_correction_mtx.lock();
				auto time1 = std::chrono::high_resolution_clock::now();
				m_tracker.run(m_detect_img);
				auto time2 = std::chrono::high_resolution_clock::now();
				m_correction_mtx.unlock();
				std::chrono::duration<double, std::milli> track_cost = time2 - time1;
				xlogD(TAG, "detectTask track cost time:%f millisecond", track_cost.count());
			}
		}
	}
}

