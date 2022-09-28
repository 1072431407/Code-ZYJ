//#include <time.h>
#include <kfTrack/track.h>
#include <fstream>

//#define SHOW_BOX

const char* WHITESPACE = " \n\r\t";


std::string TrimLeft(const std::string& s, const char* t) {
	size_t startpos = s.find_first_not_of(t);
	return (startpos == std::string::npos) ? "" : s.substr(startpos);
}

std::string TrimRight(const std::string& s, const char* t) {
	size_t endpos = s.find_last_not_of(t);
	return (endpos == std::string::npos) ? "" : s.substr(0, endpos + 1);
}

std::string Trim(const std::string& s, const char* t) {
	return TrimRight(TrimLeft(s, t), t);
}

//
//void draw(cv::Mat& mat_image, std::vector <boxInfo>& rsltBall)
//{
//	float cx, cy, wx, wy;
//	int id;
//	for (int i = 0; i < rsltBall.size(); i++) {
//		cx = rsltBall[i].cx;
//		cy = rsltBall[i].cy;
//		wx = rsltBall[i].wx;
//		wy = rsltBall[i].wy;
//		id = rsltBall[i].id;
//		cv::Rect rsltRect = { int(cx - wx / 2),int(cy - wy / 2),int(wx),int(wy) };
//		cv::rectangle(mat_image, rsltRect, cv::Scalar(0, 255, 255), 2);
//		if (id != -1) {
//			cv::putText(mat_image, std::to_string(id), { int(cx),int(cy) }, 4, 1, cv::Scalar(0, 255, 0));
//		}
//
//	}
//}
// 
// 
//========================
#define JUMP_FRM_CNT (29)
//#define VIDEO 0//MovingFast
#define VIDEO 1// NearFace1
//#define VIDEO 2//NearFace2

int main(int argc,char** argv)
{
	auto start0 = std::chrono::high_resolution_clock::now();
	while (1)
	{
		auto end0 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> tm = end0 - start0;	// ∫¡√Î
		if (tm.count() > 2000)
			break;
	}

	float timeDif = 0;
#if VIDEO==0
	std::string imgPath = std::string(TEST_DATA_PATH) + "/kfTrack/fromPenTao/MovingPeople/video";
	std::string boxFile = imgPath+"/../box_dets.txt";
#elif VIDEO==1
	std::string imgPath = std::string(TEST_DATA_PATH)  + "/kfTrack/fromPenTao/NearFace/video_1";
	std::string boxFile = imgPath + "/box_dets_highScore.txt";
#elif VIDEO==2
	std::string imgPath = std::string(TEST_DATA_PATH) + "/kfTrack/fromPenTao/NearFace/video_2";
	std::string boxFile = imgPath + "/box_dets_highScore.txt";
#endif
	std::ifstream ifs;
	ifs.open(boxFile, std::ios::in);
	std::string line;
	
	cv::Mat img;// = cv::Mat::zeros(cv::Size(1000, 700), CV_8UC3);
#ifdef SHOW_BOX
	while (getline(ifs, line)) {
		std::stringstream ss(line);
		while (ss) {
			ss >> p_LeftTop.x >> p_LeftTop.y >> p_RightBottom.x >> p_RightBottom.y;
			cv::Rect rect = { p_LeftTop.x,p_LeftTop.y,p_RightBottom.x - p_LeftTop.x,p_RightBottom.y - p_LeftTop.y };
			cv::rectangle(img, rect, cv::Scalar(50, 255, 50), 2);
		}
		cv::imshow("image", img);
		cv::waitKey(100);
		img = cv::Mat::zeros(cv::Size(1000, 700), CV_8UC3);
	}
#else
	ckfManager kfM;
	int scnt = 0;
	int boxCnt = 0;
	int imgCnt = 0;
#if VIDEO==0
	int jmpCnt = 5341; //;//5236;
#else
	int jmpCnt = 0;
#endif
	int jmpFrmCnt = 0;
	while (getline(ifs, line)) {
		line = Trim(line, WHITESPACE);
		if (jmpCnt > 0) {
			jmpCnt--;
			continue;
		}
	
		if (scnt == 0) {
			// read img name
#if VIDEO==0
			std::string imgName = imgPath + "/" + line;
#else
			std::string imgName = imgPath + "/imgs/" + line;
			if (argc > 1) {
				imgName = argv[1];
			}
			std::cout << "imgName:(" << imgName << ")" << std::endl;
#endif
			img = cv::imread(imgName, cv::IMREAD_COLOR);
			imgCnt++;
			scnt++;
			std::cout << "img:" << img.cols << "," << img.rows << std::endl;

			if (jmpFrmCnt < JUMP_FRM_CNT)
				jmpFrmCnt++;
			else
				jmpFrmCnt = 0;
			continue;
		}
		if (scnt == 1) {
			// read box_num
			boxCnt = std::stoi(line);
			if (boxCnt > 0)
				scnt++;
			else {
				scnt = 0;
			}
			continue;
		}
		std::stringstream ss(line);
		while (!ss.eof() && !ss.fail()) {
			cv::Point2f p_LeftTop, p_RightBottom;
			//float a, b, c, d;
			//ss >> a >> b >> c >> d;
			float simRatio;
			ss >> p_LeftTop.x >> p_LeftTop.y >> p_RightBottom.x >> p_RightBottom.y >> simRatio;
			cv::Rect rect = { int(p_LeftTop.x),int(p_LeftTop.y),int(p_RightBottom.x - p_LeftTop.x),int(p_RightBottom.y - p_LeftTop.y) };
			//if (jmpFrmCnt == 1) {
			//	cv::rectangle(img, rect, cv::Scalar(0, 0, 255), 6);
			//}
			//else {
			//	cv::rectangle(img, rect, cv::Scalar(0, 0, 0), 6);
			//}
			//cv::putText(img, "time: " + std::to_string(timeDif).substr(0,5) + " ms", {int(img.cols * 0.5), img.rows - 70}, 4, 1, cv::Scalar(0, 255, 0));

			float cX = (p_LeftTop.x + p_RightBottom.x) / 2;
			float cY = (p_LeftTop.y + p_RightBottom.y) / 2;
			float wX = (p_RightBottom.x - p_LeftTop.x+1);
			float wY = (p_RightBottom.y - p_LeftTop.y+1);
			//stateInfo sM = { cX,cY,wX,wY };
			//kfM.state_m.push_back(sM);
			//if (jmpFrmCnt == 1) {
			//	stateInfo sM = { cX,cY,wX,wY };
			//	kfM.state_m.push_back(sM);
			//}
		}
		if (boxCnt) {
			boxCnt--;
			if (boxCnt)
				continue;
		}
		scnt = 0;
		/*if (jmpFrmCnt !=1) {
			cv::imshow("image", img);
			cv::waitKey(30);
			continue;
		}*/
		std::cout << "imgCnt: " << imgCnt << std::endl;
		if (imgCnt == 152)
			int a = 0;
		auto start = std::chrono::high_resolution_clock::now();
 		bool ret = kfM.run(img);
		if (ret)
			jmpFrmCnt = 0;
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> tm = end - start;	// ∫¡√Î
		std::cout << "kfM.run cost : " << tm.count() << "ms" << std::endl;
		timeDif = tm.count();
		//cv::putText(img, "time: "+std::to_string(timeDif)+ "ms", { img.rows-50,int(img.cols*0.7) }, 4, 1, cv::Scalar(0, 255, 0));
		//draw(img, kfM.rsltBall);
		//cv::imshow("image", img);
		//cv::waitKey(100);
		//cv::waitKey(10);
		//img = cv::Mat::zeros(cv::Size(1000, 700), CV_8UC3);
		/*cv::imshow("image", img);
		cv::waitKey(100);
		img = cv::Mat::zeros(cv::Size(1000, 700), CV_8UC3);*/

	}
#endif
	return 0;
}
