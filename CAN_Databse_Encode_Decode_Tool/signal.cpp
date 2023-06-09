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

 // Cross platform definition of byteswap
#ifdef _MSC_VER

#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)

#elif defined(__GNUC__)

#define bswap_32(x) __builtin_bswap32(x)
#define bswap_64(x) __builtin_bswap64(x)

#elif defined(__APPLE__)

// Mac OS X / Darwin features
#include <libkern/OSByteOrder.h>
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)

#elif defined(__sun) || defined(sun)

#include <sys/byteorder.h>
#define bswap_32(x) BSWAP_32(x)
#define bswap_64(x) BSWAP_64(x)

#elif defined(__FreeBSD__)

#include <sys/endian.h>
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)

#elif defined(__OpenBSD__)

#include <sys/types.h>
#define bswap_32(x) swap32(x)
#define bswap_64(x) swap64(x)

#elif defined(__NetBSD__)

#include <sys/types.h>
#include <machine/bswap.h>
#if defined(__BSWAP_RENAME) && !defined(__bswap_32)
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)
#endif

#else

#include <byteswap.h>

#endif

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

double Signal::decodeSignal(unsigned char rawPayload[MAX_MSG_LEN], unsigned int messageSize) {
    int64_t decodedValue = 0;
    unsigned int currentBit = 0;
    // Intel
    if (byteOrder == ByteOrders::Intel) {
        // Concatenate data bytes
        // Convert each unsigned char into string
        int64_t payload = 0;
        std::string concatenatedPayload;
        std::vector<std::string> literalPayload;
        for (unsigned short i = 0; i < messageSize; i++) {
            std::ostringstream convertor;
            convertor << std::hex << (0xFF & rawPayload[i]);
            literalPayload.push_back(convertor.str());
        }
        for (size_t i = literalPayload.size(); i-- > 0; ) {
            concatenatedPayload += literalPayload[i];
        }
        // Convert to DEC
        std::istringstream converter(concatenatedPayload);
        converter >> std::hex >> payload;
        // Decode
        // std::cout << "" << std::bitset<sizeof(payload)*8>(payload) << std::endl;
        uint8_t* data = (uint8_t*)&payload;
        currentBit = startBit;
        // Access the corresponding byte and make sure we are reading a bit that is 1
        for (unsigned short bitpos = 0; bitpos < signalSize; bitpos++) {
            if (data[currentBit / CHAR_BIT] & (1 << (currentBit % CHAR_BIT))) {
                // Add dominant bit
                if (byteOrder == ByteOrders::Intel) {
                    decodedValue |= (1ULL << bitpos);
                }
                else {
                    decodedValue |= (1ULL << (signalSize - bitpos - 1));
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
    return (double)decodedValue * factor + offset;
}

uint64_t Signal::encodeSignal(double& physicalValue) {
    // Reverse linear conversion rule
    // to convert the signals physical value into the signal's raw value
    unsigned int currentBit = 0;
    uint64_t encodedValue = 0;
    unsigned int rawValue = (physicalValue - offset) / factor;
    uint8_t* rawPayload = (uint8_t*)&rawValue;
    if (byteOrder == ByteOrders::Intel) { // Intel
        // Encode
        for (unsigned short bitpos = 0; bitpos < signalSize; bitpos++) {
            // Access the corresponding byte and make sure we are reading a bit that is 1
            if (rawPayload[currentBit / CHAR_BIT] & (1 << (currentBit % CHAR_BIT))) {
                // Add dominant bit
                encodedValue |= (1ULL << (bitpos + startBit));
            }
            currentBit++;
        }
        encodedValue = bswap_64(encodedValue);
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
