#include "timer.h"

namespace sb
{
    Timer::Timer()
    {
        ::QueryPerformanceFrequency(&mFrequency);

        assert(mFrequency.QuadPart && "hi-res performance counter not supported");

        reset();
    }

    void Timer::reset()
    {
        ::QueryPerformanceCounter(&mTimeStart);
    }

    float Timer::getSecsElapsed()
    {
        LARGE_INTEGER current;
        ::QueryPerformanceCounter(&current);

        return (float)((double)(current.QuadPart - mTimeStart.QuadPart) / (double)mFrequency.QuadPart);
    }

    unsigned long long Timer::getMillisecsElapsed()
    {
        LARGE_INTEGER current;
        ::QueryPerformanceCounter(&current);

        return (current.QuadPart - mTimeStart.QuadPart) * 1000LL / mFrequency.QuadPart;
    }
} // namespace sb

