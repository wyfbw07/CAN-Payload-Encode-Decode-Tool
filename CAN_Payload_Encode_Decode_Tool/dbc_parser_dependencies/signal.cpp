/*
 *  signal.cpp
 *
 *  Created on: 05/06/2023
 *      Author: Yifan Wang
 */
#include <iostream>
#include <limits>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "signal.hpp"
#include "pack754.h"

void Signal::setSigValueType(const int sigValueTypeIdentifier) {
    if (sigValueTypeIdentifier == 1) {
        sigValueType = ValueType::IeeeFloat;
    }
    else if (sigValueTypeIdentifier == 2) {
        sigValueType = ValueType::IeeeDouble;
    }
    else {
        throw std::invalid_argument("Parse failed. "
            "Undefined signal value type identifier for signal \""
            + name
            + "\".");
    }
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
            throw std::invalid_argument("Parse failed. "
                "Found duplicated value description of signal \""
                + name
                + "\".");
        }
    }
    return in;
}

double Signal::decodeSignal(
    unsigned char const rawPayload[],
    unsigned short const MAX_MSG_LEN,
    unsigned int const messageSize) {
    int64_t decodedBitSequence = 0;
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
                decodedBitSequence |= (1ULL << bitpos);
            }
            currentBit++;
        }
    }
    // Motorola MSB
    else {
        // Translate start bit into Intel
        unsigned int translationFactor = 1;
        unsigned int translationOffset = 0;
        translationFactor = (startBit / CHAR_BIT);
        translationOffset = (CHAR_BIT - (startBit) % CHAR_BIT - 1);
        currentBit = translationFactor * CHAR_BIT + translationOffset;
        // Decode
        for (unsigned short bitpos = 0; bitpos < signalSize; bitpos++) {
            if (rawPayload[currentBit / CHAR_BIT] & (1 << ((MAX_BIT_INDEX_uint64_t - currentBit) % CHAR_BIT))) {
                // Add dominant bit
                decodedBitSequence |= (1ULL << (signalSize - bitpos - 1));
            }
            currentBit++;
        }
    }

    double decodedValue = 0;
    if ((sigValueType == ValueType::Signed) && (decodedBitSequence & (1ULL << (signalSize - 1)))) {
        // Sign extend for signed signal values
        decodedBitSequence |= ~((1ULL << signalSize) - 1);
        decodedValue = (double)decodedBitSequence * factor + offset;
    }
    else if (sigValueType == ValueType::IeeeDouble) {
        // Unpack the number from IEEE-754 format
        decodedValue = unpack754_64(decodedBitSequence) * factor + offset;
    }
    else if (sigValueType == ValueType::IeeeFloat) {
        // Unpack the number from IEEE-754 format
        decodedValue = unpack754_32(decodedBitSequence) * factor + offset;
    }
    else {
        decodedValue = (double)decodedBitSequence * factor + offset;
    }
    return decodedValue;
}

void Signal::encodeSignal(
    const double physicalValue,
    unsigned char encodedPayload[],
    unsigned short const MAX_MSG_LEN) {
    int64_t rawValue = 0;
    if (sigValueType == ValueType::IeeeDouble) {
        // Pack a floating point number into IEEE-754 format
        rawValue = pack754_64((physicalValue - offset) / factor);
    }
    else if (sigValueType == ValueType::IeeeFloat) {
        rawValue = pack754_32((physicalValue - offset) / factor);
    }
    else {
        rawValue = static_cast<int64_t>((physicalValue - offset) / factor);
    }
    uint8_t* rawPayload = (uint8_t*)&rawValue;
    if (sigByteOrder == ByteOrder::Intel) { // Intel
        uint16_t currentRawBit = 0;
        // Encode
        for (unsigned short bitPosIndex = 0; bitPosIndex < signalSize; bitPosIndex++) {
            // Access the corresponding byte and make sure we are reading a bit that is 1
            if (rawPayload[currentRawBit / CHAR_BIT] & (1 << (currentRawBit % CHAR_BIT))) {
                // Add dominant bit
                uint16_t encodeBitPosition = bitPosIndex + startBit;
                encodedPayload[encodeBitPosition / CHAR_BIT] |= (1ULL << (encodeBitPosition % CHAR_BIT));
            }
            currentRawBit++;
        }
    }
    else { // Motorola Forward MSB
        // Translate Motorola Forward MSB start bit into Motorola Sequential start bit
        unsigned int transFactor = (startBit / CHAR_BIT);
        unsigned int transOffset = (CHAR_BIT - startBit % CHAR_BIT - 1);
        unsigned int translatedstartBit = transFactor * CHAR_BIT + transOffset;
        uint16_t currentRawBit = signalSize - 1; // Start from MSB
        for (unsigned short bitPosIndex = 0; bitPosIndex < signalSize; bitPosIndex++) {
            // Access the corresponding byte and make sure we are reading a bit that is 1
            if (rawPayload[currentRawBit / CHAR_BIT] & (1 << (currentRawBit % CHAR_BIT))) {
                // Translate back from Motorola Sequential to Motorola MSB
                uint16_t encodeBitPosition = bitPosIndex + translatedstartBit;
                unsigned int transBckFactor = encodeBitPosition / CHAR_BIT;
                unsigned int transBckOffset = encodeBitPosition - ((transBckFactor) * 8 - 1) - 1;
                unsigned int translatedBitPos = (((transBckFactor + 1) * CHAR_BIT) - 1) - transBckOffset;
                // Write the bit into array
                encodedPayload[translatedBitPos / CHAR_BIT] |= (1ULL << (translatedBitPos % CHAR_BIT));
            }
            currentRawBit--;
        }
    }
}

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
            " Note: This tool does not currently support parsing multiplexer and multiplexed signals.");
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
    // (0 = big endian, 1 = little endian)
    if (rawByteOrderValue == 0) { sig.sigByteOrder = ByteOrder::Motorola; }
    else if (rawByteOrderValue == 1) { sig.sigByteOrder = ByteOrder::Intel; }
    else {
        throw std::invalid_argument("Parse failed. Unable to parse byte order "
            "of signal \"" + sig.name + "\".");
    }
    // Read value type
    char rawChar;
    in >> rawChar;
    if (rawChar == '+') { sig.sigValueType = ValueType::Unsigned; }
    else if (rawChar == '-') { sig.sigValueType = ValueType::Signed; }
    else {
        throw std::invalid_argument("Parse failed. Unable to parse value type "
            "of signal \"" + sig.name + "\".");
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
