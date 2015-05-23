#ifndef UTILS_STL_H
#define UTILS_STL_H

#include <algorithm>

namespace std
{
    template<typename Container>
    void sort(Container& container)
    {
        std::sort(std::begin(container),
                  std::end(container));
    }

    template<
        typename Container,
        typename Comparator
    >
    void sort(Container& container,
              Comparator&& comparator)
    {
        std::sort(std::begin(container),
                  std::end(container),
                  std::forward<Comparator>(comparator));
    }
}

#endif /* UTILS_STL_H */
