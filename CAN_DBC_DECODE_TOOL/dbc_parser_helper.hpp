/*
 *  dbc_parser_helper.hpp
 *
 *  Created on: 04/28/2023
 *      Author: Yifan Wang
 */

#ifndef DBC_PARSER_HELPER_H
#define DBC_PARSER_HELPER_H

#include <string>
#include <vector>

class DbcParserHelper {

protected:
    
    // Hex and Binary converter
    std::string hexToBin(const std::string &s);
    std::string binToHex(const std::string &s);
    
	// Removes the leading and trailing characters of a string
	static std::string& trimLeadingAndTrailingChar(std::string& str, const char& toTrim);

	// Separate a string into a vector of string using a char delimiter
	static void splitWithDeliminators(const std::string& str,
                                      char delimiter,
					                  std::vector<std::string>& elems);

};
#endif
