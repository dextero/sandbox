#ifndef UTILS_MISC_H
#define UTILS_MISC_H

#include "utils/logger.h"

namespace sb
{
    template<typename T>
    class bind_guard
    {
    public:
        template<typename... Args>
        bind_guard(T& object, Args&&... args):
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
        T* object;
    };

    template<
        typename T,
        typename... Args
    >
    bind_guard<T> make_bind(T& obj, Args&&... args)
    {
        return bind_guard<T>(obj, std::forward<Args>(args)...);
    }
}

#ifdef _DEBUG
#   define make_bind(obj, ...) (gLog.debug("make_bind @ %s:%d\n", __FILE__, __LINE__), make_bind(obj, ##__VA_ARGS__))
#endif

#endif /* UTILS_MISC_H */
