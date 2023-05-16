/*
 *  signal.cpp
 *
 *  Created on: 05/06/2023
 *      Author: Yifan Wang
 */

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
	in >> sig.dataLength;
	in.ignore(1);
	// Read signal byte order
	int rawByteOrderValue;
	in >> rawByteOrderValue;
	// 0=big endian, 1=little endian
	if (rawByteOrderValue == 0) {
		sig.byteOrder = ByteOrders::Motorola;
	}
	else if (rawByteOrderValue == 1) {
		sig.byteOrder = ByteOrders::Intel;
	}
	else {
		throw std::invalid_argument("Unable to parse byte order of signal: " + sig.name + "\n");
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
		throw std::invalid_argument("Unable to parse value type of signal: " + sig.name + "\n");
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

double Signal::getDecodedValue(std::string rawPayload){
    
    // Split and remove delimitors
    std::vector<std::string> payload;
    std::string concatenatedPayload;
    unsigned int payloadInDec; unsigned short bit;
    splitWithDeliminators(rawPayload, ',', payload);
    if (byteOrder == ByteOrders::Motorola) {
        for (unsigned long i = 0; i < payload.size(); i++){
            concatenatedPayload += payload[i];
        }
    }else{
        for (unsigned long i = payload.size(); i-- > 0; ){
            concatenatedPayload += payload[i];
        }
    }
    // Reverse bit sequence in big-endian order
    if (byteOrder == ByteOrders::Motorola) {
        std::string payloadInBin;
        payloadInBin = hexToBin(concatenatedPayload);
        std::reverse(payloadInBin.begin(), payloadInBin.end());
        concatenatedPayload = binToHex(payloadInBin);
        bit = startBit - dataLength;
    }else{
        bit = startBit;
    }
    // Convert to DEC
    std::istringstream converter(concatenatedPayload);
    converter >> std::hex >> payloadInDec;
    // Decode
    int64_t result = 0;
    uint8_t *data = (uint8_t *)&payloadInDec;
    for (int bitpos = 0; bitpos < dataLength; bitpos++) {
        if (data[bit / 8] & (1 << (bit % 8))) {
            if (byteOrder == ByteOrders::Intel) {
                result |= (1ULL << bitpos);
            } else {
                result |= (1ULL << (dataLength - bitpos - 1));
            }
        }
        bit++;
    }
    if ((valueType == ValueTypes::Signed) && (result & (1ULL << (dataLength - 1)))) {
        result |= ~((1ULL << dataLength) - 1);
    }
    return (double)result*factor + offset;
    
}
