#include "Timer.h"
#include <iostream>

namespace tinykit{

	Timer::Timer(){
		bRunning = false;
	}

	Timer::~Timer(){
		stopAll();
	}

	void Timer::start(uint64_t timerID, int64_t duration, bool isRepeat, const TimeoutCallback& timeoutCallback){
		TimeInfo info;
		info.start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		info.duration = duration;
		info.repeat = isRepeat;
		info.callback = timeoutCallback;
		mTimeInfos[timerID] = info;
		if (!bRunning){
			bRunning = true;
			mLoopThread = std::thread(std::mem_fn(&Timer::loop), this);
			mLoopThread.detach();
		}
	}

	void Timer::stop(uint64_t timerID){
		std::unique_lock<std::mutex> lck(mInfoMutex);
		map<uint64_t, TimeInfo>::iterator it = mTimeInfos.find(timerID);
		if (it != mTimeInfos.end()){
			mTimeInfos.erase(it);
		}
		mLoopCondition.notify_all();
	}

	void Timer::stopAll(){
		std::unique_lock<std::mutex> lck(mInfoMutex);
		bRunning = false;
		mTimeInfos.clear();
		mLoopCondition.notify_all();
		mLoopMutex.lock();
		mLoopMutex.unlock();
	}


	void Timer::loop(){
		std::unique_lock<std::recursive_mutex> loopLock(mLoopMutex);
		std::mutex waitMtx;
		std::unique_lock<std::mutex> waitLock(waitMtx);
		uint64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		while (bRunning){
			if (mTimeInfos.size() > 0){
				int64_t sleeptime = _sleepTime();
				mLoopCondition.wait_for(waitLock, std::chrono::milliseconds(sleeptime));
				if (bRunning){
					_checkTimer();
				}
			}
			else{
				//没有timer任务，退出线程
				bRunning = false;
			}
		}
	}

	//计算出最小的休眠时间
	uint64_t Timer::_sleepTime(){
		std::unique_lock<std::mutex> lck(mInfoMutex);
		uint64_t min = 0;
		uint64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		for (map<uint64_t, TimeInfo>::iterator it = mTimeInfos.begin(); it != mTimeInfos.end(); it++){
			uint64_t left = it->second.duration - ts + it->second.start;
			if (min == 0){
				min = left;
			}
			else if (left < min){
				min = left;
			}
		}
		return min < 1 ? 1 : min;
	}

	void Timer::_checkTimer(){
		uint64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		map<uint64_t, TimeInfo> timeoutInfos; //已经timeout，需要callback的info
		mInfoMutex.lock();
		map<uint64_t, TimeInfo>::iterator it = mTimeInfos.begin();
		while (it != mTimeInfos.end()){
			if (ts - it->second.start >= it->second.duration){
				//触发timeout
				timeoutInfos[it->first] = it->second;
				if (it->second.repeat){
					//循环timer,重置计时时间
					it->second.start = ts;
					it++;
				}
				else{
					//单次timer，删除timer任务
					it = mTimeInfos.erase(it);
				}
			}
			else{
				it++;
			}
		}
		mInfoMutex.unlock();
		for (map<uint64_t, TimeInfo>::iterator it = timeoutInfos.begin(); it != timeoutInfos.end(); it++){
			if (ts - it->second.start >= it->second.duration){
				if (it->second.callback){
					//执行回调
					it->second.callback(it->first);
				}
			}
		}
	}

};