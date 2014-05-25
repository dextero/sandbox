#include "utils/stringUtils.h"

#include <fstream>

namespace sb
{
    namespace utils
    {
        std::string toString(const std::wstring& wstr)
        {
            return std::string(wstr.begin(), wstr.end());
        }

        std::wstring toWString(const std::string& str)
        {
            return std::wstring(str.begin(), str.end());
        }

        std::vector<std::string> split(const std::string& str, char c)
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

        std::string readFile(const std::string& path)
        {
            std::ifstream file(path);
            std::string contents;

            if (file) {
                file.seekg(0, std::ios::end);
                contents.resize(file.tellg());
                file.seekg(0, std::ios::beg);

                file.read(&contents[0], contents.size());
            }

            return contents;
        }
    } // namespace utils
} // namespace sb
