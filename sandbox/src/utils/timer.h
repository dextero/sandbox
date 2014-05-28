#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

#include <cassert>

namespace sb
{
    class Timer
    {
    public:
        Timer();

        void reset();
        float getSecsElapsed();
        unsigned long long getMillisecsElapsed();

    private:
        timeval mTimeStart;
    };
} // namespace sb

#endif //TIMER_H
