#include "Alarmer.h"
#include <iostream>
#include <regex>
#include <list>

namespace tinykit {

	Alarmer::Alarmer() {
		bRunning = false;
	}

	Alarmer::~Alarmer() {
		stopAll();
	}

	void Alarmer::start(uint64_t alarmerID, int date, int mday, int week, int time, bool isRepeat, const TimeoutCallback& timeoutCallback) {
		AlarmerInfo info;
		info.date = date;
		info.mday = mday;
		info.week = week;
		info.time = time;
		info.repeat = isRepeat;
		info.callback = timeoutCallback;
		mAlarmInfos[alarmerID] = info;
		if (!bRunning) {
			bRunning = true;
			mLoopThread = std::thread(std::mem_fn(&Alarmer::loop), this);
			mLoopThread.detach();
		}
	}


	void Alarmer::stop(uint64_t AlarmerID) {
		std::unique_lock<std::mutex> lck(mInfoMutex);
		map<uint64_t, AlarmerInfo>::iterator it = mAlarmInfos.find(AlarmerID);
		if (it != mAlarmInfos.end()) {
			mAlarmInfos.erase(it);
		}
		mLoopCondition.notify_all();
	}

	void Alarmer::stopAll() {
		std::unique_lock<std::mutex> lck(mInfoMutex);
		bRunning = false;
		mAlarmInfos.clear();
		mLoopCondition.notify_all();
		mLoopMutex.lock();
		mLoopMutex.unlock();
	}


	void Alarmer::loop() {
		std::unique_lock<std::recursive_mutex> loopLock(mLoopMutex);
		std::mutex waitMtx;
		std::unique_lock<std::mutex> waitLock(waitMtx);
		uint64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		while (bRunning) {
			if (mAlarmInfos.size() > 0) {
				mLoopCondition.wait_for(waitLock, std::chrono::milliseconds(500));
				if (bRunning) {
					checkAlarmer();
				}
			}
			else {
				//没有Alarmer任务，退出线程
				bRunning = false;
			}
		}
	}


	void Alarmer::checkAlarmer() {
		time_t t;
		time(&t);
		tm *tm_now = localtime(&t);
		int date = (tm_now->tm_year + 1900) * 10000 + (tm_now->tm_mon + 1) * 100 + tm_now->tm_mday;
		int time = tm_now->tm_hour * 10000 + tm_now->tm_min * 100 + tm_now->tm_sec;
		mInfoMutex.lock();
		for (map<uint64_t, AlarmerInfo>::iterator it = mAlarmInfos.begin(); it != mAlarmInfos.end(); it++) {
			if (!it->second.date && it->second.date != date) {
				continue;
			}
			else if (!it->second.mday && it->second.mday != tm_now->tm_mday + 1) {
				continue;
			}
			else if (!it->second.week && it->second.week != tm_now->tm_wday + 1) {
				continue;
			}

			if (it->second.time == time) {
				if (it->second.callback) {
					//执行回调
					it->second.callback(it->first);
				}
				if (!it->second.repeat) {
					it = mAlarmInfos.erase(it);
				}
			}
		}
		mInfoMutex.unlock();
	}

};


