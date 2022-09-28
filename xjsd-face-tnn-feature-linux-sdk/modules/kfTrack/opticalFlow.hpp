#include <opencv2/opencv.hpp>
#include <fstream>
#include <vector>

cv::Rect toCvRect(const stateInfo& rectIn)
{
	//int x = round(rectIn.CX - rectIn.WX / 2) > 0 ? round(rectIn.CX - rectIn.WX / 2) : 0;
	//int y = round(rectIn.CY - rectIn.WY / 2) > 0 ? round(rectIn.CY - rectIn.WY / 2) : 0;
	//int w = round(rectIn.WX) > 0 ? round(rectIn.WX) : 0;
	//int h = round(rectIn.WY) > 0 ? round(rectIn.WY) : 0;
	int x = round(rectIn.CX - rectIn.WX / 2);
	int y = round(rectIn.CY - rectIn.WY / 2);
	int w = round(rectIn.WX);
	int h = round(rectIn.WY);
	return { x,y,w,h };
}
stateInfo rectExp(stateInfo rectIn, int imgH, int imgW, float ratioW = 1.2, float ratioH = 1.2)
{
	stateInfo rectNew;
	float wNew = rectIn.WX * ratioW;
	float hNew = rectIn.WY * ratioH;
	float xNew_leftTop = rectIn.CX - (wNew - 1) / 2;
	float yNew_leftTop = rectIn.CY - (hNew - 1) / 2;
	float xNew_rightBot = rectIn.CX + (wNew - 1) / 2;
	float yNew_rightBot = rectIn.CY + (hNew - 1) / 2;
	wNew = xNew_rightBot - xNew_leftTop + 1;
	hNew = yNew_rightBot - yNew_leftTop + 1;
	return { rectIn.CX,rectIn.CY ,wNew ,hNew };
}
void cropImg(const cv::Mat& img, const cv::Rect& rectCV, cv::Mat& imgSub)
{
//	std::cout << "x1-------------------" << std::endl;
	imgSub = cv::Mat::zeros(rectCV.height, rectCV.width, CV_8UC1);
	cv::Point2i shift_LeftTop = { std::max(0 - rectCV.x,0) ,std::max(0 - rectCV.y,0) };
	//cv::Point2i RightBot = {  - 1, rectCV.y + rectCV.height - 1 };
	cv::Point2i shift_RightTop = { std::min(img.cols - (rectCV.x + rectCV.width),0),std::min(img.rows - (rectCV.y + rectCV.height),0) };
	for (size_t x = 0; x < rectCV.width; x++) {
		for (size_t y = 0; y < rectCV.height; y++) {
//			std::cout << "x,y: " << x << ", " << y << std::endl;
			size_t xglb = x + rectCV.x;
			size_t yglb = y + rectCV.y;
			if (xglb >= 0 && xglb < img.cols && yglb >= 0 && yglb < img.rows)
				imgSub.at<uchar>(y, x) = img.at<uchar>(yglb, xglb);
			//else
			//	imgSub.at<uchar>(y, x) = 0;
		}
	}
	return;
}
void getFeature_onRect(cv::Mat& imgGray, const cv::Rect& rectCV, const cv::Rect& rectCVExp, cv::Mat& imgGraySubExp, std::vector<cv::Point2f>& fptsSubExp)
{
	cv::Mat imgGraySub;
	std::vector<cv::Point2f> fptsSub;
	cropImg(imgGray, rectCV, imgGraySub);

	int maxCorners = 60;
	double qualityLevel = 0.1;
	double minDist = (imgGraySub.rows + imgGraySub.cols) / (2 * maxCorners);//3.0;
	minDist = (minDist < 2) ? 2 : (minDist > 5 ? 5 : minDist);
	cv::goodFeaturesToTrack(imgGraySub, fptsSub, maxCorners, qualityLevel, minDist);

	imgGraySubExp = cv::Mat::zeros(rectCVExp.height, rectCVExp.width, CV_8UC1);
	size_t dx = (rectCVExp.width - rectCV.width) / 2;
	size_t dy = (rectCVExp.height - rectCV.height) / 2;
	for (size_t x = 0; x < rectCV.width; x++) {
		for (size_t y = 0; y < rectCV.height; y++) {
			imgGraySubExp.at<uchar>(y + dy, x + dx) = imgGraySub.at<uchar>(y, x);
		}
	}
	for (cv::Point2f f : fptsSub) {
		fptsSubExp.push_back({ f.x + dx ,f.y + dy });
	}
	return;
} 
/*
* [return]  -1: no valid err
*           0:  has valid err
*/
float calcErrAve(const std::vector<float>& err, std::vector<uchar> status, std::vector<int>& inlierInds_db, float factor = 2.0)
{
	float ave = 0, sigma = 0;
	float ave_ = 0;
	int cnt = 0;
	for (int i = 0; i < status.size(); i++) {
		if (status[i]) { // a valid data
			ave_ += err[i];
			cnt++;
		}
	}
	if (cnt == 0)
		return -1;
	else if (cnt == 1) {
		ave = ave_;
		sigma = 0;
		return 0;
	}
	ave_ /= cnt;
	for (int i = 0; i < status.size(); i++) {
		if (status[i]) { // a valid data
			sigma += (err[i] - ave_) * (err[i] - ave_);
		}
	}
	sigma = sqrt(sigma / (cnt - 1));
	int acnt = 0;
	for (int i = 0; i < status.size(); i++) {
		if (status[i]) { // a valid data
			//if ((err[i] >= ave_ - factor * sigma) && (err[i] <= ave_ + factor * sigma)) {
			if (err[i] <= ave_ + factor * sigma) {
				ave += err[i];
				acnt++;
	
				inlierInds_db.push_back(i);
			}
		}
	}
	ave = ave / acnt;
	return sigma;
	//std::cout << "(ave,sigma): " << ave << " ," << sigma << std::endl;
}
//#define DelBoundaryFeature
#define ShowSubImg
//#define UseInlier_vDif
/*
* return 0: success
*       -1: fail --> no enough feature points, or feature is bad
*/
int opticalFlow(cv::Mat& imgGray1, cv::Mat& imgGray2, stateInfo& rect1, stateInfo& rect2)
{
	std::vector<cv::Point2f> feature1, feature2;
	cv::Mat imgGraySub1Exp, imgGraySub2Exp;
	// crop img2
	std::cout << "m1-------------------" << std::endl;
	stateInfo rect2_exp = rectExp(rect1, imgGray2.rows, imgGray2.cols);
	std::cout << "m2-------------------" << std::endl;
	cropImg(imgGray2, toCvRect(rect2_exp), imgGraySub2Exp);
	std::cout << "m3-------------------" << std::endl;
	// crop img1
	getFeature_onRect(imgGray1, toCvRect(rect1), toCvRect(rect2_exp), imgGraySub1Exp, feature1);
	std::cout << "m4-------------------" << std::endl;
	// optical flow
	std::vector<uchar> status;
	std::vector<float> err;
	//calcOpticalFlowPyrLK(imgGraySub1Exp, imgGraySub2Exp, feature1, feature2, status, err, cv::Size(50, 50), 2);
	std::cout << "m5-------------------" << std::endl;
	calcOpticalFlowPyrLK(imgGraySub1Exp, imgGraySub2Exp, feature1, feature2, status, err, cv::Size(cv::min(imgGraySub2Exp.cols, 50), cv::min(imgGraySub2Exp.rows, 50)), 2);
	std::cout << "m6-------------------" << std::endl;
#ifdef DelBoundaryFeature
	//// delete boundary points
	for (int i = 0; i < feature1.size(); i++) {      
		cv::Point2f f1 = feature1[i];
		cv::Point2f f2 = feature2[i];
		if (round(f1.x) < 0 || round(f1.x) > (imgGraySub1Exp.cols - 1) || round(f1.y) < 0 || round(f1.y) > (imgGraySub1Exp.rows - 1) ||
			round(f2.x) < 0 || round(f2.x) > (imgGraySub1Exp.cols - 1) || round(f2.y) < 0 || round(f2.y) > (imgGraySub1Exp.rows - 1))
			if (status[i]) {
				status[i] = 0;
				std::cout << "--------------------------------err: " << err[i] << std::endl;
			}
	}
#endif
	int cntNeg = 0;
	float cntVld = 0;
	for (int i = 0; i < feature1.size(); i++) {
		cv::Point2f f1 = feature1[i];
		cv::Point2f f2 = feature2[i];
		if (status[i]) {
			if (round(f1.x) < 0 || round(f1.x) > (imgGraySub1Exp.cols - 1) || round(f1.y) < 0 || round(f1.y) > (imgGraySub1Exp.rows - 1) ||
				round(f2.x) < 0 || round(f2.x) > (imgGraySub1Exp.cols - 1) || round(f2.y) < 0 || round(f2.y) > (imgGraySub1Exp.rows - 1))
				if (status[i]) {
					cntNeg++;
				}
			cntVld++;
		}
	}
	bool badMatch = (cntNeg > 0) && (cntNeg / cntVld > 0.4);
	std::cout << "ratio: " << cntNeg / cntVld << std::endl;
	if (badMatch) {
		std::cout << "~~~~~~~~~~badMatch! " << std::endl;
		//return -1;
		float EdgeLen = 2;
		for (int i = 0; i < feature1.size(); i++) {
			cv::Point2f f1 = feature1[i];
			cv::Point2f f2 = feature2[i];
			if (round(f1.x) < EdgeLen || round(f1.x) > (imgGraySub1Exp.cols - 1-EdgeLen) || round(f1.y) < EdgeLen || round(f1.y) > (imgGraySub1Exp.rows - 1-EdgeLen) ||
				round(f2.x) < EdgeLen || round(f2.x) > (imgGraySub1Exp.cols - 1-EdgeLen) || round(f2.y) < EdgeLen || round(f2.y) > (imgGraySub1Exp.rows - 1-EdgeLen))
				if (status[i]) {
					status[i] = 0;
				}
		}
	}
	
	std::vector<int> inlierInds_db, inlierInds;
	float err_sigma;
	if (cntNeg / cntVld < 0.1)
		err_sigma = calcErrAve(err, status, inlierInds_db);
	else
		err_sigma = calcErrAve(err, status, inlierInds_db, 0);

	if (err_sigma > 15.0) {
		badMatch = true;
		std::cout << "badMatch! "<< std::endl;
	}
	std::cout <<"err_sigma: "<< err_sigma << std::endl;

	if (inlierInds_db.size() < 2) {
		rect2 = rect1;
		return -1;
	}
	std::vector<cv::Point2f> vDifs; // mht dist
	cv::Point2f vDifAve = { 0,0 };
	std::vector<float>Difs; // mht dist
	float DifAve=0, DifSigma=0;
	for (auto i : inlierInds_db) {
		//--- dx, dy
		cv::Point2f vDif = feature2[i] - feature1[i];
		vDifs.push_back(vDif);
		if (badMatch) {
			float mhtDist = fabs(vDif.x) + fabs(vDif.y);
			Difs.push_back(mhtDist);
			DifAve += mhtDist;
		}
		else
			vDifAve += vDif;
	}
	if (badMatch) {
		DifAve /= inlierInds_db.size();
		for (int i = 0; i < inlierInds_db.size(); i++) {
			DifSigma += (Difs[i] - DifAve) * (Difs[i] - DifAve);
		}
		DifSigma = sqrt(DifSigma / (inlierInds_db.size() - 1));
		float InlierFactor = 1.0;

		for (int i = 0; i < inlierInds_db.size(); i++) {
			if ((Difs[i] >= DifAve - 2*InlierFactor * DifSigma) && (Difs[i] <= DifAve + 0 * DifSigma)) {
				inlierInds.push_back(inlierInds_db[i]);
				vDifAve += vDifs[i];
			}
		}
		vDifAve = { vDifAve.x / inlierInds.size(),vDifAve.y / inlierInds.size() };
	}
	else
		vDifAve = { vDifAve.x / inlierInds_db.size(),vDifAve.y / inlierInds_db.size() };

	rect2 = { rect1.CX + vDifAve.x,rect1.CY + vDifAve.y,rect1.WX,rect1.WY }; // global rect coord

#ifdef ShowSubImg
	cv::Mat imgSub1Exp, imgSub2Exp;
	cv::cvtColor(imgGraySub1Exp, imgSub1Exp, cv::COLOR_GRAY2BGR);
	cv::cvtColor(imgGraySub2Exp, imgSub2Exp, cv::COLOR_GRAY2BGR);

	for (int i = 0; i < status.size(); i++) {
		if (status[i]) {
			////--------- show -------//
			cv::circle(imgSub1Exp, feature1[i], 1, cv::Scalar(0, 255, 0), 2);
			cv::circle(imgSub2Exp, feature2[i], 1, cv::Scalar(0, 255, 0), 2);
			////--------- show end -------//
		}
	}
	for (auto i : inlierInds_db) {
		////--------- show -------//
		cv::circle(imgSub1Exp, feature1[i], 1, cv::Scalar(0, 0, 255), 2);
		cv::circle(imgSub2Exp, feature2[i], 1, cv::Scalar(0, 0, 255), 2);
		cv::line(imgSub2Exp, feature1[i], feature2[i], cv::Scalar(255, 0, 0), 1);
		////--------- show end -------//
	}
	cv::Mat imgGraySub2;
	cropImg(imgGray2, toCvRect(rect2), imgGraySub2);
	cv::cvtColor(imgGraySub2, imgGraySub2, cv::COLOR_GRAY2BGR);
	cv::Mat imgGraySub12 = cv::Mat::zeros(imgSub1Exp.rows, 2*imgSub1Exp.cols+ imgGraySub2.cols, CV_8UC3);
	for (int h = 0; h < imgGraySub12.rows; h++) {
		for (int w = 0; w < imgGraySub12.cols; w++) {
			if (w < imgSub1Exp.cols)
				imgGraySub12.at<cv::Vec3b>(h, w) = imgSub1Exp.at<cv::Vec3b>(h, w);
			else if (w < 2 * imgSub1Exp.cols)
				imgGraySub12.at<cv::Vec3b>(h, w) = imgSub2Exp.at<cv::Vec3b>(h, w - imgSub1Exp.cols);
			else if (w < 2 * imgSub1Exp.cols + imgGraySub2.cols && h < imgGraySub2.rows)
				imgGraySub12.at<cv::Vec3b>(h, w) = imgGraySub2.at<cv::Vec3b>(h, w - 2 * imgSub1Exp.cols);
		}
	}
	
	//cv::imshow("i12", imgGraySub12);
	//cv::waitKey(0);
	//cv::destroyWindow("i12");
#endif
	if (badMatch)
		return -1;
	else
		return 0;
}