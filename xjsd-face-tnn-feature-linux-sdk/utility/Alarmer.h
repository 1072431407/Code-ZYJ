#pragma once

#include <stdint.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <condition_variable>
#include <functional>

using namespace std;

namespace tinykit{
    class Alarmer
	{
	public:
		typedef std::function<void(uint64_t alarmerID)> TimeoutCallback;
    private:
        struct AlarmerInfo
		{
			int date = 0;
			int week = 0;
			int mday = 0;
			int time = 0;
            bool repeat = false;
            TimeoutCallback callback = nullptr;
        };
        std::atomic_bool bRunning;					//是否运行中
        std::thread mLoopThread;					//主循环Thread
        std::recursive_mutex mLoopMutex;			//主循环Mutex
        std::condition_variable mLoopCondition;		//主循环触发条件变量
        std::mutex mInfoMutex;						//Alarmer info Mutex
        map<uint64_t, AlarmerInfo> mAlarmInfos;     //Alarmer info
        void checkAlarmer();						//check Alarmer
    public:
        Alarmer();
        virtual ~Alarmer();
		/**
		*开始闹钟
		*@param alarmerID : 闹钟id
		*@param date : 日期，如果不为0，则定时到该日期触发
		*@param mday : 一个月中的第几天，如果不为0，则定时到每个月该日期触发
		*@param week : 一周中的第几天，如果不为0，则定时到每周触发，1-7表示周一到周日
		*@param time : 触发时间
		*@param isRepeat : 是否重复触发
		*@param timeoutCallback : 到期回调
		*/
		void start(uint64_t alarmerID, int date, int mday, int week, int time, bool isRepeat, const TimeoutCallback& timeoutCallback);

        void stop(uint64_t AlarmerID = 0);

        void stopAll();
    private:
        void loop();
    };
};

