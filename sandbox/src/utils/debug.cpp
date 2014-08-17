#include "utils/debug.h"

#ifdef _DEBUG

#include <execinfo.h>
#include <cxxabi.h>
#include <string>
#include <cstdio>

#define MAX_TRACE_LEVELS 256
 
namespace sb {
namespace {

std::string demangle(const char* symbol) {
    size_t size;
    int status;
    char temp[1024];
    char* demangled;

    //first, try to demangle a c++ name
    if (1 == sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp)) {
        if (NULL != (demangled = abi::__cxa_demangle(temp, NULL, &size, &status))) {
            std::string result(demangled);
            free(demangled);
            return result;
        }
    }

    //if that didn't work, try to get a regular c symbol
    if (1 == sscanf(symbol, "%127s", temp)) {
        return temp;
    }

    //if all else fails, just return the symbol
    return symbol;
}

} // namespace

void printTrace()
{
    void *array[MAX_TRACE_LEVELS];
    size_t size;
    char **strings;

    size = backtrace(array, sizeof(array) / sizeof(array[0]));
    strings = backtrace_symbols(array, size);

    gLog.trace("Obtained %zd stack frames", size);

    for (size_t i = 0; i < size; i++) {
        gLog.trace("% 3u: %s", (unsigned)i, demangle(strings[i]).c_str());
    }

    free(strings);
}

} // namespace sb

#endif // _DEBUG

