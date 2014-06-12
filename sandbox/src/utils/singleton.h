#ifndef SINGLETON_H
#define SINGLETON_H

#include <cassert>
#include <cstdlib>


namespace sb
{
    template<typename T> class Singleton
    {
    public:
        virtual ~Singleton() {}

        static T& get()
        {
            if (mPtr) return *mPtr;
            else
            {
                mPtr = new T;
                return *mPtr;
            }
        }

        static void reset()
        {
            if (mPtr)
            {
                delete mPtr;
                mPtr = new T;
            }
        }

        static void release()
        {
            if (mPtr)
                delete mPtr;
            mPtr = NULL;
        }

    private:
        static T* mPtr;

    protected:
        Singleton()
        {
            // there should be only one instance!
            assert(!mPtr);
        }
    };
} // namespace sb

#define SINGLETON_INSTANCE(type) template<> type* sb::Singleton<type>::mPtr = NULL

#endif //SINGLETON_H
