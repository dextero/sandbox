#ifndef LEXICALCAST_H
#define LEXICALCAST_H

#include <sstream>
#include <string>
#include <cassert>

namespace detail
{
    template<typename DstT, typename SrcT>
    struct lexical_cast_helper
    {
        static inline DstT cast(const SrcT& src);
    };

    template<>
    struct lexical_cast_helper<std::string, std::string>
    {
        static inline const std::string& cast(const std::string &src)
        {
            return src;
        }
    };

    template<typename SrcT>
    struct lexical_cast_helper<std::string, SrcT>
    {
        static inline std::string cast(const SrcT& src)
        {
            std::stringstream ss;
            ss << src;
            return ss.str();
        }
    };

    template<typename DstT>
    struct lexical_cast_helper<DstT, std::string>
    {
        static inline DstT cast(const std::string& src);
    };
}

template<typename DstT, typename SrcT>
DstT lexical_cast(const SrcT& src)
{
    return detail::lexical_cast_helper<DstT, SrcT>::cast(src);
}

template<typename DstT, typename SrcT>
DstT detail::lexical_cast_helper<DstT, SrcT>::cast(const SrcT& src)
{
    std::stringstream ss;
    ss << src;
    DstT ret;
    ss >> ret;
    assert(ss.eof() && "not all data converted");
    return ret;
}

template<typename DstT>
DstT detail::lexical_cast_helper<DstT, std::string>::cast(const std::string& src)
{
    std::stringstream ss(src);
    DstT ret;
    ss >> ret;
    assert(ss.eof() && "not all data converted");
    return ret;
}

#endif // LEXICALCAST_H
