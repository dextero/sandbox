#include "timer.h"

namespace sb
{
    Timer::Timer()
    {
        ::QueryPerformanceFrequency(&mFrequency);

        assert(mFrequency.QuadPart && "hi-res performance counter not supported");

        Reset();
    }

    void Timer::Reset()
    {
        ::QueryPerformanceCounter(&mTimeStart);
    }

    float Timer::GetSecsElapsed()
    {
        LARGE_INTEGER current;
        ::QueryPerformanceCounter(&current);

        return (float)((double)(current.QuadPart - mTimeStart.QuadPart) / (double)mFrequency.QuadPart);
    }

    unsigned long long Timer::GetMillisecsElapsed()
    {
        LARGE_INTEGER current;
        ::QueryPerformanceCounter(&current);

        return (current.QuadPart - mTimeStart.QuadPart) * 1000LL / mFrequency.QuadPart;
    }
} // namespace sb

