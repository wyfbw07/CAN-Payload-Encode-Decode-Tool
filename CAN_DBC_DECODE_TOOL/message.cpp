/*
 *  message.cpp
 *
 *  Created on: 04/28/2023
 *      Author: Yifan Wang
 */

#include <fstream>
#include <iostream>

#include "message.hpp"

std::istream& operator>>(std::istream& in, Message& msg) {
	// Read message ID
	in >> msg.id;
	// Read message name, and remove the extranious colon
	std::string name;
	in >> name;
	if (!name.empty()) {
		name.pop_back();
	}
	msg.name = name;
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
            throw std::invalid_argument("Signal \"" + sig.getName() + "\" has duplicates in the same message. Parse Failed.\n");
        }
		// Update stream position after each signal read
		posBeforePeek = in.tellg();
	}
	// End peeking, restore istream position
	in.seekg(posBeforePeek);
	return in;
}

// Create a hash table for all decoded signals
std::unordered_map<std::string, double> Message::decode(std::string payload){
	std::unordered_map<std::string, double> sigValues;
	for (auto& it : signalsLibrary) {
		sigValues.insert(std::pair(it.second.getName(), it.second.getDecodedValue(payload)));
	}
	return sigValues;
}
