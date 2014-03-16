#ifndef LOGGER_H
#define LOGGER_H

#include "utils/singleton.h"
#include <cstdio>

namespace sb
{
    class Logger: public Singleton<Logger>
    {
        FILE* mFile;

    public:
        Logger(FILE* f = stderr);
        ~Logger();

        void Printf(const char* msg, ...);
        void Info(const char* msg, ...);
        void Warn(const char* msg, ...);
        void Err(const char* msg, ...);
    };
} // namespace sb

#define gLog sb::Logger::Get()

#endif // LOGGER_H
