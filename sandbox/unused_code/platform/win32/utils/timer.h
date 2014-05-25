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

        void reset();
        float getSecsElapsed();
        unsigned long long getMillisecsElapsed();
    };
} // namespace sb

#endif //TIMER_H
