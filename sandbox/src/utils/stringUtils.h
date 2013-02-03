#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <sstream>
#include <vector>
#include <cml/vector.h>

namespace sb
{
	namespace StringUtils
	{
		template<typename T> const std::string ToString(T elem)
		{
			std::stringstream ss;
			ss << elem;
			return ss.str();
		}
	
		template<typename T> const std::wstring ToWString(T elem)
		{
			std::wstringstream wss;
			wss << elem;
			return wss.str();
		}
	
		template<typename T> const std::string ToString(const cml::vector<T, cml::fixed<3, -1> >& v)
		{
			std::stringstream ss;
			ss << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
			return ss.str();
		}
	
		template<typename T> const std::wstring ToWString(const cml::vector<T, cml::fixed<3, -1> >& v)
		{
			std::wstringstream wss;
			wss << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
			return wss.str();
		}

		const std::string ToString(const std::wstring& wstr);
		const std::wstring ToWString(const std::string& str);
	
		std::vector<std::string> Split(const std::string& str, char c);
	} // namespace StringUtils
} // namespace sb
	
#endif //STRINGUTILS_H
