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
#include <limits>
#include <iosfwd>
#include <optional>
#include <unordered_map>

constexpr int MAX_MSG_LEN = 8;
constexpr int MAX_BIT_INDEX_uint64_t = (sizeof(uint64_t) * CHAR_BIT) - 1;

enum class ByteOrder {
	NotSet,
	Intel,  // little-endian
	Motorola    // Big-endian
};
enum class ValueType {
	NotSet,
	Signed,
	Unsigned
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
    std::optional<double> getInitialValue() const { return initialValue; }
	ByteOrder getByteOrder() const { return sigByteOrder; }
	ValueType getValueTypes() const { return sigValueType; }
	// Get names of all the nodes that receives this signal
	std::vector<std::string> getReceiversName() const { return receiversName; }
    void setInitialValue(const double& initialValue) { this->initialValue = initialValue; }
    // Decode/Encode
	double decodeSignal(const unsigned char rawPayload[MAX_MSG_LEN], const unsigned int messageSize);
	uint64_t encodeSignal(const double& physicalValue);
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
