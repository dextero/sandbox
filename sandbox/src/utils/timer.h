#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
#include <cstdint>

namespace sb
{
    class Timer
    {
    public:
        Timer();

        void reset();
        uint64_t getMicrosecondsElapsed();

        inline float getSecondsElapsed()
        {
            return (float)getMicrosecondsElapsed() / 1000000.0f;
        }

    private:
        timeval mTimeStart;
    };
} // namespace sb

#endif //TIMER_H
