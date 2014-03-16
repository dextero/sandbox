#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

#include <cassert>

namespace sb
{
    class Timer
    {
    private:
        timeval mTimeStart;

    public:
        Timer();

        void Reset();
        float GetSecsElapsed();
        unsigned long long GetMillisecsElapsed();
    };
} // namespace sb

#endif //TIMER_H
