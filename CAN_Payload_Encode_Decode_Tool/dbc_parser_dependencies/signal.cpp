/*
 *  signal.cpp
 *
 *  Created on: 05/06/2023
 *      Author: Yifan Wang
 */

#include <bitset>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "signal.hpp"

std::istream& operator>>(std::istream& in, Signal& sig) {
    // Read signal name
    in >> sig.name;
    // A deliminator that is useless
    std::string rawString;
    in >> rawString;
    if (rawString != ":") {
        sig.sigSignalType = SignalType::Multiplexed;
        // in >> rawString; // Comment the exception and uncomment this line if you still want to proceed.
        throw std::invalid_argument("Parse failed. Signal \"" + sig.name + "\" may not be a normal signal."
                                    + " Note: This tool does not currently support parsing multiplexer and multiplexed signals.");
    }
    else {
        sig.sigSignalType = SignalType::Normal;
    }
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
        sig.sigByteOrder = ByteOrder::Motorola;
    }
    else if (rawByteOrderValue == 1) {
        sig.sigByteOrder = ByteOrder::Intel;
    }
    else {
        throw std::invalid_argument("Parse failed. Unable to parse byte order of signal \"" + sig.name + "\".");
    }
    // Read value type
    char rawChar;
    in >> rawChar;
    if (rawChar == '+') {
        sig.sigValueType = ValueType::Unsigned;
    }
    else if (rawChar == '-') {
        sig.sigValueType = ValueType::Signed;
    }
    else {
        throw std::invalid_argument("Parse failed. Unable to parse value type of signal \"" + sig.name + "\".");
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
    getline(in, rawString, '\"');
    getline(in, rawString, '\"');
    sig.unit = rawString;
    // Read destination nodes
    in >> rawString;
    if (rawString != "Vector__XXX") {
        std::stringstream tempStream(rawString);
        std::string item;
        while (std::getline(tempStream, item, ',')) {
            sig.receiversName.push_back(item);
        }
    }
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return in;
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
            throw std::invalid_argument("Parse failed. Found duplicated value description of signal \"" + name + "\".");
        }
    }
    return in;
}

double Signal::decodeSignal(const unsigned char rawPayload[MAX_MSG_LEN], const unsigned int messageSize) {
    int64_t decodedValue = 0;
    uint16_t currentBit = 0;
    // Intel
    if (sigByteOrder == ByteOrder::Intel) {
        // Change endianness
        int64_t payload = 0;
        for (int i = MAX_MSG_LEN; i > 0; i--) {
            payload <<= 8;
            payload |= (uint64_t)rawPayload[i - 1];
        }
        // Decode
        uint8_t* data = (uint8_t*)&payload;
        currentBit = startBit;
        // Access the corresponding byte and make sure we are reading a bit that is 1
        for (unsigned short bitpos = 0; bitpos < signalSize; bitpos++) {
            if (data[currentBit / CHAR_BIT] & (1 << (currentBit % CHAR_BIT))) {
                // Add dominant bit
                if (sigByteOrder == ByteOrder::Intel) {
                    decodedValue |= (1ULL << bitpos);
                }
            }
            currentBit++;
        }
    }
    // Motorola MSB
    else {
        // Translate start bit into sequential order
        unsigned int translationFactor = 1;
        unsigned int translationOffset = 0;
        translationFactor = (startBit / CHAR_BIT);
        translationOffset = (CHAR_BIT - (startBit) % CHAR_BIT - 1);
        currentBit = translationFactor * CHAR_BIT + translationOffset;
        // Decode
        for (unsigned short bitpos = 0; bitpos < signalSize; bitpos++) {
            if (rawPayload[currentBit / CHAR_BIT] & (1 << ((MAX_BIT_INDEX_uint64_t - currentBit) % CHAR_BIT))) {
                // Add dominant bit
                decodedValue |= (1ULL << (signalSize - bitpos - 1));
            }
            currentBit++;
        }
    }
    // Sign extend based on signal value type
    if ((sigValueType == ValueType::Signed) && (decodedValue & (1ULL << (signalSize - 1)))) {
        decodedValue |= ~((1ULL << signalSize) - 1);
    }
    return (double)decodedValue * factor + offset;
}

uint64_t Signal::encodeSignal(const double& physicalValue) {
    // Reverse linear conversion rule
    // to convert the signals physical value into the signal's raw value
    uint16_t currentBit = 0;
    uint64_t encodedValue = 0;
    int64_t rawValue = (physicalValue - offset) / factor;
    uint8_t* rawPayload = (uint8_t*)&rawValue;
    if (sigByteOrder == ByteOrder::Intel) { // Intel
        // Encode
        for (unsigned short bitpos = 0; bitpos < signalSize; bitpos++) {
            // Access the corresponding byte and make sure we are reading a bit that is 1
            if (rawPayload[currentBit / CHAR_BIT] & (1 << (currentBit % CHAR_BIT))) {
                // Add dominant bit
                encodedValue |= (1ULL << (bitpos + startBit));
            }
            currentBit++;
        }
        // Change endianness
        unsigned char encodedPayload[MAX_MSG_LEN];
        for (short i = MAX_MSG_LEN - 1; i >= 0; i--) {
            encodedPayload[i] = encodedValue % 256; // get the last byte
            encodedValue /= 256; // get the remainder
        }
        encodedValue = 0;
        for (int i = MAX_MSG_LEN; i > 0; i--) {
            encodedValue <<= 8;
            encodedValue |= (uint64_t)encodedPayload[i - 1];
        }
    }
    else { // Motorola MSB
        // Translate start bit into sequential order
        unsigned int translationFactor = 1; // Factor CANNOT default to 0
        unsigned int translationOffset = 0;
        unsigned int translatedstartBit = 0;
        translationFactor = (startBit / CHAR_BIT);
        translationOffset = (CHAR_BIT - (startBit) % CHAR_BIT - 1);
        translatedstartBit = translationFactor * CHAR_BIT + translationOffset;
        // Access the corresponding byte and make sure we are reading a bit that is 1
        for (unsigned short bitpos = 0; bitpos < signalSize; bitpos++) {
            if (rawPayload[currentBit / CHAR_BIT] & (1 << (currentBit % CHAR_BIT))) {
                // Add dominant bit
                encodedValue |= (1ULL << (bitpos + (MAX_BIT_INDEX_uint64_t - (translatedstartBit + signalSize - 1))));
                // std::cout << " " << std::bitset<64>(encodedValue) << std::endl;
            }
            currentBit++;
        }
    }
    return encodedValue;
}
