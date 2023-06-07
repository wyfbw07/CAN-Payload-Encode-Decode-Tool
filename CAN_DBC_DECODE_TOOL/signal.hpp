/*
 *  signal.hpp
 *
 *  Created on: 04/27/2023
 *      Author: Yifan Wang
 */

#ifndef SIGNAL_H
#define SIGNAL_H

#include <string>
#include <vector>
#include <iosfwd>
#include "dbc_parser_helper.hpp"

enum class ByteOrders {
	NotSet,
	Intel,
	Motorola
};
enum class ValueTypes {
	NotSet,
	Signed,
	Unsigned
};

// Respresents a signal. Contains all signal info.
class Signal : DbcParserHelper {

public:

	// Get signal name and unit of the signal payload in string
	std::string getName() const { return name; }
	std::string getUnit() const { return unit; }
	// Get factor and offset
	double getFactor() const { return factor; }
	double getOffset() const { return offset; }
	// Get max and min possible value of the signal payload
	double getMinValue() const { return minValue; }
	double getMaxValue() const { return maxValue; }
	// Get start bit and data length
	unsigned short getStartBit() const { return startBit; }
	unsigned short getDataLength() const { return signalSize; }
	// Get byte order: Intel (little-endian) or Motorola (Big-endian)
	ByteOrders getByteOrder() const { return byteOrder; }
	ValueTypes getValueTypes() const { return valueType; }
	// Get names of all the nodes that receives this signal
	std::vector<std::string> getReceiversName() const { return receiversName; }
    // Convert the signals raw value into the signal's physical value and vice versa
	double decodeSignal(unsigned char rawPayload[], unsigned int messageSize);
    uint64_t encodeSignal(double& physicalValue);
    // Parse signal value descrption
    std::istream& parseSignalValueDescription(std::istream& in);
	// Operator overload, allows parsing of signals info
	friend std::istream& operator>>(std::istream& in, Signal& sig);

private:

    typedef std::unordered_map<double, std::string>::iterator valueDescriptions_iterator;
	// Name of the signal
	std::string name{};
	// Represents the physical unit of the signal, which is a string type
	std::string unit{};
	// Two variables: factor and offset
	// Used to convert between the original value of the signal and the physical value
	// Conversion formula: physical value = original value * factor + offset
	double factor{};
	double offset{};
	// Specifies the range of the signal value; these two values are of type double
	double maxValue{};
	double minValue{};
	// Signal start bit
	unsigned int startBit{};
	// The signal_size specifies the size of the signal in bits
	unsigned int signalSize{};
	// Byte order can be either Intel (little-endian) or Motorola (Big-endian)
	ByteOrders byteOrder = ByteOrders::NotSet;
	// Value order can be either unsigned or signed
	ValueTypes valueType = ValueTypes::NotSet;
	// Names of all the nodes that receives this signal
	std::vector<std::string> receiversName{};
    // Signal value descriptions: define encodings for specific signal raw values
    std::unordered_map<double, std::string> valueDescriptions;
	// Helper functions that are essential to parsing
    std::vector<std::string>& splitWithDeliminators(const std::string& str,
                                                    char delimiter,
                                                    std::vector<std::string>& elems) {
		DbcParserHelper::splitWithDeliminators(str, delimiter, elems);
        return elems;
	}
    std::string& trimLeadingAndTrailingChar(std::string& str, const char& toTrim) {
        return DbcParserHelper::trimLeadingAndTrailingChar(str, toTrim);
    }
	std::string binToHex(const std::string& s) {
		return DbcParserHelper::binToHex(s);
	}
	std::string hexToBin(const std::string& s) {
		return DbcParserHelper::hexToBin(s);
	}
};

#endif /* SIGNAL_H */
