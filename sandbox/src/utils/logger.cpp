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

            int bytes_written = vsnprintf(buffer, sizeof(buffer), msg, args);
            if (bytes_written > 0
                    && bytes_written + 1 < (int)sizeof(buffer)
                    && buffer[bytes_written - 1] != '\n') {
                buffer[bytes_written] = '\n';
                buffer[bytes_written + 1] = '\0';
            }

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

    void Logger::trace(const char* msg, ...)
    {
        va_list list;

        va_start(list, msg);
        log(mFile, LogColor::WHITE, "[TRACE] ", msg, list);
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
        log(mFile, LogColor::RED, "[ERR] ", msg, list);
        va_end(list);
    }

#ifdef _DEBUG
    void Logger::debug(const char* msg, ...)
    {
        va_list list;

        va_start(list, msg);
        log(mFile, LogColor::BLUE, "[DEBUG] ", msg, list);
        va_end(list);
    }
#endif
} // namespace sb
