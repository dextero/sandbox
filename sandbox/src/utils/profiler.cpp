#include "profiler.h"
#include <algorithm>

#ifndef NO_PROFILE

namespace sb
{
    _Profiler::FuncCallData _Profiler::mRoot = _Profiler::FuncCallData(NULL);
    _Profiler::FuncCallData* _Profiler::mCurrent = &_Profiler::mRoot;

    void _Profiler::FuncCall(const char* func)
    {
        assert(mCurrent);

        if (mCurrent->subCalls.find(func) == mCurrent->subCalls.end())
            mCurrent->subCalls.insert(std::make_pair(func, FuncCallData(mCurrent)));

        mCurrent = &mCurrent->subCalls[func];
        ++mCurrent->totalCalls;
    }

    void _Profiler::FuncEnd(unsigned long long millisecs)
    {
        assert(mCurrent);

        mCurrent->totalMilliseconds += millisecs;
        mCurrent = mCurrent->parent;
    }

    void _Profiler::PrintProfileReport(const char* file)
    {
        FILE* out = stdout;
        if (file != NULL)
        {
            out = fopen(file, "w");
            if (out == NULL)
                out = stdout;
        }

        PrintProfileReportTree(out);
        PrintProfileReportSorted(out);

        if (out != stdout)
        {
            fclose(out);
            fprintf(stdout, "Profile report saved to %s\n.", file);
        }
    }

    void _Profiler::PrintProfileReportTree(FILE* out)
    {
        fprintf(out, "--- PROFILE REPORT - TREE ---\n\n");
        mRoot.Print(out);
        fprintf(out, "\n--- END OF PROFILE REPORT - TREE ---\n");
    }

    void _Profiler::PrintProfileReportSorted(FILE* out)
    {
        // total time, total calls
        std::map<const char*, FuncTotalData> totalTime;
        std::vector<std::pair<const char*, FuncTotalData> > sorted;
        unsigned long long programTimeMilliseconds = 0ULL;

        programTimeMilliseconds = mRoot.GetTotalTime(totalTime);
        for (std::map<const char*, FuncTotalData>::iterator it = totalTime.begin(); it != totalTime.end(); ++it)
            sorted.push_back(*it);

        // by total time
        fprintf(out, "--- PROFILE REPORT - SORTED BY TOTAL TIME ---\n\n");

        std::sort(sorted.begin(), sorted.end(), [](const std::pair<const char*, FuncTotalData>& first, const std::pair<const char*, FuncTotalData>& second) -> bool {
            return first.second.totalTime > second.second.totalTime;
        });

        for (auto it = sorted.begin(); it != sorted.end(); ++it)
            fprintf(out, "%4llu calls\t %8llu ms (%3.2f%%) - %s\n", it->second.totalCalls, it->second.totalTime, (float)(10000ULL * it->second.totalTime / programTimeMilliseconds) / 100.f, it->first);

        fprintf(out, "\n--- END OF PROFILE REPORT - SORTED BY TOTAL TIME ---\n");

        // by percentage
        fprintf(out, "--- PROFILE REPORT - SORTED BY TOTAL CALLS ---\n\n");

        std::sort(sorted.begin(), sorted.end(), [](const std::pair<const char*, FuncTotalData>& first, const std::pair<const char*, FuncTotalData>& second) -> bool {
            return first.second.totalCalls > second.second.totalCalls;
        });

        for (auto it = sorted.begin(); it != sorted.end(); ++it)
            fprintf(out, "%4llu calls\t %8llu ms (%3.2f%%) - %s\n", it->second.totalCalls, it->second.totalTime, (float)(10000ULL * it->second.totalTime / programTimeMilliseconds) / 100.f, it->first);

        fprintf(out, "\n--- END OF PROFILE REPORT - SORTED BY TOTAL CALLS ---\n");

    }

    const std::string _Profiler::Backtrace()
    {
        std::string ret;
        FuncCallData* ptr = mCurrent;
        if (ptr)
        {
            while (ptr->parent)
            {
                for (std::map<const char*, FuncCallData>::iterator it = ptr->parent->subCalls.begin(); it != ptr->parent->subCalls.end(); ++it)
                    if (&it->second == ptr)
                        ret += it->first + std::string("\n");

                ptr = ptr->parent;
            }
        }

        return ret;
    }
} // namespace sb

#endif //!NO_PROFILE
