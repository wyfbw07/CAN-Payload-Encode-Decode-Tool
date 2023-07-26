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
#include <optional>
#include <unordered_map>
#include "dbc_parser_helper.hpp"

unsigned short maxMsgLen = 8;   // Can be changed based on bus type
constexpr int MAX_BIT_INDEX_uint64_t = (sizeof(uint64_t) * CHAR_BIT) - 1;

enum class ByteOrder {
	NotSet,
	Intel,  // little-endian
	Motorola    // Big-endian
};
enum class ValueType {
	NotSet,
	Signed,
	Unsigned,
    IeeeFloat,
    IeeeDouble
};
enum class SignalType {
    NotSet,
    Normal,
    Multiplexed
};

class Signal {

public:
    
	std::string getName() const { return name; }
	std::string getUnit() const { return unit; }
	double getFactor() const { return factor; }
	double getOffset() const { return offset; }
	double getMinValue() const { return minValue; }
	double getMaxValue() const { return maxValue; }
    unsigned int getStartBit() const { return startBit; }
    unsigned int getSignalSize() const { return signalSize; }
    ByteOrder getByteOrder() const { return sigByteOrder; }
    ValueType getValueTypes() const { return sigValueType; }
    std::optional<double> getInitialValue() const { return initialValue; }
	// Get names of all the nodes that receives this signal
	std::vector<std::string> getReceiversName() const { return receiversName; }
    void setInitialValue(const double& initialValue) { this->initialValue = initialValue; }
    void setSigValueType(const int sigValueTypeIdentifier);
    // Decode/Encode
	double decodeSignal(unsigned char const rawPayload[],
                        unsigned short const MAX_MSG_LEN,
                        unsigned int const messageSize);
	void encodeSignal(const double physicalValue,
                          unsigned char encodedPayload[],
                          unsigned short const MAX_MSG_LEN);
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
	// Specifies the range of the signal value
	double maxValue{};
	double minValue{};
	// Signal start bit
	unsigned int startBit{};
	// The signal_size specifies the size of the signal in bits
	unsigned int signalSize{};
    // The default RAW value for the signal if no value is provided upon encoding
    std::optional<double> initialValue{};
	// Byte order can be either Intel (little-endian) or Motorola (Big-endian)
	ByteOrder sigByteOrder = ByteOrder::NotSet;
	// Value order can be either unsigned or signed
	ValueType sigValueType = ValueType::NotSet;
    SignalType sigSignalType = SignalType::NotSet;
	// Names of all the nodes that receives this signal
	std::vector<std::string> receiversName{};
	// Signal value descriptions: define encodings for specific signal raw values
	// <physical value, label of the value>
	std::unordered_map<double, std::string> valueDescriptions;
};

#endif /* SIGNAL_H */
