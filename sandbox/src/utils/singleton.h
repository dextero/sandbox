#ifndef SINGLETON_H
#define SINGLETON_H

#include <cassert>
#include <cstdlib>


namespace sb
{
    template<typename T> class Singleton
    {
    private:
        static T* mPtr;

    protected:
        Singleton()
        {
            // there should be only one instance!
            assert(!mPtr);
        }

    public:
        static T& Get()
        {
            if (mPtr) return *mPtr;
            else
            {
                mPtr = new T;
                return *mPtr;
            }
        }

        static void Reset()
        {
            if (mPtr)
            {
                delete mPtr;
                mPtr = new T;
            }
        }

        static void Release()
        {
            if (mPtr)
                delete mPtr;
            mPtr = NULL;
        }
    };
} // namespace sb

#define SINGLETON_INSTANCE(type) template<> type* sb::Singleton<type>::mPtr = NULL

#endif //SINGLETON_H
