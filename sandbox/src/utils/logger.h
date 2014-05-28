#ifndef LOGGER_H
#define LOGGER_H

#include "utils/singleton.h"
#include <cstdio>

namespace sb
{
    class Logger: public Singleton<Logger>
    {
    public:
        Logger(FILE* f = stderr);
        ~Logger();

        void printf(const char* msg, ...);
        void trace(const char* msg, ...);
        void info(const char* msg, ...);
        void warn(const char* msg, ...);
        void err(const char* msg, ...);

#ifdef _DEBUG
        void debug(const char* msg, ...);
#else
        inline void debug(const char*, ...) {}
#endif
    private:
        FILE* mFile;
    };
} // namespace sb

#define gLog sb::Logger::get()

#endif // LOGGER_H
