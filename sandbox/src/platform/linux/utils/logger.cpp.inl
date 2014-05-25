#include "logger.h"
#include <cstdarg>

namespace sb
{
    SINGLETON_INSTANCE(Logger);


    Logger::Logger(FILE* f):
        mFile(f)
    {
    //#ifdef _DEBUG
        //static const char* logFile = "debug.log";
        //f = fopen(logFile, "w");
        //if (f)
        //{
            //fprintf(mFile, "log redirected to %s\n", logFile);
            //mFile = f;
        //}
    //#endif //_DEBUG
    }

    Logger::~Logger()
    {
        if (mFile)
            fflush(mFile);
    }

    namespace
    {
        enum class LogColor
        {
            WHITE = 0,
            GREEN = 32,
            YELLOW = 33,
            RED = 31,
            BLUE = 34
        };

        void log(FILE* file,
                 LogColor color,
                 const char* prefix,
                 const char* msg,
                 va_list args)
        {
            static char buffer[1024];
            
            vsprintf(buffer, msg, args);
            fprintf(file, "\033\[%dm%s%s\033\[0m", color, prefix, buffer);
            fflush(file);
        }
    }

    void Logger::printf(const char* msg, ...)
    {
        va_list list;

        va_start(list, msg);
        log(mFile, LogColor::WHITE, "", msg, list);
        va_end(list);
    }

    void Logger::info(const char* msg, ...)
    {
        va_list list;

        va_start(list, msg);
        log(mFile, LogColor::GREEN, "[INFO] ", msg, list);
        va_end(list);
    }

    void Logger::warn(const char* msg, ...)
    {
        va_list list;

        va_start(list, msg);
        log(mFile, LogColor::YELLOW, "[WARN] ", msg, list);
        va_end(list);
    }

    void Logger::err(const char* msg, ...)
    {
        va_list list;

        va_start(list, msg);
        log(mFile, LogColor::RED, "[ERR]  ", msg, list);
        va_end(list);
    }

#ifdef _DEBUG
    void Logger::debug(const char* msg, ...)
    {
        va_list list;

        va_start(list, msg);
        log(mFile, LogColor::BLUE, "[DBG]  ", msg, list);
        va_end(list);
    }
#endif
} // namespace sb
