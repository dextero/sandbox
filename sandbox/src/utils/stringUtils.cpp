#include "stringUtils.h"

namespace sb
{
    namespace StringUtils
    {
        const std::string ToString(const std::wstring& wstr)
        {
            return std::string(wstr.begin(), wstr.end());
        }

        const std::wstring ToWString(const std::string& str)
        {
            return std::wstring(str.begin(), str.end());
        }

        std::vector<std::string> Split(const std::string& str, char c)
        {
            std::vector<std::string> ret;

            size_t prev = 0, at = (size_t)-1;
            while ((at = str.find(c, at + 1)) != std::string::npos)
            {
                ret.push_back(str.substr(prev, at - prev));
                prev = at;
            }
            ret.push_back(str.substr(prev));

            return ret;
        }
    } // namespace StringUtils
} // namespace sb
