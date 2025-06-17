#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <cctype>

namespace mymake {
namespace helper {

inline std::string trim(const std::string& str)
{
	std::string s(str);
	auto notisspace = [](int c){ return !std::isspace(c); };
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), notisspace));
	s.erase(std::find_if(s.rbegin(), s.rend(), notisspace).base(), s.end());
	return s;
}

inline std::vector<std::string> split(const std::string& str)
{
	const std::string s(trim(str));
	if (s.empty())
		return {};

	std::vector<std::string> toks;
	auto it = s.cbegin();
	while (it != s.cend()) {
		auto end = std::find_if(it, s.cend(), ::isspace);
		toks.emplace_back(it, end);
		it = std::find_if(end, s.cend(), [](int c){ return !std::isspace(c); });
	}
	return toks;
}

} // namespace helper
} // namespace mymake
