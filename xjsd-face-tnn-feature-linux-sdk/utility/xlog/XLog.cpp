#include <iostream>
#include "platform.h"
#include "XLog.h"

#ifdef WIN32
#include <windows.h>
#define snprintf _snprintf

std::wstring StringUtf8ToWideChar(const std::string& strUtf8)
{
	std::wstring ret;
	if (!strUtf8.empty())
	{
		int nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (nNum)
		{
			WCHAR* wideCharString = new WCHAR[nNum + 1];
			wideCharString[0] = 0;

			nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wideCharString, nNum + 1);

			ret = wideCharString;
			delete[] wideCharString;
		}
		else
		{
			//CCLOG("Wrong convert to WideChar code:0x%x", GetLastError());
		}
	}
	return ret;
}

std::string UTF8StringToMultiByte(const std::string& strUtf8)
{
	std::string ret;
	if (!strUtf8.empty())
	{
		std::wstring strWideChar = StringUtf8ToWideChar(strUtf8);
		int nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
		if (nNum)
		{
			char* ansiString = new char[nNum + 1];
			ansiString[0] = 0;

			nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, ansiString, nNum + 1, nullptr, FALSE);

			ret = ansiString;
			delete[] ansiString;
		}
		else
		{
			//CCLOG("Wrong convert to Ansi code:0x%x", GetLastError());
		}
	}

	return ret;
}

std::string localPath(const std::string& path) {
	return UTF8StringToMultiByte(path);
}
#else
std::string localPath(const std::string& path) {
	return path;
}
#endif


namespace tinykit {
	static XLog* sInstance = NULL;


	XLog::XLog() :
		mLogIndex(0),
		mWriteIndex(1)
	{
		bRunning = false;
		mLogLevel = XLog::XLogError | XLog::XLogWarning | XLog::XLogInfo;
	}

	XLog::~XLog() {
		close();
	}

	XLog* XLog::getInstance() {
		if (!sInstance) {
			sInstance = new XLog();
		}
		return sInstance;
	}

	void  XLog::setLogFilePath(const std::string& logFilePath) {
		if (mOfs.is_open()) {
			mOfs.close();
		}
		mOfs.open(localPath(logFilePath), std::ios::out);
		if (mOfs.fail()) {
			char err[400];
			sprintf(err, "XLog error! can not open log file \"%s\"", logFilePath.c_str());
			platformConsoleLog("Error: ", err, err);
		}
	}

	void XLog::setSyncTime(uint32_t ms) {
		mSyncTimeMS = ms;
	}

	void XLog::setLogLevel(int level) {
		mLogLevel = level;
	}

	void XLog::start() {
		if (bRunning) {
			stop();
		}
		bRunning = true;
		mLoopThread = std::thread(std::mem_fn(&XLog::loop), this);
		mLoopThread.detach();
	}

	void XLog::stop() {
		mLogMtx.lock();
		if (mOfs.is_open()) {
			mOfs.close();
		}
		mLogMtx.unlock();
		bRunning = false;
		mLoopCondition.notify_all();
		mLoopMutex.lock();
		mLoopMutex.unlock();
	}

	void XLog::loop() {
		std::unique_lock<std::mutex> loopLock(mLoopMutex);
		std::mutex waitMtx;
		std::unique_lock<std::mutex> waitLock(waitMtx);
		while (bRunning) {
			mLoopCondition.wait_for(waitLock, std::chrono::milliseconds(mSyncTimeMS));
			if (bAutoSync) {
				sync();
			}
		}
	}

	void XLog::autoSync(bool enable) {
		std::unique_lock<std::mutex> lck(mSynchMtx);
		bAutoSync = enable;
	}

	void XLog::sync() {
		std::unique_lock<std::mutex> lck(mSynchMtx);
		mLogMtx.lock();
		mWriteIndex = mLogIndex;
		mLogIndex = !mLogIndex;
		mLogMtx.unlock();
		if (mLogBuf[mWriteIndex].length() > 0) {
			mOfs << mLogBuf[mWriteIndex];
			mOfs.flush();
			mLogBuf[mWriteIndex].clear();
		}
	}

	void XLog::set(const char* tag, const char* log, const char* fmtLog) {
		std::unique_lock<std::mutex> lck(mLogMtx);
		platformConsoleLog(tag, log, fmtLog);
		//直接写文件，临时处理
		mOfs << fmtLog;
		mOfs << "\n";
		mOfs.flush();
		//mLogBuf[mLogIndex].append(fmtLog);
		//mLogBuf[mLogIndex].append("\n");
	}


	void XLog::close() {
		sInstance->stop();
	}

	static const char* levelStr(XLog::XLogLevel level) {
		switch (level)
		{
		case XLog::XLogLevel::XLogDebug:
			return "D";
		case XLog::XLogLevel::XLogInfo:
			return "I";
		case XLog::XLogLevel::XLogWarning:
			return "W";
		case XLog::XLogLevel::XLogError:
			return "E";
		default:
			return "U";
		}
	}

	void XLog::log(const char* tag, XLogLevel level, const char* format, ...) {
		if (sInstance && (sInstance->mLogLevel & level) == level) {
			char logbuf[1024 * 4];
			char fmtbuf[1024 * 4];
			int printed;
			va_list args;
			va_start(args, format);
			printed = vsnprintf(logbuf, sizeof(logbuf), format, args);
			va_end(args);

			int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			time_t tt = now / 1000;
			int ms = now % 1000;
			struct tm* ptm = localtime(&tt);
			snprintf(fmtbuf, sizeof(fmtbuf), "[%s][%d-%02d-%02d %02d:%02d:%02d.%03d][%-15s][%s]", levelStr(level), (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday, (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec, ms, tag, logbuf);
			sInstance->set(tag, logbuf, fmtbuf);
		}
	}

};
