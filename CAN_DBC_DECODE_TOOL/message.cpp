/*
 *  message.cpp
 *
 *  Created on: 04/28/2023
 *      Author: Yifan Wang
 */

#include <fstream>
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
	in >> msg.dataLength;
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
			throw std::invalid_argument("Signal \"" + sig.getName() + "\" has duplicates in the same message. Parse Failed.");
		}
		// Update stream position after each signal read
		posBeforePeek = in.tellg();
	}
	// End peeking, restore istream position
	in.seekg(posBeforePeek);
	return in;
}

// Create a hash table for all decoded signals
std::unordered_map<std::string, double> Message::decode(unsigned char rawPayload[], unsigned short dlc) {
	// Check input payload length
	// If the length of the input payload is different than what is required by the DBC file,
	// reject and fail the decode operation
    if (dlc != dataLength) {
        throw std::invalid_argument("The data length of the input payload does not match with DBC info. Decode failed.");
    }
    // Convert each unsigned char into string
    std::vector<std::string> payload;
    for (unsigned short i = 0; i < dlc; i++){
        std::ostringstream convertor;
        convertor << std::hex << (0xFF & rawPayload[i]);
        payload.push_back(convertor.str());
    }
    // Decode
	std::unordered_map<std::string, double> sigValues;
	for (auto& it : signalsLibrary) {
		sigValues.insert(std::make_pair(it.second.getName(), it.second.getDecodedValue(payload)));
	}
	return sigValues;
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
        throw std::invalid_argument("Cannot find signal: " + name + ". Parse failed.");
    }
    return in;
}
