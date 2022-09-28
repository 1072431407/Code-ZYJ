#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <thread>
#include <string>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <fstream>
#include "XLog.h"


namespace tinykit {
	class XLog
	{
	public:
		enum XLogLevel {
			XLogDebug = 0x01,
			XLogInfo = 0x02,
			XLogWarning = 0x04,
			XLogError = 0x08
		};
	public:
		XLog();
		virtual ~XLog();
		static XLog* getInstance();
		void setLogLevel(int level);	//设置输出log level
		void sync();					//同步到文件
		void log(const char* tag, XLogLevel level, const char* format, ...);
		void close();																	
		void autoSync(bool enable);  //开启自动文件同步
	private:
		std::atomic_bool bRunning;
		std::thread mLoopThread;
		std::mutex mLoopMutex;                  //循环锁
		std::mutex mLogMtx;						//log锁
		std::mutex mSynchMtx;					//同步锁
		std::condition_variable mLoopCondition;
		std::string mLogBuf[2];					//log buffer，一个读，一个写，同步到文件之后切换
		int mLogIndex;							//存储log的buffer index
		int mWriteIndex;                        //写文件buffer index
		std::ofstream mOfs;                     //log文件输出流
		int mLogLevel;                          //log level，可以选择多个log level
		uint32_t mSyncTimeMS = 5 * 1000;		//文件同步时间(millisecond),默认5s
		bool bAutoSync = true;                  //自动同步开关
	private:
		void setLogFilePath(const std::string& logFilePath);
		void setSyncTime(uint32_t ms);
		void start();
		void stop();
		void loop();
		void set(const char* tag, const char* log, const char* fmtLog);
	};

};

#define xlogD(tag,...) tinykit::XLog::getInstance()->log(tag,tinykit::XLog::XLogDebug,__VA_ARGS__);
#define xlogI(tag,...) tinykit::XLog::getInstance()->log(tag,tinykit::XLog::XLogInfo,__VA_ARGS__);
#define xlogW(tag,...) tinykit::XLog::getInstance()->log(tag,tinykit::XLog::XLogWarning,__VA_ARGS__);
#define xlogE(tag,...) tinykit::XLog::getInstance()->log(tag,tinykit::XLog::XLogError,__VA_ARGS__);
