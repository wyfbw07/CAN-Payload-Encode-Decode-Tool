/*
 *  dbc_parser.cpp
 *
 *  Created on: 05/04/2023
 *      Author: Yifan Wang
 */

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <limits>
#include "dbc_parser.hpp"

void DbcParser::loadAndParseFromFile(std::istream& in) {
	std::string line;
	// Read the file line by line
	while (std::getline(in, line)) {
		// Get the first word in the line
		std::string lineInitial;
		in >> lineInitial;
		// Look for messages
		if (lineInitial == "BO_") {
			// Parse the message
			Message msg;
			in >> msg;
			// Message name uniqueness check. Message names by definition need to be unqiue within the file
			std::unordered_map<uint32_t, Message>::iterator data_itr = messageLibrary.find(msg.getId());
			if (data_itr == messageLibrary.end()) {
				// Uniqueness check passed, store the message
				messageLibrary.insert(std::make_pair(msg.getId(), msg));
				data_itr = messageLibrary.find(msg.getId());
				messagesInfo.push_back(&(data_itr->second));
			}
			else {
				// Uniqueness check failed, then something must be wrong with the DBC file, parse failed
				throw std::invalid_argument("Message \"" + msg.getName() + "\" has a duplicate. Parse Failed.\n");
			}
		}
		// Look for signal value descriptions
		else if (lineInitial == "VAL_") {
            
		}
		else {
			// Skip the line for uninterested data and make sure we can get a whole new line in next iteration
			in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
}

// Load file from path. Parse and store the content
// A returned bool is used to indicate whether parsing succeeds or not
bool DbcParser::parse(const std::string& filePath) {
	// Get file path, open the file stream
	std::ifstream dbcFile(filePath.c_str(), std::ios::binary);
	if (dbcFile) {
		// Parse file content
		loadAndParseFromFile(dbcFile);
	}
	else {
		throw std::invalid_argument("Could not open DBC file.");
		return false;
	}
	// Parse complete, mark as successful
	dbcFile.close();
	emptyLibrary = false;
	return true;
}

// Displays DBC file info
void DbcParser::printDbcInfo() {
	if (emptyLibrary) {
		std::cout << "Empty Library. Load and parse DBC file first." << std::endl;
		return;
	}
	// Print details for each signal and message
	for (auto message : messagesInfo) {
		std::cout << "-------------------------------" << std::endl;
		std::cout << message->getName() << " " << (*message).getId() << std::endl;
		for (auto& sig : message->getSignalsInfo()) {
			std::cout << "Signal: " << sig.second.getName() << "  ";
			std::cout << sig.second.getStartBit() << "," << sig.second.getDataLength() << std::endl;
			std::cout << "(" << sig.second.getFactor() << ", " << sig.second.getOffset() << ")" << std::endl;
			std::cout << "[" << sig.second.getMinValue() << "," << sig.second.getMaxValue() << "]" << std::endl;
			if (sig.second.getByteOrder() == ByteOrders::Intel) {
				std::cout << "INTEL" << std::endl;
			}
			else {
				std::cout << "MOTO" << std::endl;
			}
			if (sig.second.getValueTypes() == ValueTypes::Unsigned) {
				std::cout << "UNSIGNED" << std::endl;
			}
			else {
				std::cout << "SIGNED" << std::endl;
			}
			if (sig.second.getUnit() != "") {
				std::cout << sig.second.getUnit() << std::endl;
			}
			std::cout << std::endl;
		}
	}
	std::cout << "-------------------------------" << std::endl;
}

// If no specific signal name is requested, decode all signals by default
std::unordered_map<std::string, double> DbcParser::decode(uint32_t msgId, std::string payload) {
    std::unordered_map<uint32_t, Message>::iterator data_itr_msg = messageLibrary.find(msgId);
    std::unordered_map<std::string, double> result;
    if (data_itr_msg == messageLibrary.end()) {
        std::cout << "No matching message found. Decaode failed. An empty result is returned.\n" << std::endl;
    }
    else {
        result = messageLibrary[msgId].decode(payload);
        // Print decoded message info
        // std::cout << "Decoded message[" << messageLibrary[msgId].getId() << "]: " << messageLibrary[msgId].getName() << std::endl;
        // for (auto& decodedSig : result) {
        //       std::cout << "  Signal: " << decodedSig.first << " " << decodedSig.second << std::endl;
        // }
    }
	return result;
}

// If specific signal name is requested, decode all signals but only displays decoded value of the requested signal
double DbcParser::decodeSignalOnRequest(uint32_t msgId, std::string payload, std::string sigName) {
    std::unordered_map<uint32_t, Message>::iterator data_itr_msg = messageLibrary.find(msgId);
    if (data_itr_msg == messageLibrary.end()) {
        std::cout << "No matching message found. Decaode failed. A NULL is returned.\n" << std::endl;
        return NULL;
    }
    else {
        std::unordered_map<std::string, double> result;
        result = messageLibrary[msgId].decode(payload);
        std::unordered_map<std::string, double>::iterator data_itr_sig = result.find(sigName);
        if (data_itr_sig == result.end()) {
            std::cout << "No matching signal found. Decaode failed. A NULL is returned.\n" << std::endl;
            return NULL;
        }
        else {
            // Print decoded signal info
            // std::cout << "  Signal: " << msgName << " " << data_itr->second << std::endl;
            return data_itr_sig->second;
        }
    }
}
