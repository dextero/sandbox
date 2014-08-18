#include "utils/debug.h"

#ifdef _DEBUG

#include <execinfo.h>
#include <cxxabi.h>
#include <signal.h>
#include <string>
#include <cstdio>
#include <cstring>

#define MAX_TRACE_LEVELS 256

namespace sb {
namespace {

void fixStackTrace(ucontext_t* context,
                   void** stackTrace)
{
    void* caller_address;
    /* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
    caller_address = (void*)context->uc_mcontext.gregs[REG_EIP]; // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
    caller_address = (void*)context->uc_mcontext.gregs[REG_RIP]; // RIP: x86_64 specific
#else
#error Unsupported architecture. // TODO: Add support for other arch.
#endif

    stackTrace[2] = caller_address;
}
 
std::string demangle(const char* symbol) {
    size_t size;
    int status;
    char temp[1024];
    char* demangled;

    //first, try to demangle a c++ name
    if (sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp) == 1) {
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

void printTrace(ucontext_t* context)
{
    void *array[MAX_TRACE_LEVELS];
    size_t size;
    char **strings;

    size = backtrace(array, sizeof(array) / sizeof(array[0]));
    if (context) {
        fixStackTrace(context, array);
    }

    strings = backtrace_symbols(array, size);

    gLog.trace("Obtained %zd stack frames", size);

    for (size_t i = 0; i < size; i++) {
        gLog.trace("% 3u: %s", (unsigned)i, demangle(strings[i]).c_str());
    }

    free(strings);
}

struct sigaction DEFAULT_SIGNAL_HANDLERS[32] = {};

void signalHandler(int sigNum,
                   siginfo_t* info,
                   void* context)
{
    const char* sigName = strsignal(sigNum);
    gLog.err("signal caught: %s", sigName);
    printTrace((ucontext_t*)context);

    if (DEFAULT_SIGNAL_HANDLERS[sigNum].sa_sigaction) {
        DEFAULT_SIGNAL_HANDLERS[sigNum].sa_sigaction(sigNum, info, context);
    }
    if (DEFAULT_SIGNAL_HANDLERS[sigNum].sa_handler) {
        DEFAULT_SIGNAL_HANDLERS[sigNum].sa_handler(sigNum);
    }

    gLog.err("no default signal handler, aborting");
    abort();
}

void initSignalHandlers() __attribute__((constructor));
void initSignalHandlers()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_sigaction = signalHandler;
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    for (int sig: { SIGINT, SIGTERM, SIGHUP, SIGSEGV, SIGFPE }) {
        sigaction(sig, &sa, &DEFAULT_SIGNAL_HANDLERS[sig]);
    }
}

} // namespace

void printTrace()
{
    printTrace(nullptr);
}

} // namespace sb

#endif // _DEBUG

