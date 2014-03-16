#include "logger.h"
#include <cstdarg>
#include <windows.h>

namespace sb
{
    SINGLETON_INSTANCE(Logger);


    Logger::Logger(FILE* f):
        mFile(f),
        mConsole(::GetStdHandle(STD_ERROR_HANDLE))
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

        fprintf(mFile, buffer);
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

        fprintf(mFile, buffer);
        fflush(mFile);
    }

    void Logger::Warn(const char* msg, ...)
    {
        static char buffer[1024];
        va_list list;

        ::SetConsoleTextAttribute(mConsole, FOREGROUND_RED | FOREGROUND_GREEN);
        fprintf(mFile, "[WARN] ");
        ::SetConsoleTextAttribute(mConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        va_start(list, msg);
        vsprintf(buffer, msg, list);
        va_end(list);

        fprintf(mFile, buffer);
        fflush(mFile);
    }

    void Logger::Err(const char* msg, ...)
    {
        static char buffer[1024];
        va_list list;

        ::SetConsoleTextAttribute(mConsole, FOREGROUND_RED);
        fprintf(mFile, "[ERR]  ");
        ::SetConsoleTextAttribute(mConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        va_start(list, msg);
        vsprintf(buffer, msg, list);
        va_end(list);

        fprintf(mFile, buffer);
        fflush(mFile);
    }
} // namespace sb
