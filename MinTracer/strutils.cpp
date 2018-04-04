/*********************************************************************/
/** strutils.cpp by Alex Koukoulas (C) 2017 All Rights Reserved     **/
/** File Description: Implementation of string helper functions     **/
/*********************************************************************/

// Local Headers
#include "strutils.h"

// Remote Headers
#include <sstream>

bool strutils::startsWith(const std::string& s, const std::string& pattern)
{
	if (pattern.size() > s.size())
		return false;

	const auto patternLen = pattern.size();
	for (auto i = 0U; i < patternLen; ++i)
	{
		if (s[i] != pattern[i])
			return false;
	}

	return true;
}

void strutils::split(const std::string& s, char delim, std::vector<std::string>& elems)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		elems.push_back(item);
	}
}

std::vector<std::string> strutils::split(const std::string& s, char delim)
{
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}