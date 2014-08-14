#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <sstream>
#include <vector>
#include <functional>

#include "utils/types.h"
#include "utils/lexical_cast.h"

namespace sb {
namespace utils {

template<typename T>
std::string toString(const T& elem)
{
    std::stringstream ss;
    ss << elem;
    return ss.str();
}

template<typename T>
std::wstring toWString(const T& elem)
{
    std::wstringstream wss;
    wss << elem;
    return wss.str();
}

namespace detail {

template<typename Arg>
void makeString(std::ostream& os,
                Arg&& arg)
{
    os << arg;
}

template<typename FirstArg, typename... Args>
void makeString(std::ostream& os,
                FirstArg&& first,
                Args&&... args)
{
    os << first;
    makeString(os, std::forward<Args>(args)...);
}

} // namespace detail

template<typename... Args>
std::string makeString(Args&&... args)
{
    std::stringstream ss;
    detail::makeString(ss, std::forward<Args>(args)...);
    return ss.str();
}

std::string toString(const std::wstring& wstr);
std::wstring toWString(const std::string& str);

std::vector<std::string>
split(const std::string& str, char c);

std::vector<std::string>
split(const std::string& str,
      const std::function<bool(char)>& isSeparator);

std::string readFile(const std::string& path);

std::vector<std::string> split(const std::string& text,
                           const std::string& separator);

std::vector<std::string> split(
        const std::string& text,
        const std::vector<std::string>& separators = { " ", "\t", "\n", "\r" });

static inline std::vector<std::string> split(
        const std::string& text,
        const std::initializer_list<std::string>& separators)
{
    return split(text, std::vector<std::string>(separators));
}

template<typename Iterable>
std::string join(const Iterable& iterable,
                 const std::string& separator)
{
    std::stringstream ss;

    auto it = std::begin(iterable);
    if (it == std::end(iterable)) {
        return "";
    }

    ss << *it++;
    while (it != std::end(iterable)) {
        ss << separator << *it++;
    }

    return ss.str();
}

std::string replaceAll(const std::string &original,
                       const std::string &needle,
                       const std::string &replacement);

namespace detail {

struct FormatArg
{
    size_t startOffset;
    size_t endOffset;
    size_t number;
};

std::string unescape(const std::string& str);

size_t findFirstUnescaped(const std::string& str,
                          char c,
                          size_t offset = 0);

bool findNextFormatArg(const std::string& str,
                       FormatArg& out,
                       size_t offsetStart);

std::vector<FormatArg> parseFormatString(const std::string& format);

} // namespace detail

template<typename... Args>
std::string format(const std::string& format,
                   const Args&... args)
{
    std::vector<std::string> argsAsStrings {
        lexical_cast<std::string>(args)...
    };

    const std::vector<detail::FormatArg> fmtArgs = detail::parseFormatString(format);
    std::stringstream ret;
    size_t lastEnd = 0;

    for (const auto &fmtArg: fmtArgs) {
        if (fmtArg.number >= argsAsStrings.size()) {
            PKE_Assert(false, "insufficient arguments to format(): found index "
                              + lexical_cast<std::string>(fmtArg.number));
            continue;
        }

        size_t chunkLength = fmtArg.startOffset - lastEnd;
        const std::string chunk = format.substr(lastEnd, chunkLength);
        ret << detail::unescape(chunk) << argsAsStrings[fmtArg.number];

        lastEnd = fmtArg.endOffset;
    }

    ret << detail::unescape(format.substr(lastEnd));
    return ret.str();
}

} // namespace utils
} // namespace sb

#endif //STRINGUTILS_H
