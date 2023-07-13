/*
 *  message.cpp
 *
 *  Created on: 04/28/2023
 *      Author: Yifan Wang
 */

#include <iomanip>
#include <iostream>
#include <sstream>
#include "message.hpp"

std::istream& operator>>(std::istream& in, Message& msg) {
	// Read message ID
	in >> msg.id;
	// Read message name
	std::string name;
	in >> name;
	char lastCharOnName = name.back();
	if (lastCharOnName == ':') {
		// For old format, just remove the trailing colon
		if (!name.empty()) {
			name.pop_back();
		}
		msg.name = name;
	}
	else {
		// For new format, read the name directly and let the stream consume the colon
		msg.name = name;
		in >> name;
	}
	// Read message data length
	in >> msg.messageSize;
	// Read message sender name
	in >> msg.senderName;
	// Save istream positon before peeking next word
	std::streampos posBeforePeek = in.tellg();
	// Look for signals under this message
	std::string preamble;
	while (in >> preamble && preamble == "SG_") {
		// Read signal info
		Signal sig;
		in >> sig;
		// Signal name uniqueness check. Signal names by definition need to be unqiue within each message
		std::unordered_map<std::string, Signal>::iterator data_itr = msg.signalsLibrary.find(sig.getName());
		if (data_itr == msg.signalsLibrary.end()) {
			// Uniqueness check passed, store the signal
			msg.signalsLibrary.insert(std::make_pair(sig.getName(), sig));
		}
		else {
			// Uniqueness check failed, then something must be wrong with the DBC file, parse failed
			throw std::invalid_argument("Parse Failed. Signal \"" + sig.getName()
                                        + "\" has duplicates in the same message.");
		}
		// Update stream position after each signal read
		posBeforePeek = in.tellg();
	}
	// End peeking, restore istream position
	in.seekg(posBeforePeek);
	return in;
}

std::istream& Message::parseSignalValueDescription(std::istream& in) {
    // Search for corresponding signal to parse the value descriptions
    std::string sigName;
    in >> sigName;
    signalsLibrary_iterator signals_itr = signalsLibrary.find(sigName);
    if (signals_itr != signalsLibrary.end()) {
        signals_itr->second.parseSignalValueDescription(in);
    }
    else {
        throw std::invalid_argument("Parse failed during parsing signal value description. Cannot find signal: "
                                    + sigName + " in CAN database.");
    }
    return in;
}

std::istream& Message::parseSignalInitialValue(std::istream& in) {
    std::string sigName;
    in >> sigName;
    signalsLibrary_iterator signals_itr = signalsLibrary.find(sigName);
    if (signals_itr != signalsLibrary.end()) {
        double initialValue;
        in >> initialValue;
        signals_itr->second.setInitialValue(initialValue);
    }
    else {
        throw std::invalid_argument("Parse failed during parsing signal's initial value. Cannot find signal: "
                                    + sigName + " in CAN database.");
    }
    return in;
}

// Create a hash table for all decoded signals
std::unordered_map<std::string, double> Message::decode(const unsigned char rawPayload[MAX_MSG_LEN],
                                                        const unsigned int dlc) {
	// Check input payload length
	// If the length of the input payload is different than what is required by the DBC file,
	// reject and fail the decode operation
	if (dlc != messageSize) {
		throw std::invalid_argument("Decode failed. The data length of the input payload does not match with DBC info.");
	}
	// Decode
	std::unordered_map<std::string, double> sigValues;
	for (auto& it : signalsLibrary) {
		sigValues.insert(std::make_pair(it.second.getName(), it.second.decodeSignal(rawPayload, messageSize)));
	}
	return sigValues;
}

unsigned int Message::encode(std::vector<std::pair<std::string, double> >& signalsToEncode,
                             const double defaultGlobalInitialValue,
                             unsigned char encodedPayload[]) {
    // Check all signals are valid under the message
	uint64_t encodedValue = 0;
	for (unsigned short i = 0; i < signalsToEncode.size(); i++) {
        signalsLibrary_iterator signals_itr = signalsLibrary.find(signalsToEncode[i].first);
        if (signals_itr == signalsLibrary.end()) {
            throw std::invalid_argument("Encode failed. Cannot find signal: "
                                        + signalsToEncode[i].first + " in CAN database.");
        }
        unsigned int rawValue = (signalsToEncode[i].second - signals_itr->second.getOffset()) / signals_itr->second.getFactor();
        if (!(rawValue <= signals_itr->second.getMaxValue()
              && rawValue >= signals_itr->second.getMinValue())) {
            std::cerr << "<Warning> Trying to encode a value that is out of the min and max range of signal "
            << std::quoted(name) << " is not allowed. This signal will encode with its initial value: "
            << signals_itr->second.getInitialValue().value_or(defaultGlobalInitialValue) << '.' << std::endl;
            signalsToEncode[i].second = signals_itr->second.getInitialValue().value_or(defaultGlobalInitialValue);
        }
    }
    // Find the signal, then encode
    for (auto& sig : signalsLibrary) {
        bool hasValuetoEncode = false;
        uint64_t encodedValueOfSingleSignal = 0;
        for (unsigned short i = 0; i < signalsToEncode.size(); i++) {
            if (signalsToEncode[i].first == sig.first) {
                hasValuetoEncode = true;
                encodedValueOfSingleSignal = sig.second.encodeSignal(signalsToEncode[i].second);
                break;
            }
        }
        // If no value is provided, use initial (default) values
        // If the signal does not have a initial value, use the global initial value
        if (!hasValuetoEncode) {
            encodedValueOfSingleSignal = sig.second.encodeSignal(sig.second.getInitialValue().value_or(defaultGlobalInitialValue));
        }
        // Store encoded result with results from other signals
        encodedValue |= encodedValueOfSingleSignal;
    }
    // Convert encodedValue into byte array
	for (short i = MAX_MSG_LEN - 1; i >= 0; i--) {
		encodedPayload[i] = encodedValue % 256; // get the last byte
		encodedValue /= 256; // get the remainder
	}
	return messageSize;
}
