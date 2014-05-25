#include "timer.h"
#include <cstdlib>

namespace sb
{
    Timer::Timer()
    {
        reset();
    }

    void Timer::reset()
    {
        gettimeofday(&mTimeStart, NULL);
    }

    float Timer::getSecsElapsed()
    {
        timeval current;
        gettimeofday(&current, NULL);

        return (float)((double)(current.tv_usec - mTimeStart.tv_usec) / (double)1000000.);
    }

    unsigned long long Timer::getMillisecsElapsed()
    {
        timeval current;
        gettimeofday(&current, NULL);

        return (unsigned long long)(current.tv_usec - mTimeStart.tv_usec) * 1000LL;
    }
} // namespace sb

