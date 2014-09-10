#include "utils/stringUtils.h"
#include "utils/logger.h"
#include "utils/debug.h"

#include <fstream>
#include <algorithm>

namespace sb {
namespace utils {

std::string toString(const std::wstring& wstr)
{
    return std::string(wstr.begin(), wstr.end());
}

std::wstring toWString(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}

std::string strip(const std::string& text,
                  const std::string& chars)
{
    size_t first = text.find_first_not_of(chars);
    if (first == std::string::npos) {
        return "";
    }

    size_t last = text.find_last_not_of(chars);
    if (last == std::string::npos) {
        return "";
    }

    return text.substr(first, last - first + 1);
}

std::string readFile(const std::string& path)
{
    std::ifstream file(path);

    if (!file.is_open()) {
        sbFail("cannot open file: %s", path.c_str());
    }

    file.seekg(0, std::ios::end);
    size_t filesize = file.tellg();

    gLog.debug("reading %lu bytes from file %s\n", filesize, path.c_str());

    std::string contents;
    contents.resize(filesize);
    file.seekg(0, std::ios::beg);

    file.read(&contents[0], contents.size());
    return contents;
}

std::string replaceAll(const std::string &original,
                       const std::string &needle,
                       const std::string &replacement)
{
    std::stringstream ss;
    size_t lastAt = 0;
    size_t at = original.find(needle);

    while(at != std::string::npos)
    {
        ss << original.substr(lastAt, at - lastAt) << replacement;
        lastAt = at + replacement.size();
        at = original.find(needle, lastAt);
    }

    ss << original.substr(lastAt);
    return ss.str();
}

namespace {

size_t findFirstSeparator(const std::string& text,
                          const std::vector<std::string>& separators,
                          size_t start)
{
    size_t at = std::string::npos;

    for (const auto& sep: separators) {
        size_t sepAt = text.find(sep, start);
        if (sepAt < at) {
            at = sepAt;
        }
    }

    return at;
}

size_t skipSeparators(const std::string& text,
                      const std::vector<std::string>& separators,
                      size_t start)
{
    const std::string* nextSep = nullptr;

    do {
        nextSep = nullptr;

        for (const auto& sep: separators) {
            if (start + sep.size() <= text.size()
                    && text.compare(start, sep.size(), sep) == 0
                    && (!nextSep || nextSep->size() < sep.size())) {
                nextSep = &sep;
            }
        }

        if (nextSep) {
            start += nextSep->size();
        }
    } while (nextSep);

    return start;
}

} // namespace

std::vector<std::string> split(const std::string& str, char c)
{
    std::vector<std::string> ret;

    size_t prev = 0, at = (size_t)-1;
    while ((at = str.find(c, at + 1)) != std::string::npos)
    {
        if (at != prev + 1) {
            ret.push_back(str.substr(prev, at - prev));
        }

        prev = at;
    }
    ret.push_back(str.substr(prev));

    return ret;
}

std::vector<std::string> split(const std::string& text,
                               const std::vector<std::string>& separators)
{
    std::vector<std::string> ret;
    size_t lastSeparatorEnd = skipSeparators(text, separators, 0);
    size_t at = findFirstSeparator(text, separators, lastSeparatorEnd);

    while (at != std::string::npos) {
        ret.push_back(text.substr(lastSeparatorEnd, at - lastSeparatorEnd));

        lastSeparatorEnd = skipSeparators(text, separators, at);
        at = findFirstSeparator(text, separators, lastSeparatorEnd);
    }

    if (lastSeparatorEnd < text.size()) {
        ret.push_back(text.substr(lastSeparatorEnd));
    }

    return ret;
}

std::vector<std::string> split(const std::string& str,
                               const std::function<bool(char)>& isSeparator)
{
    std::vector<std::string> ret;

    size_t prev = 0;
    for (size_t at = 0; at < str.size(); ++at) {
        if (!isSeparator(str[at])) {
            continue;
        } else if (prev == at) {
            ++prev;
            continue;
        }

        ret.push_back(str.substr(prev, at - prev));
        prev = at + 1;
    }

    if (prev != str.size()) {
        ret.push_back(str.substr(prev));
    }

    return ret;
}

std::vector<std::string> split(const std::string& text,
                               const std::string& separator)
{
    std::vector<std::string> ret;
    size_t lastSeparatorEnd = 0;
    size_t at = text.find(separator);

    while (at != std::string::npos) {
        ret.push_back(text.substr(lastSeparatorEnd, at - lastSeparatorEnd));
        lastSeparatorEnd = at + separator.size();
        at = text.find(separator, lastSeparatorEnd);
    }

    ret.push_back(text.substr(lastSeparatorEnd));
    return ret;
}

namespace detail {

std::string unescape(const std::string& str)
{
    size_t at = 0;
    size_t next;
    std::stringstream out;

    for (next = str.find('\\');
         next != std::string::npos;
         next = str.find('\\', at)) {
        out << str.substr(at, next - at);
        at = next + 1;
    }

    out << str.substr(at);
    return out.str();
}

size_t findFirstUnescaped(const std::string& str, char c, size_t offset)
{
    std::string charAndBackslash = std::string(&c, 1) + "\\";

    offset = str.find_first_of(charAndBackslash, offset);
    while (offset != std::string::npos
            && str[offset] != c) {
        offset = str.find_first_of(charAndBackslash, offset + 2);
    }

    return offset;
}

bool findNextFormatArg(const std::string& str,
                       FormatArg& out,
                       size_t offsetStart)
{
    size_t start;
    size_t end;
    std::string insideBraces;

    start = findFirstUnescaped(str, '{', offsetStart);
    end = str.find('}', start + 1);

    if (start == std::string::npos
            || end == std::string::npos) {
        return false;
    }

    insideBraces = str.substr(start + 1, end - start - 1);
    
    if (!std::all_of(insideBraces.begin(), insideBraces.end(), isdigit)) {
        sbFail("invalid format specifier: expected a positive integer, got "
               "'%s'", insideBraces.c_str());
    }

    out.startOffset = start;
    out.endOffset = end + 1;
    out.number = lexical_cast<size_t>(insideBraces);
    return true;
}

std::vector<FormatArg> parseFormatString(const std::string& format)
{
    std::vector<FormatArg> ret(1);
    size_t offset = 0;

    while (findNextFormatArg(format, ret.back(), offset)) {
        offset = ret.back().endOffset;
        ret.push_back({});
    }

    ret.pop_back();
    return ret;
}

} // namespace detail
} // namespace utils
} // namespace sb
