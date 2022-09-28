#include "FileUtil.h"


#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <regex>

#if defined(_WIN32) || defined(_WIN64)
#pragma warning (disable:6309)
#pragma warning (disable:6387)
#endif

#if defined(_WIN32) || defined(_WIN64)
#define RM(a) remove(a)
#define ACCESS _access
#define MKDIR(a,m) _mkdir((a))
#else
#define RM(a) remove(a)
#define ACCESS access
#define MKDIR(a,m) mkdir((a),m)
#endif

#if defined(_WIN32) || defined(_WIN64)
static std::wstring StringUtf8ToWideChar(const std::string& strUtf8)
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

static std::string StringWideCharToUtf8(const std::wstring& strWideChar)
{
	std::string ret;
	if (!strWideChar.empty())
	{
		int nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
		if (nNum)
		{
			char* utf8String = new char[nNum + 1];
			utf8String[0] = 0;

			nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, utf8String, nNum + 1, nullptr, FALSE);

			ret = utf8String;
			delete[] utf8String;
		}
		else
		{
			//CCLOG("Wrong convert to Utf8 code:0x%x", GetLastError());
		}
	}

	return ret;
}

static std::string UTF8StringToMultiByte(const std::string& strUtf8)
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
#endif

namespace tinykit {
	// checks if a file exists, exits otherwise
	string FileUtil::localPath(const string& path) {

#if defined(_WIN32) || defined(_WIN64)
		return UTF8StringToMultiByte(path);
#else
		return path;
#endif
	}

	bool FileUtil::isExists(const string& path) {
		return ACCESS(localPath(path).c_str(), 0) == 0;
	}

	bool FileUtil::createDir(const string& dir, int model) {
		int i = 0;
		int iRet;
		int iLen;
		string locdir = localPath(dir);
		if (locdir.empty())
		{
			return false;
		}
		iLen = locdir.length();
		char* pszDir = (char*)malloc(iLen + 10);
		strcpy(pszDir, locdir.c_str());
		//在末尾加/ 
		if (pszDir[iLen - 1] != '\\' && pszDir[iLen - 1] != '/')
		{
			pszDir[iLen] = '/';
			pszDir[iLen + 1] = '\0';
		}

		// 创建中间目录
		for (i = 0; i < iLen; ++i)
		{
			if (pszDir[i] == '\\' || pszDir[i] == '/')
			{
				pszDir[i] = '\0';
				if (strlen(pszDir) > 0) {
					//如果不存在,创建
					if (ACCESS(pszDir, 0))
					{
						iRet = MKDIR(pszDir, model);
						if (iRet != 0)
						{
							return false;
						}
					}
				}
				//支持linux,将所有\换成/
				pszDir[i] = '/';
			}
		}

		iRet = MKDIR(pszDir, model) == 0;
		free(pszDir);
		return iRet;
	}

	bool FileUtil::rmFile(const string& path) {
		string locpath = localPath(path);
		return RM(locpath.c_str()) == 0;
	}

	bool FileUtil::fileRename(const string& oldpath, const string& newpath) {
		string locoldpath = localPath(oldpath);
		string locnewpathh = localPath(newpath);
		int state = rename(locoldpath.c_str(), locnewpathh.c_str());
		return state == 0;
	}

	string FileUtil::pathStrip(const string& path) {
		char buf[1000];
		bool vaild = true;
		int start = 0;
		int end = 0;
		char ch;
		string repath;
		if (!path.empty()) {
			for (int i = 0; i < path.length(); ++i) {
				ch = path.at(i);
				if (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') {
					vaild = false;
				}
				else {
					vaild = true;
					start = i;
					break;
				}
			}

			for (int i = path.length() - 1; i >= 0; i--) {
				ch = path.at(i);
				if (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') {
					vaild = false;
				}
				else {
					vaild = true;
					end = i;
					break;
				}
			}
			repath = path.substr(start, end - start + 1);
		}
		return repath;
	}

	vector<string> FileUtil::pathSplit(const string& path) {
		vector<string> subs;
		char buf[1000];
		bool vaild = true;
		int index = 0;
		char ch;
		memset(buf, 0, 1000);
		string strip = pathStrip(path);
		if (!strip.empty()) {
			string repath = strip + "/";
			for (int i = 0; i < repath.length(); ++i) {
				ch = repath.at(i);
				if (ch == '/' || ch == '\\') {
					if (vaild) {
						buf[index] = 0;
						subs.push_back(buf);
						index = 0;
					}
					vaild = false;
				}
				else {
					vaild = true;
					buf[index++] = ch;
				}
			}
		}
		return subs;
	}

	void FileUtil::clearOrCreateFile(const string& path) {
		string locpath = localPath(path);
		FILE *fp = fopen(locpath.c_str(), "w");
		if (fp) {
			fclose(fp);
		}
	}

	string FileUtil::basename(const string& path) {
		vector<string> subs = pathSplit(path);
		if (subs.size() > 0) {
			return subs.back();
		}
		return path;
	}

	string FileUtil::basedir(const string& path) {
		vector<string> subs = pathSplit(path);
		if (subs.size() > 0) {
			subs.pop_back();
			return joinPaths(subs);
		}
		return path;
	}


	int FileUtil::copy(const string& srcPath, const string& desPath) {
		string locsrcpath = localPath(srcPath);
		string locdespath = localPath(desPath);
		FILE *fpsrc = fopen(locsrcpath.c_str(), "rb");
		FILE *fpdes = fopen(locdespath.c_str(), "wb");
		if (fpsrc && fpdes) {
			char buffer[1000];
			size_t len;
			while ((len = fread(buffer, 1, 1000, fpsrc)) > 0) {
				fwrite(buffer, 1, len, fpdes);
			}
			fclose(fpsrc);
			fclose(fpdes);
			return true;
		}
		else {
			if (fpsrc) {
				fclose(fpsrc);
				fpdes = NULL;
			}
			if (fpdes) {
				fclose(fpdes);
				fpdes = NULL;
			}
			return false;
		}
	}

	string FileUtil::reservePath(const string& path) {
		if (!isExists(path)) {
			return path;
		}
		string parent = basedir(path);
		string file = basename(path);
		string name;
		string suffix;
		int pos = file.rfind('.');
		if (pos != string::npos && pos > 0 && pos < file.size()) {
			name = file.substr(0, pos);
			suffix = file.substr(pos);
		}
		else {
			name = file;
			suffix = "";
		}
		string repath;
		for (int i = 1; i < 100; ++i) {
			repath = joinPath(parent, name + "(" + to_string(i) + ")" + suffix);
			if (!isExists(repath)) {
				return repath;
			}
		}
		uint64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		repath = joinPath(parent, name + "(" + to_string(ts) + ")" + suffix);
		return repath;
	}

	string FileUtil::vaildName(const string& filename) {
		if (filename.size() > 255) {
			uint64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			return to_string(ts);
		}
		else if (std::regex_match(filename, std::regex("(con)|(prn)|(aux)|(nul)|(com[1-9])|(lpt[1-9])", std::regex::icase))) {
			uint64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			return to_string(ts);
		}
		else {
			string result = std::regex_replace(filename, std::regex(u8"[\+\:\*\/\?\"\<\>\\\|\t\n\r]"), "");
			return result;
		}
	}

	size_t FileUtil::fileSize(const string& path) {
		string locpath = localPath(path);
		size_t size = 0;
		FILE *f = fopen(locpath.c_str(), "r");
		if (f) {
			fseek(f, 0, SEEK_END);
			size = ftell(f);
			fclose(f);
		}
		return size;
	}

	vector<string> FileUtil::filelist(const string& path) {
		string locpath = localPath(path);
		std::vector<std::string> files;

#if defined(_WIN32) || defined(_WIN64)
		long hFile = 0;
		struct _finddata_t fileInfo;
		string pathName;
		if ((hFile = _findfirst(pathName.assign(locpath).append("\\*").c_str(), &fileInfo)) == -1) {
			return files;
		}
		do
		{
			if (strcmp(fileInfo.name, ".") && strcmp(fileInfo.name, "..")) {
				files.push_back(locpath + fileSepator() + fileInfo.name);
			}
		} while (_findnext(hFile, &fileInfo) == 0);
		_findclose(hFile);
#else
		DIR *dp;
		struct dirent *dirp;
		if ((dp = opendir(locpath.c_str())) != NULL)
		{
			while ((dirp = readdir(dp)) != NULL)
			{
				if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {
					files.push_back(locpath + fileSepator() + dirp->d_name);
				}
			}
			closedir(dp);
		}
#endif
		return files;
	}

	string FileUtil::fileSepator() {
#if defined(_WIN32) || defined(_WIN64)
		return "\\";
#else
		return "/";
#endif
	}

	string FileUtil::joinPath(const string& parent, const string& sub) {
		string path = parent;
		if (!path.empty()) {
			path += fileSepator();
		}
		path += sub;
		return path;
	}

	string FileUtil::joinPaths(const vector<string>& subs) {
		string path = "";
		for (const string& s : subs) {
			if (!path.empty()) {
				path += fileSepator();
			}
			path += s;
		}
		return path;
	}

	bool FileUtil::read(const string& path, string &data) {
		string locpath = localPath(path);
		FILE *fp = fopen(locpath.c_str(), "rb");
		if (fp) {
			long lSize = 0;
			fseek(fp, 0, SEEK_END);
			lSize = ftell(fp);
			rewind(fp);
			data.resize(lSize);
			char *buffer = (char*)data.data();
			size_t result = fread(buffer, 1, lSize, fp);
			fclose(fp);
			fp = nullptr;
			buffer = nullptr;
			return true;
		}
		else {
			return false;
		}
	}

	bool FileUtil::write(const string& path, const void *data, size_t lenght) {
		string locpath = localPath(path);
		FILE *fp = fopen(locpath.c_str(), "wb");
		if (fp) {
			fwrite(data, 1, lenght, fp);
			fclose(fp);
			fp = nullptr;
			return true;
		}
		else {
			return false;
		}
	}

	bool FileUtil::append(const string& path, const void *data, size_t lenght) {
		string locpath = localPath(path);
		FILE *fp = fopen(locpath.c_str(), "ab");
		if (fp) {
			fwrite(data, 1, lenght, fp);
			fclose(fp);
			fp = nullptr;
			return true;
		}
		else {
			return false;
		}
	}

} // end namespace
