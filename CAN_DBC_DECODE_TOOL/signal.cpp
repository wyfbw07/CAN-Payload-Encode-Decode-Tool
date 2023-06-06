/*
 *  signal.cpp
 *
 *  Created on: 05/06/2023
 *      Author: Yifan Wang
 */

#include <iomanip>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include "signal.hpp"

std::istream& operator>>(std::istream& in, Signal& sig) {
	// Read signal name
	in >> sig.name;
	// A deliminator that is useless
	std::string rawString;
	in >> rawString;
	// Read start bit, signal size, byte order and value type 
	in >> sig.startBit;
	in.ignore(1);
	in >> sig.signalSize;
	in.ignore(1);
	// Read signal byte order
	short rawByteOrderValue;
	in >> rawByteOrderValue;
	// 0=big endian, 1=little endian
	if (rawByteOrderValue == 0) {
		sig.byteOrder = ByteOrders::Motorola;
	}
	else if (rawByteOrderValue == 1) {
		sig.byteOrder = ByteOrders::Intel;
	}
	else {
		throw std::invalid_argument("Unable to parse byte order of signal: " + sig.name + ". Parse failed.");
	}
	// Read value type
	char rawChar;
	in >> rawChar;
	if (rawChar == '+') {
		sig.valueType = ValueTypes::Unsigned;
	}
	else if (rawChar == '-') {
		sig.valueType = ValueTypes::Signed;
	}
	else {
		throw std::invalid_argument("Unable to parse value type of signal: " + sig.name + ". Parse failed.");
	}
	// Read factor and offset
	in.ignore(2);
	in >> sig.factor;
	in.ignore(1);
	in >> sig.offset;
	// Read min and max value
	in.ignore(3);
	in >> sig.minValue;
	in.ignore(1);
	in >> sig.maxValue;
	in.ignore(1);
	// Read unit, if there exist one
	in >> rawString;
	if (rawString != "\"\"") {
		sig.unit = sig.trimLeadingAndTrailingChar(rawString, '\"');
	}
	// Read destination nodes
	in >> rawString;
	if (rawString != "Vector__XXX") {
		sig.splitWithDeliminators(rawString, ',', sig.receiversName);
	}
	return in;
}

double Signal::getDecodedValue(std::vector<std::string> rawPayload) {
	// Split and remove delimitors
	std::string concatenatedPayload;
	unsigned int payload; unsigned short bit;
	// Concatenate data bytes based on different byte order
	if (byteOrder == ByteOrders::Motorola) {
		for (unsigned long i = 0; i < rawPayload.size(); i++) {
			concatenatedPayload += rawPayload[i];
		}
        concatenatedPayload = hexToBin(concatenatedPayload);
        std::reverse(concatenatedPayload.begin(), concatenatedPayload.end());
        concatenatedPayload = binToHex(concatenatedPayload);
		bit = startBit - signalSize;
	}
	else {
		for (unsigned long i = rawPayload.size(); i-- > 0; ) {
			concatenatedPayload += rawPayload[i];
		}
		bit = startBit;
	}
	// Convert to DEC
	std::istringstream converter(concatenatedPayload);
	converter >> std::hex >> payload;
	// Decode
	int64_t decodedResult = 0;
	uint8_t* data = (uint8_t*)&payload;
    // Access the corresponding byte and make sure we are reading a bit that is 1
	for (int bitpos = 0; bitpos < signalSize; bitpos++) {
		if (data[bit / 8] & (1 << (bit % 8))) {
            // Add dominant bit
			if (byteOrder == ByteOrders::Intel) {
				decodedResult |= (1ULL << bitpos);
			}
			else {
				decodedResult |= (1ULL << (signalSize - bitpos - 1));
			}
		}
		bit++;
	}
	if ((valueType == ValueTypes::Signed) && (decodedResult & (1ULL << (signalSize - 1)))) {
		decodedResult |= ~((1ULL << signalSize) - 1);
	}
	return (double)decodedResult * factor + offset;
}

std::istream& Signal::parseSignalValueDescription(std::istream& in) {
    std::string preamble;
    // Parse signal value descriptions unitl hit the end of the line
    while (in >> preamble && preamble != ";") {
        // Get description for each signal value
        double sigValue = std::stod(preamble);
        std::string rawDescription;
        // Remove double quotes
        in >> std::quoted(rawDescription);
        // Uniqueness check
        valueDescriptions_iterator description_itr = valueDescriptions.find(sigValue);
        if (description_itr == valueDescriptions.end()) {
            // Store signal value description
            valueDescriptions.insert(std::make_pair(sigValue, rawDescription));
        }
        else {
            throw std::invalid_argument("Found duplicated value description of signal: " + name + ". Parse failed.");
        }
    }
    return in;
}
