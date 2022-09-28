#ifndef MOBILEFACE_H
#define MOBILEFACE_H

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <TNNKit.h>

class MobileFace {
public:
	MobileFace();
	virtual ~MobileFace();

	virtual int init(const std::string& proto_content,
		const std::string& model_content) final;

	virtual int extract(const cv::Mat& img, std::vector<float>& features) final;
	static float featureContrast(const std::vector<float>& feature1, const std::vector<float>& feature2);
protected:
	TNNKit m_tnnKit;
	const int DIMENSION = 512;
};


#endif //MOBILEFACE_H
