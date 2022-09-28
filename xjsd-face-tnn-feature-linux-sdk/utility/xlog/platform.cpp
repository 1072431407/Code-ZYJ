#include "platform.h"

#ifdef SYS_ANDROID
    #include <android/log.h>
    void platformConsoleLog(const char* tag, const char* out, const char* fmtLog) {
		__android_log_print(ANDROID_LOG_INFO, tag, out);
    }
#else
    void platformConsoleLog(const char* tag, const char* out, const char* fmtLog) {
        std::cout << fmtLog << std::endl;
    }
#endif

