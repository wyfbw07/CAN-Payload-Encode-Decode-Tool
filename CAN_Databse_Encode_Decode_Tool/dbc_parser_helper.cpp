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

// Convert a string that contains hexadeciaml into binary
std::string DbcParserHelper::hexToBin(const std::string& s) {
	std::string out;
	// Bitwise operation
	for (auto i : s) {
		uint8_t n;
		if (i <= '9' and i >= '0')
			n = i - '0';
		else
			n = 10 + i - 'A';
		for (int8_t j = 3; j >= 0; --j)
			out.push_back((n & (1 << j)) ? '1' : '0');
	}
	return out;
}

// Convert a string that contains binary into hexadecimal
std::string DbcParserHelper::binToHex(const std::string& s) {
	std::string out;
	// Bitwise operation
	for (unsigned int i = 0; i < s.size(); i += 4) {
		int8_t n = 0;
		for (unsigned int j = i; j < i + 4; ++j) {
			n <<= 1;
			if (s[j] == '1')
				n |= 1;
		}
		if (n <= 9)
			out.push_back('0' + n);
		else
			out.push_back('A' + n - 10);
	}
	return out;
}