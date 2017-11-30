/********************************************************************/
/** string_utils.h by Alex Koukoulas (C) 2017 All Rights Reserved  **/
/** File Description:                                              **/
/********************************************************************/

#pragma once

// Local Headers

// Remote Headers
#include <string>
#include <sstream>
#include <vector>

namespace string_utils
{
	void split(const std::string& s, char delim, std::vector<std::string>& elems);
	std::vector<std::string> split(const std::string& s, char delim);
}