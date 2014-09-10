#ifndef SRC_UTILS_DEBUG_H
#define SRC_UTILS_DEBUG_H

#include <cstdlib>

# define sbFail(message, ...) \
    do { \
        gLog.err(message, ##__VA_ARGS__); \
        abort(); \
    } while (0)

# ifdef _DEBUG

#  define sbAssert(condition, message, ...) \
    do { \
        if (!(condition)) { \
            sbFail(message, ##__VA_ARGS__); \
        } \
    } while (0)

# else // _DEBUG

#  define sbAssert(condition, ...) (void)(condition)

# endif // _DEBUG

#include "utils/logger.h"

#endif // SRC_UTILS_DEBUG_H

