#ifndef PROFILER_H
#define PROFILER_H

#include <vector>
#include <map>
#include <string>
#include <cstdio>

#include "timer.h"

#ifndef NO_PROFILE

namespace sb
{
    class _Profiler
    {
    private:
        struct FuncTotalData {
            unsigned long long totalCalls;
            unsigned long long totalTime;

            FuncTotalData(): totalCalls(0ULL), totalTime(0ULL) {}
        };

        struct FuncCallData {
            FuncCallData* parent;
            unsigned totalCalls;
            unsigned long long totalMilliseconds;
            // using __FUNCTION__ macro, so string pointer stays the same
            std::map<const char*, FuncCallData> subCalls;

            FuncCallData(FuncCallData* parent = NULL): parent(parent), totalCalls(0u), totalMilliseconds(0ULL) {}

            void Print(FILE* out, unsigned indentLevel = 0, unsigned long long parentTime = (unsigned long long)-1) const
            {
                for (std::map<const char*, FuncCallData>::const_iterator it = subCalls.begin(); it != subCalls.end(); ++it)
                {
                    for (unsigned i = 0; i < indentLevel; ++i)
                        fprintf(out, "  ");

                    fprintf(out, "%s: %u calls (%llu ms, %.2f%%)\n", it->first, it->second.totalCalls, it->second.totalMilliseconds,
                        parentTime != (unsigned long long)-1 ? ((float)((it->second.totalMilliseconds * 1000ULL) / (parentTime + 1ULL)) / 10.f) : 100.f);
                    it->second.Print(out, indentLevel + 1, it->second.totalMilliseconds);
                }
            }

            // returns total runtime
            unsigned long long GetTotalTime(std::map<const char*, FuncTotalData>& time) const
            {
                unsigned long long totalRuntime = 0ULL;

                for (std::map<const char*, FuncCallData>::const_iterator it = subCalls.begin(); it != subCalls.end(); ++it)
                {
                    if (time.find(it->first) == time.end())
                        time.insert(std::make_pair(it->first, FuncTotalData()));

                    time[it->first].totalTime += it->second.totalMilliseconds;
                    time[it->first].totalCalls += it->second.totalCalls;
                    totalRuntime += it->second.totalMilliseconds;

                    it->second.GetTotalTime(time);
                }

                return totalRuntime;
            }
        };

        // using __FUNCTION__ macro, so string pointer stays the same
        static FuncCallData mRoot;
        static FuncCallData* mCurrent;
    public:
        class Timer: public sb::Timer
        {
        public:
            Timer(const char* func)
            {
                _Profiler::FuncCall(func);
            }

            ~Timer()
            {
                _Profiler::FuncEnd(GetMillisecsElapsed());
            }
        };

        static void FuncCall(const char* func);
        static void FuncEnd(unsigned long long millisecs);

        static void PrintProfileReport(const char* file = NULL);
        static void PrintProfileReportTree(FILE* file);
        static void PrintProfileReportSorted(FILE* file);

        static const std::string Backtrace();
    };
} // namespace sb


#   define PROFILE()    sb::_Profiler::Timer __profile_timer(__FUNCTION__)
#   define PRINT_PROFILE_REPORT(foo) sb::_Profiler::PrintProfileReport(foo)

#else //!NO_PROFILE

#   define PROFILE()
#   define PRINT_PROFILE_REPORT(foo)    // compatibility

#endif //NO_PROFILE

#endif //PROFILER_H
