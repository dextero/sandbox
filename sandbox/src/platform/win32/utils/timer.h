#ifndef TIMER_H
#define TIMER_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cassert>

namespace sb
{
    class Timer
    {
    private:
        LARGE_INTEGER mFrequency, mTimeStart;

    public:
        Timer();

        void Reset();
        float GetSecsElapsed();
        unsigned long long GetMillisecsElapsed();
    };
} // namespace sb

#endif //TIMER_H
