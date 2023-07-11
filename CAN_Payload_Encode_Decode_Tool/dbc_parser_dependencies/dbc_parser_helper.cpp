/*
 *  dbc_parser_helper.cpp
 *
 *  Created on: 05/04/2023
 *      Author: Yifan Wang
 */

#include <string>
#include <sstream>
#include <algorithm>
#include "dbc_parser_helper.hpp"

 // Trim out the first and last character from a string
std::string& DbcParserHelper::trimLeadingAndTrailingChar(std::string& str, const char& delim) {
	str.erase(remove(str.begin(), str.end(), delim), str.end());
	return str;
}

// Split a string into separate elements and store it into a vector
std::vector<std::string>& DbcParserHelper::splitWithDeliminators(const std::string& str,
																 char delim,
																 std::vector<std::string>& elems) {
	std::stringstream tempStream(str);
	std::string item;
	while (std::getline(tempStream, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}
