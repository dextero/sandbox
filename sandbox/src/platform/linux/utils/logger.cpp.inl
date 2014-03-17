#include "logger.h"
#include <cstdarg>

namespace sb
{
    SINGLETON_INSTANCE(Logger);


    Logger::Logger(FILE* f):
        mFile(f)
    {
    #ifdef _DEBUG
        static const char* logFile = "debug.log";
        f = fopen(logFile, "w");
        if (f)
        {
            fprintf(mFile, "log redirected to %s\n", logFile);
            mFile = f;
        }
    #endif //_DEBUG
    }

    Logger::~Logger()
    {
        if (mFile)
            fflush(mFile);
    }

    void Logger::Printf(const char* msg, ...)
    {
        static char buffer[1024];
        va_list list;

        va_start(list, msg);
        vsprintf(buffer, msg, list);
        va_end(list);

        fprintf(mFile, "%s", buffer);
        fflush(mFile);
    }

    void Logger::Info(const char* msg, ...)
    {
        static char buffer[1024];
        va_list list;

        fprintf(mFile, "[INFO] ");
        va_start(list, msg);
        vsprintf(buffer, msg, list);
        va_end(list);

        fprintf(mFile, "%s", buffer);
        fflush(mFile);
    }

    void Logger::Warn(const char* msg, ...)
    {
        static char buffer[1024];
        va_list list;

        fprintf(mFile, "[WARN] ");
        va_start(list, msg);
        vsprintf(buffer, msg, list);
        va_end(list);

        fprintf(mFile, "%s", buffer);
        fflush(mFile);
    }

    void Logger::Err(const char* msg, ...)
    {
        static char buffer[1024];
        va_list list;

        fprintf(mFile, "[ERR]  ");
        va_start(list, msg);
        vsprintf(buffer, msg, list);
        va_end(list);

        fprintf(mFile, "%s", buffer);
        fflush(mFile);
    }
} // namespace sb
