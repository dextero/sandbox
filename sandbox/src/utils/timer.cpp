#include <sandbox/utils/timer.h>

#include <cstdlib>

namespace sb
{
    Timer::Timer():
        mTimeStart{0, 0}
    {
        reset();
    }

    void Timer::reset()
    {
        gettimeofday(&mTimeStart, NULL);
    }

    uint64_t Timer::getMicrosecondsElapsed()
    {
        timeval current;
        gettimeofday(&current, NULL);

        uint64_t seconds = (uint64_t)(current.tv_sec - mTimeStart.tv_sec);
        uint64_t useconds = (uint64_t)(current.tv_usec - mTimeStart.tv_usec);
        return seconds * 1000000ULL + useconds;
    }
} // namespace sb

