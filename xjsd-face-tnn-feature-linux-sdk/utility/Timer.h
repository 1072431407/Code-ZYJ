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
    class Timer
    {
    public:
        typedef std::function<void(uint64_t timerID)> TimeoutCallback;
    private:
        struct TimeInfo
        {
            uint64_t duration = 0;
            uint64_t start = 0;
            bool repeat = false;
            TimeoutCallback callback = nullptr;
        };
        std::atomic_bool bRunning;				//是否运行中
        std::thread mLoopThread;				//主循环Thread
        std::recursive_mutex mLoopMutex;		//主循环Mutex
        std::condition_variable mLoopCondition;	//主循环触发条件变量
        std::mutex mInfoMutex;		            //timer info Mutex
        map<uint64_t, TimeInfo> mTimeInfos;     //timer info
        uint64_t _sleepTime();                  //计算休眠时间
        void _checkTimer();                      //check timer
    public:
        Timer();
        virtual ~Timer();

        void start(uint64_t timerID, int64_t duration, bool isRepeat, const TimeoutCallback& timeoutCallback);

        void stop(uint64_t timerID = 0);

        void stopAll();
    private:
        void loop();
    };
};

