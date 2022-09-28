#include "Clock.h"
#include <sstream>

namespace tinykit {
	Clock::Clock(const string& name) {
		name_ = name;
	}
	
	Clock::~Clock() {
	}

	void Clock::start() {
		points_["start"] = std::chrono::steady_clock::now();
	}
	void Clock::end() {
		points_["end"] = std::chrono::steady_clock::now();
	}

	void Clock::split(const string& tip) {
		string key = tip.empty() ? ("time_" + to_string(points_.size())) : tip;
		points_[key] = std::chrono::steady_clock::now();
	}

	void Clock::reset() {
		points_.clear();
	}

	string Clock::summary() {
		stringstream ss;
		ss << "Clock	" << name_ << endl;
		if (points_.size() > 1) {
			if (points_.size() > 2) {
				ss << "--------------" << endl;
				for (int i = 1; i < points_.size(); i++) {
					std::chrono::milliseconds d = std::chrono::duration_cast<std::chrono::milliseconds>(points_.at(i) - points_.at(i - 1));
					int64_t count = d.count();
					if (count > 1000) {
						ss << points_.key(i) << "	:" << double(count)/1000 << "s" << endl;
					}
					else {
						ss << points_.key(i) << "	:" << count << "ms" << endl;
					}
				}
				ss << "--------------" << endl;
			}
			std::chrono::milliseconds total = std::chrono::duration_cast<std::chrono::milliseconds>(points_.back() - points_.front());
			int64_t count = total.count();
			if (count > 1000) {
				ss << "Total	:" << double(count)/1000 << "s" << endl;
			}
			else {
				ss << "Total	:" << count << "ms" << endl;
			}
		}
		else {
			ss << "Insufficient time points" << endl;
		}
		return ss.str();
	}

	list<int> Clock::durationMilliseconds() {
		list<int> dms;
		for (int i = 1; points_.size() > 1 && i < points_.size(); i++) {
			std::chrono::milliseconds d = std::chrono::duration_cast<std::chrono::milliseconds>(points_.at(i) - points_.at(i - 1));
			dms.push_back(d.count());
		}
		return dms;
	}

	list<double> Clock::durationSeconds() {
		list<double> ds;
		for (int i = 1; points_.size() > 1 && i < points_.size(); i++) {
			std::chrono::milliseconds d = std::chrono::duration_cast<std::chrono::milliseconds>(points_.at(i) - points_.at(i - 1));
			ds.push_back(double(d.count())/1000.0);
		}
		return ds;
	}
};
