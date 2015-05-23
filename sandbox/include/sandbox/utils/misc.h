#ifndef UTILS_MISC_H
#define UTILS_MISC_H

#include <utility>

#include <sandbox/utils/logger.h>

namespace sb
{
    template<typename T>
    class bind_guard
    {
    public:
        template<typename... Args>
        bind_guard(const T& object, Args&&... args):
            object(&object)
        {
            object.bind(std::forward<Args>(args)...);
        }

        bind_guard(const bind_guard&) = delete;
        bind_guard& operator =(const bind_guard&) = delete;

        bind_guard(bind_guard&& old):
            object(old.object)
        {
            old.object = nullptr;
        }

        bind_guard& operator =(bind_guard&& old)
        {
            object = old.object;
            old.object = nullptr;
            return *this;
        }

        ~bind_guard()
        {
            if (object) {
                object->unbind();
            }
        }

    private:
        const T* object;
    };

    template<
        typename T,
        typename... Args
    >
    bind_guard<T> make_bind(const T& obj, Args&&... args)
    {
        return bind_guard<T>(obj, std::forward<Args>(args)...);
    }
}

#endif /* UTILS_MISC_H */
