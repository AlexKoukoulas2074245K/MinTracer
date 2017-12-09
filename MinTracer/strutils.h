/********************************************************************/
/** strutils.h by Alex Koukoulas (C) 2017 All Rights Reserved      **/
/** File Description:                                              **/
/********************************************************************/

#pragma once

// Local Headers

// Remote Headers
#include <string>
#include <vector>

namespace strutils
{
	bool startsWith(const std::string& s, const std::string& pattern);
	void split(const std::string& s, char delim, std::vector<std::string>& elems);
	std::vector<std::string> split(const std::string& s, char delim);
}