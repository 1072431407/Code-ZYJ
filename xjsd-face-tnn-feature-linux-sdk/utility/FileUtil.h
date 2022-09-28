#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <functional>

using namespace std;

namespace tinykit{
    class FileUtil
    {
    public:
        static string localPath(const string& path);    //转换系统能识别的路径编码
        // checks if a file exists, exits otherwise
        static bool isExists(const string& path);
        // create the directory
		// support create multi-level directory
        static bool createDir(const string& dir, int model = 0777);
        //remove 
        static bool rmFile(const string& path);

        static bool fileRename(const string& oldpath, const string& newpath);
        
        static size_t fileSize(const string& path);

        static string pathStrip(const string& path);

        static vector<string> pathSplit(const string& path);

        static string basename(const string& path);

        static string basedir(const string& path);

        static void clearOrCreateFile(const string& path);

		static int copy(const string& srcPath,const string& desPath);

        static string reservePath(const string& path);    //预定文件名，如果路径已经存在则添加后缀

        static string vaildName(const string& filename);    //有效文件名，为了对应windows命名限制

        static vector<string> filelist(const string& path);

        static string fileSepator();

        static string joinPath(const string& parent, const string& sub);

        static string joinPaths(const vector<string>& subs);

        static bool read(const string& path,string &data);

        static bool write(const string& path, const void *data, size_t lenght);

        static bool append(const string& path, const void *data, size_t lenght);

    };
};

