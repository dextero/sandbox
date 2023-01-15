#ifndef LOGGER_H
#define LOGGER_H

#include "utils/singleton.h"

#include <cstdarg>
#include <cstdio>
#include <deque>
#include <sstream>

#include "utils/debug.h"

namespace sb {

class FileOutput
{
public:
    FileOutput(FILE* file = stderr):
        mFile(file)
    {}

    void write(const char* text)
    {
        fprintf(mFile, "%s", text);
    }

    void flush()
    {
        fflush(mFile);
    }

private:
    FILE* mFile;
};

class QueueOutput
{
public:
    QueueOutput(size_t queueSize = 32):
        mQueueSize(queueSize)
    {}

    QueueOutput(QueueOutput&& o)
    {
        *this = std::move(o);
    }
    QueueOutput& operator =(QueueOutput&& o)
    {
        mQueueSize = o.mQueueSize;
        mQueue.swap(o.mQueue);
        mNextLine.str(o.mNextLine.str());

        o.mNextLine.str("");
        return *this;
    }

    void write(const char* text)
    {
        mNextLine << text;
    }

    void flush()
    {
        while (mQueue.size() >= mQueueSize) {
            mQueue.pop_front();
        }

        mQueue.push_back(mNextLine.str());
        mNextLine.str("");
    }

    std::string getOutput() const
    {
        std::stringstream ss;
        for (const std::string& entry: mQueue) {
            ss << entry;
        }
        return ss.str();
    }

private:
    size_t mQueueSize;
    std::deque<std::string> mQueue;
    std::stringstream mNextLine;
};

template<typename> class Logger;
extern Logger<FileOutput> default_logger;

#define gLog ::sb::default_logger

template<typename Output>
class Logger
{
public:
    enum class Color
    {
        White = 0,
        Green = 32,
        Yellow = 33,
        Red = 31,
        Blue = 34
    };

    Logger(Output&& output = std::move(Output())):
        mOutput(std::forward<Output>(output))
    {}

    virtual ~Logger()
    {
        mOutput.flush();
    }

    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator =(const Logger&) = delete;
    Logger& operator =(Logger&&) = delete;

#if !TEST
# define DEFINE_LOG_LEVEL(Name, Color, Prefix) \
    void Name(const char* msg, ...) \
    { \
        va_list list; \
        va_start(list, msg); \
        log_internal(Color, Prefix, msg, true, list); \
        va_end(list); \
    }
#else // TEST
# define DEFINE_LOG_LEVEL(Name, Color, Prefix) \
    void Name(const char*, ...) {}
#endif // TEST

    DEFINE_LOG_LEVEL(printf, Color::White, "")
    DEFINE_LOG_LEVEL(trace, Color::White, "[TRACE] ")
    DEFINE_LOG_LEVEL(info, Color::Green, "[INFO] ")
    DEFINE_LOG_LEVEL(warn, Color::Yellow, "[WARN] ")
    DEFINE_LOG_LEVEL(err, Color::Red, "[ERR] ")

#ifdef _DEBUG
    DEFINE_LOG_LEVEL(debug, Color::Blue, "[DEBUG] ")
#else
    inline void debug(const char*, ...) {}
#endif

#if TEST
    void printf(Color color, const char* msg, ...)
    {
        va_list list;
        va_start(list, msg);
        log_internal(color, "", msg, false, list);
        va_end(list);
    }
#endif // TEST

    void flush() { mOutput.flush(); }
    const Output& getOutput() const { return mOutput; }

private:
    Output mOutput;

    void log_internal(Color color,
                      const char* prefix,
                      const char* msg,
                      bool appendNewline,
                      va_list args)
    {
        char format_buffer[1024];
        char buffer[4096];

        int bytes_written = snprintf(format_buffer, sizeof(format_buffer),
                                     "\033\[%dm%s%s\033\[0m",
                                     static_cast<int>(color), prefix, msg);
        sbAssert(bytes_written >= 0
                     && bytes_written < (int)sizeof(format_buffer),
                 "format_buffer too small");

        bytes_written = vsnprintf(buffer, sizeof(buffer),
                                  format_buffer, args);
        sbAssert(bytes_written >= 0
                    && bytes_written < (int)sizeof(buffer),
                 "buffer too small");

        if (appendNewline
                && bytes_written > 5
                && buffer[bytes_written - 5] != '\n'
                && (size_t)(bytes_written + 1) < sizeof(buffer)) {
            buffer[bytes_written] = '\n';
            buffer[bytes_written + 1] = '\0';
        }

        mOutput.write(buffer);
    }
};

} // namespace sb

#endif // LOGGER_H

