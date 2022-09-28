#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include "Hungarian.h"
//#include <float.h>
//#define IMG_H 360
//#define IMG_W 640
//#define IMG_HW_ADD (IMG_H + IMG_W)
#define MAX_KF_NUM 32
#define MAX_ID_NUM (1<<31)
#define SNUM 8
#define MNUM 4
//#define CostOddThreshold (IMG_HW_ADD/8)

class MeasureInfo {
public:
	float CX; // centerX of box
	float CY; // centerY of box
	float WX; // widthX of box
	float WY; // widthY of box
	std::vector<cv::Point2f> landmarks; //face landmarks
};
class RsltInfo {
public:
	cv::Rect rect;
	int id;
};
//-----------------------
class cKF {
public:
	cKF(void) :KF(SNUM, MNUM) {};
	cv::KalmanFilter KF;
	cv::Mat matZ;
	int lifeCnt = 0;
	int mboxSeqNum;
	int ID;
	void init(void);
};

//-----------------------
class ckfManager {
public:
	int StrongNum = 1;
	int DisappearFrmCntAllowed = 15;
	float CostOddThreshold = 1.1;
	float CostNotGoodThreshold = 0.5; // 
	float MatchNotGoodRatioThresh = 0.3; // > thresh: require GT box on next img
	std::vector<cv::Rect2f> boxIn;
	std::vector<RsltInfo> rsltOut;
	bool run(const cv::Mat& img);
private:
	void cKF_born(int mboxSeqNum);
	void cKF_lostDeal(int ckfSeqNum);
	void cKF_kill(int cKF_id);
	float costCalc(int idx_state_m, int idx_state_p);
	void separateNormal(bool measureIsShorter, std::set<int>& oddSet);
	void match(bool measureIsShorter);
	//void match_2(std::set<int>& lostSet_grow, std::set<int>& newSet, std::set<int>& lostSet);
	void match_wrapper(void);
	void pred(void);
	void correct(void);
	//int draw(cv::Mat& mat_image);
	void updateRslt(const cv::Mat& img);
private:
	std::vector <MeasureInfo> state_m;
	int endIdNum=0, startIdNum=0;
	cKF* p_cKF[MAX_KF_NUM] = { nullptr };
	std::set<int> aliveSet, newSet, lostSet;
	std::set<std::pair<int, int>> normalSet;
	//int imgH, imgW;
	int DEAD_NUM;
	float MatchNotGoodRatio=0;
};
