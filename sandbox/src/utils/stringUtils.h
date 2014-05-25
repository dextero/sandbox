#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <sstream>
#include <vector>
#include <functional>

#include "utils/types.h"

namespace sb
{
    namespace utils
    {
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
        }

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
    } // namespace utils
} // namespace sb

#endif //STRINGUTILS_H
