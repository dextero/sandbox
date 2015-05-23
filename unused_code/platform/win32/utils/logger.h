#ifndef LOGGER_H
#define LOGGER_H

#include "utils/singleton.h"
#include <cstdio>

namespace sb
{
    class Logger: public Singleton<Logger>
    {
        FILE* mFile;
        void* mConsole;

    public:
        Logger(FILE* f = stderr);
        ~Logger();

        void printf(const char* msg, ...);
        void info(const char* msg, ...);
        void warn(const char* msg, ...);
        void err(const char* msg, ...);
    };
} // namespace sb

#define gLog sb::Logger::get()

#endif // LOGGER_H
