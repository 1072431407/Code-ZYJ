#include "track.h"
#include "math.h"
//#define SHOW_RSLT
#define INC_RATIO (1.0e6)
#define DEC_RATIO (1.0e-6)
#define Q_SCALAR (1e-6)
#define R_SCALAR (1e-3)
#define RectSize_RATIO 0.1
void cKF::init(void)
{
	KF.init(SNUM, MNUM);
	KF.transitionMatrix = (cv::Mat_<float>(SNUM, SNUM) <<
		1, 0, 0, 0, 1, 0, 0, 0,
		0, 1, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 0, 0, 0, RectSize_RATIO, 0,
		0, 0, 0, 1, 0, 0, 0, RectSize_RATIO,
		0, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 1);
	setIdentity(KF.measurementMatrix);
	setIdentity(KF.processNoiseCov, cv::Scalar::all(Q_SCALAR));
	setIdentity(KF.measurementNoiseCov, cv::Scalar::all(R_SCALAR));
	setIdentity(KF.errorCovPost, cv::Scalar::all(0.5));
	randn(KF.statePost, cv::Scalar::all(0), cv::Scalar::all(0.1));
	matZ = cv::Mat::zeros(MNUM, 1, CV_32F);
}
//-----------------------
void ckfManager::cKF_born(int mboxSeqNum)
{
	for (int j = 0; j < MAX_KF_NUM; j++) {
		if (p_cKF[j] == nullptr) {
			p_cKF[j] = new cKF;
			p_cKF[j]->init();
			// KF initialization
			p_cKF[j]->matZ = (cv::Mat_<float>(MNUM, 1) << state_m[mboxSeqNum].CX, state_m[mboxSeqNum].CY, state_m[mboxSeqNum].WX, state_m[mboxSeqNum].WY);
			p_cKF[j]->mboxSeqNum = mboxSeqNum;
			// 
			p_cKF[j]->KF.predict();
			float costRslt = FLT_MAX;
			//int cnt = 0;
			while (costRslt > 0.1) {
				p_cKF[j]->KF.processNoiseCov *= INC_RATIO;
				p_cKF[j]->KF.measurementNoiseCov *= DEC_RATIO;
				p_cKF[j]->KF.correct(p_cKF[j]->matZ);
				setIdentity(p_cKF[j]->KF.processNoiseCov, cv::Scalar::all(Q_SCALAR));
				setIdentity(p_cKF[j]->KF.measurementNoiseCov, cv::Scalar::all(R_SCALAR));
				p_cKF[j]->KF.predict();

				costRslt = costCalc(mboxSeqNum, j);
				//cnt++;
			}
			//std::cout << "init cnt: " << cnt << std::endl;
			p_cKF[j]->ID = endIdNum;
			endIdNum = (endIdNum + 1) % MAX_ID_NUM;
			aliveSet.insert(j);
			//std::cout << endIdNum << std::endl;
			
			break;
		}
	}
}
//void ckfManager::cKF_lostDeal(int ckfSeqNum)
//{
//	if ((p_cKF[ckfSeqNum]->lifeCnt >= STRONG_NUM) && (p_cKF[ckfSeqNum]->lifeCnt < DEAD_NUM)) {
//		p_cKF[ckfSeqNum]->matZ = p_cKF[ckfSeqNum]->KF.measurementMatrix * p_cKF[ckfSeqNum]->state_p;
//		p_cKF[ckfSeqNum]->lifeCnt++;
//		p_cKF[ckfSeqNum]->mboxSeqNum = -1;
//		setIdentity(p_cKF[ckfSeqNum]->KF.measurementNoiseCov, cv::Scalar::all(9999));
//	}
//	else {
//		cKF_kill(ckfSeqNum);
//	}
//}
void ckfManager::cKF_kill(int cKF_id)
{
	aliveSet.erase(cKF_id);
	delete p_cKF[cKF_id];
	p_cKF[cKF_id] = nullptr;
}
float ckfManager::costCalc(int idx_state_m, int idx_state_p)
{
	cv::Rect2f mRect = { state_m[idx_state_m].CX - state_m[idx_state_m].WX / 2,
		state_m[idx_state_m].CY - state_m[idx_state_m].WY / 2,
		state_m[idx_state_m].WX,state_m[idx_state_m].WY };
	cv::Rect2f pRect = { p_cKF[idx_state_p]->KF.statePre.at<float>(0) - p_cKF[idx_state_p]->KF.statePre.at<float>(2) / 2,
		p_cKF[idx_state_p]->KF.statePre.at<float>(1) - p_cKF[idx_state_p]->KF.statePre.at<float>(3) / 2,
		p_cKF[idx_state_p]->KF.statePre.at<float>(2),p_cKF[idx_state_p]->KF.statePre.at<float>(3) };
	float area_m = mRect.area();
	float area_p = pRect.area();
	float area_inter = 0;
	float inter_w = std::min(mRect.br().x, pRect.br().x)-std::max(mRect.tl().x, pRect.tl().x);
	float inter_h = std::min(mRect.br().y, pRect.br().y) - std::max(mRect.tl().y, pRect.tl().y);
	if (inter_w > 0 && inter_h > 0)
		area_inter = inter_w * inter_h;
	float iou = area_inter / (area_m + area_p - area_inter);
	cv::Point2f outer_tl = { std::min(mRect.x, pRect.x),std::min(mRect.y, pRect.y) };
	cv::Point2f outer_br = { std::max(mRect.br().x, pRect.br().x) ,std::max(mRect.br().y, pRect.br().y)};
	cv::Point2f vDiagLine = outer_br - outer_tl;
	float diagLine_sqr = vDiagLine.x * vDiagLine.x + vDiagLine.y * vDiagLine.y;
	cv::Point2f ct_mRect = { mRect.x + mRect.width / 2,mRect.y + mRect.height / 2 };
	cv::Point2f ct_pRect = { pRect.x + pRect.width / 2,pRect.y + pRect.height / 2 };
	cv::Point2f vCt = ct_pRect - ct_mRect;
	float ctLine_sqr = vCt.x * vCt.x + vCt.y * vCt.y;
	float costRslt = 1 - iou + ctLine_sqr / diagLine_sqr;
	//std::cout << "costRslt: " << costRslt << std::endl;
	return costRslt;
}

void ckfManager::separateNormal(bool measureIsShorter, std::set<int>& oddSet)
{
	int MatchNotGoodCnt = 0;
	float InCnt = 0;
	std::vector<int> shortInd,longInd;
	std::vector< std::vector<float> > costMatrix; //[ind_long,ind_short]
	std::vector<int> assignment; // [indLong][indShort]
	HungarianAlgorithm HungAlgo;
	float cost, costRowMin;
	int costRowMinIdx = -1;
	std::set<int>::iterator it;
	if (measureIsShorter) {
		InCnt = state_m.size();
		for (int i = 0; i < state_m.size(); i++) {
			std::vector<float> costM_row;
			costRowMin = CostOddThreshold;
			
			float test_costRowMin = FLT_MAX;
			for (it = aliveSet.begin(); it != aliveSet.end(); it++) {
				cost = costCalc(i, *it);
				costM_row.push_back(cost);
				if (cost < costRowMin) {
					costRowMin = cost;
					costRowMinIdx = *it;
				}
				if (cost < test_costRowMin)
					test_costRowMin = cost;
			}
			if (costRowMin >= CostOddThreshold) {
				oddSet.insert(i); // newSet
			}
			else {
				shortInd.push_back(i); // idxMeasure
				costMatrix.push_back(costM_row);
			}
		}
		// -------------------------------
		if (costMatrix.size()==0)
			return;
		lostSet = aliveSet;
		std::vector<int> vv(aliveSet.begin(),aliveSet.end());
		cost = HungAlgo.Solve(costMatrix, assignment); // assignment: [ind_alive][ind_m]
		for (int i = 0; i < assignment.size(); i++) { // i:ind_normTmp ,assignment[i]:ind of alive
			if (costMatrix[i][assignment[i]] >= CostOddThreshold) {
				oddSet.insert(shortInd[i]);
			}
			else {
				normalSet.insert(std::make_pair(vv[assignment[i]], shortInd[i]));
				lostSet.erase(vv[assignment[i]]);
				if (costMatrix[i][assignment[i]] >= CostNotGoodThreshold)
					MatchNotGoodCnt++;
			}
		}
	}
	else {
		InCnt = aliveSet.size();
		for (it = aliveSet.begin(); it != aliveSet.end(); it++) {
			std::vector<float> costM_row;
			costRowMin = CostOddThreshold;
			for (int i = 0; i < state_m.size(); i++) {
				cost = costCalc(i, *it);  // *it is kfIdx
				costM_row.push_back(cost);
				if (cost < costRowMin) {
					costRowMinIdx = i;
					costRowMin = cost;
				}

			}
			if (costRowMin >= CostOddThreshold) {
				oddSet.insert(*it); // lostSet
			}
			else {
				longInd.push_back(costRowMinIdx);
				shortInd.push_back(*it); // idxAlive
				costMatrix.push_back(costM_row);
			}
			//std::cout << "test_costRowMin: " << test_costRowMin << std::endl;
		}
		if (costMatrix.size() == 0)
			return;
		for (int i = 0; i < state_m.size(); i++) {
			newSet.insert(i);
		}
		cost = HungAlgo.Solve(costMatrix, assignment); // assignment: [ind_alive][ind_m]
		for (int i = 0; i < assignment.size(); i++) {
			if (costMatrix[i][assignment[i]] >= CostOddThreshold) {
				oddSet.insert(shortInd[i]); // oddSet: lostSet
			}
			else {
				normalSet.insert(std::make_pair(shortInd[i], assignment[i]));
				newSet.erase(assignment[i]);
				if (costMatrix[i][assignment[i]] >= CostNotGoodThreshold)
					MatchNotGoodCnt++;
			}
		}
	}
	MatchNotGoodRatio = MatchNotGoodCnt / InCnt;
}
// {norm,lost} --> index for alive
// {new} -> index for measure
void ckfManager::match(bool measureIsShorter)
{
	std::vector< std::vector<float> > costMatrix; //[ind_long,ind_short]
	std::vector<int> assignment;
	HungarianAlgorithm HungAlgo;
	float cost;
	if(measureIsShorter) {
		separateNormal(true, newSet);
	}
	else {
		separateNormal(false, lostSet);
	}
}

void ckfManager::match_wrapper(void)
{
	HungarianAlgorithm HungAlgo;
	std::vector< std::vector<float> > costMatrix;
	std::vector<int> assignment;
	std::set<int>::iterator it;
	float cost;
   	if ((state_m.size()) && (!aliveSet.size())) {
		for (int i = 0; i < state_m.size(); i++)
			newSet.insert(i);
	}
	else if((!state_m.size()) && (aliveSet.size())){
		lostSet = aliveSet;
	}
	else if((state_m.size()) && (aliveSet.size())) {
		if (state_m.size() < aliveSet.size())
			match(true);
		else 
			match(false);
	}
}

void ckfManager::pred(void)
{
	if (aliveSet.size()) {
		for (std::set<int>::iterator it = aliveSet.begin(); it != aliveSet.end(); it++) {
			p_cKF[*it]->KF.predict();
		}
	}
}
void ckfManager::correct(void)
{
	// newSet: born
// lostSet: < STRONG_NUM => kill
//          >= STRONG_NUM && < DEAD_NUM grow
//          >= DEAD_NUM kill
// normalSet: lifeCnt --> STRONG_NUM
//--- newSet ---//
	for (int n : newSet) {
		cKF_born(n);
	}
	//--- lostSet ---//
	for (int lost : lostSet) {
		if (p_cKF[lost]->lifeCnt < StrongNum-1 || p_cKF[lost]->lifeCnt >= DEAD_NUM) {
			cKF_kill(lost);
		}
		else // [STRONG_NUM,DEAD_NUM)
		{
			p_cKF[lost]->lifeCnt++;
			p_cKF[lost]->mboxSeqNum = -1;
		}
	}
	//--- {normalSet} correct ---// newSet is correct at born
	for (auto pr : normalSet) {
		int nm = pr.first;
		if (p_cKF[nm]->lifeCnt < StrongNum)
			p_cKF[nm]->lifeCnt++;
		else
			p_cKF[nm]->lifeCnt = StrongNum;
		p_cKF[nm]->mboxSeqNum = pr.second;
		p_cKF[nm]->matZ = (cv::Mat_<float>(MNUM, 1) << state_m[pr.second].CX, state_m[pr.second].CY, state_m[pr.second].WX, state_m[pr.second].WY);
		//p_cKF[nm]->KF.correct(p_cKF[nm]->matZ);
		p_cKF[nm]->KF.processNoiseCov *= INC_RATIO;
		p_cKF[nm]->KF.measurementNoiseCov *= DEC_RATIO;
		p_cKF[nm]->KF.correct(p_cKF[nm]->matZ);
		setIdentity(p_cKF[nm]->KF.processNoiseCov, cv::Scalar::all(Q_SCALAR));
		setIdentity(p_cKF[nm]->KF.measurementNoiseCov, cv::Scalar::all(R_SCALAR));
	}
	if(!newSet.empty())
		newSet.clear();
	if (!lostSet.empty())
		lostSet.clear();
	if (!normalSet.empty())
		normalSet.clear();
}
void ckfManager::updateRslt(const cv::Mat& mat_image)
{
	for (int a : aliveSet) {
		float pred_cx, pred_cy, pred_wx, pred_wy;
		pred_cx = p_cKF[a]->KF.statePost.at<float>(0);
		pred_cy = p_cKF[a]->KF.statePost.at<float>(1);
		pred_wx = p_cKF[a]->KF.statePost.at<float>(2);
		pred_wy = p_cKF[a]->KF.statePost.at<float>(3);
		cv::Rect predRect = { int(pred_cx - pred_wx / 2), int(pred_cy - pred_wy / 2), int(pred_wx), int(pred_wy) };
		rsltOut.push_back({ predRect,p_cKF[a]->ID });
#ifdef SHOW_RSLT
		cv::Rect predRect = { int(pred_cx - pred_wx / 2), int(pred_cy - pred_wy / 2), int(pred_wx), int(pred_wy) };
		cv::rectangle(mat_image, predRect, cv::Scalar(0, 255, 255), 2);
		cv::putText(mat_image, std::to_string(p_cKF[a]->ID), { int(pred_cx),int(pred_cy) }, 4, 1, cv::Scalar(0, 255, 0));
#endif
	}
}
/*
* return: true --> need send measure box next frame
*		  false --> good pred
*/ 
bool ckfManager::run(const cv::Mat& mat_image)
{
	DEAD_NUM = StrongNum + 1 + DisappearFrmCntAllowed;
	for (auto box : boxIn) {
		float cx = box.x + 0.5 * box.width;
		float cy = box.y + 0.5 * box.height;
		MeasureInfo m({ cx, cy, (float)box.width, (float)box.height });
		state_m.push_back(m);
	}
	boxIn.clear();
//	imgW = mat_image.cols;
//	imgH = mat_image.rows;
	rsltOut.clear();
	pred();
	
	match_wrapper();
//#ifdef SHOW_RSLT
//	draw(mat_image);
//#endif
	correct();
	updateRslt(mat_image);
	state_m.clear();
	return (MatchNotGoodRatio >= MatchNotGoodRatioThresh);
}
