/*
 *  dbc_parser.cpp
 *
 *  Created on: 05/04/2023
 *      Author: Yifan Wang
 */

#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "dbc_parser.hpp"

std::ostream& operator<<(std::ostream& os, const DbcParser& dbcFile){
    if (dbcFile.isEmptyLibrary) {
        std::cout << "Empty Library. Load and parse DBC file first." << std::endl;
        return os;
    }
    // Print details for each signal and message
    for (auto message : dbcFile.messagesInfo) {
        std::cout << "-------------------------------" << std::endl;
        std::cout << message->getName() << " " << (*message).getId() << std::endl;
        for (auto& sig : message->getSignalsInfo()) {
            std::cout << "Signal: " << sig.second.getName() << "  ";
            std::cout << sig.second.getStartBit() << "," << sig.second.getSignalSize() << std::endl;
            std::cout << "(" << sig.second.getFactor() << ", " << sig.second.getOffset() << ")" << std::endl;
            std::cout << "[" << sig.second.getMinValue() << "," << sig.second.getMaxValue() << "]" << std::endl;
            if (sig.second.getByteOrder() == ByteOrder::Intel) {
                std::cout << "INTEL" << std::endl;
            }
            else {
                std::cout << "MOTO" << std::endl;
            }
            if (sig.second.getValueTypes() == ValueType::Unsigned) {
                std::cout << "UNSIGNED" << std::endl;
            }
            else {
                std::cout << "SIGNED" << std::endl;
            }
            if (sig.second.getUnit() != "") {
                std::cout << sig.second.getUnit() << std::endl;
            }
            if (sig.second.getInitialValue().has_value()) {
                std::cout << sig.second.getInitialValue().value() << std::endl;
            }
            std::cout << std::endl;
        }
    }
    std::cout << "-------------------------------" << std::endl;
    return os;
}

void DbcParser::loadAndParseFromFile(std::istream& in) {
	std::string lineInitial;
	// Read the file line by line
	while (in >> lineInitial) {
		// Get the first word in the line
		if (lineInitial == "NS_") {
			while (in >> lineInitial && !(lineInitial == "BS_:" || lineInitial == "BS_"));
		}
		// Messages
		if (lineInitial == "BO_") {
			// Parse the message
			Message msg;
			in >> msg;
			// Message name uniqueness check. Message names by definition need to be unqiue within the file
			messageLibrary_iterator data_itr = messageLibrary.find(msg.getId());
			if (data_itr == messageLibrary.end()) {
				// Uniqueness check passed, store the message
				messageLibrary.insert(std::make_pair(msg.getId(), msg));
				data_itr = messageLibrary.find(msg.getId());
				messagesInfo.push_back(&(data_itr->second));
			}
			else {
				throw std::invalid_argument("Message \"" + msg.getName() + "\" has a duplicate. Parse Failed.");
			}
		}
		// Value descriptions
		else if (lineInitial == "VAL_") {
			// There are two types of value descriptions: Environment variable value descriptions and Signal value descriptions
			// Signal value descriptions define encodings for specific signal raw values.
			// Environment variable value descriptions provide textual representations of specific values of the variable.
            // Check "DBC File Format Documentation" if confused
			unsigned int messageId;
			in >> messageId;
			if (messageId != 0) {
				// If there exists a message ID, this is a signal value description
				messageLibrary_iterator messages_itr = messageLibrary.find(messageId);
				if (messages_itr != messageLibrary.end()) {
					// Search for signals to store signal value description
					messages_itr->second.parseSignalValueDescription(in);
				}
				else {
					throw std::invalid_argument("Cannot find message (ID: "
                                                + std::to_string(messageId)
                                                + ") for a given signal value description. Parse Failed.");
				}
			}
		}
        // Attribute definitions
        else if (lineInitial == "BA_DEF_") {
            std::string objectType;
            in >> objectType;
            if (objectType == "SG_") {
                std::string attributeName;
                getline(in, attributeName, '\"');
                getline(in, attributeName, '\"');
                if (attributeName == "GenSigStartValue") {
                    in >> attributeName;
                    in >> sigInitialValueMin;
                    in >> sigInitialValueMax;
                }
            }
        }
        // Attribute defaults
        else if (lineInitial == "BA_DEF_DEF_") {
            std::string attributeName;
            getline(in, attributeName, '\"');
            getline(in, attributeName, '\"');
            if (attributeName == "GenSigStartValue") {
                in >> sigInitialValueDefault;
            }
        }
        // Attribute values
        else if (lineInitial == "BA_") {
            std::string attributeName;
            getline(in, attributeName, '\"');
            getline(in, attributeName, '\"');
            if (attributeName == "GenSigStartValue") {
                std::string objectType;
                in >> objectType;
                if (objectType == "SG_") {
                    unsigned int messageId;
                    in >> messageId;
                    messageLibrary_iterator messages_itr = messageLibrary.find(messageId);
                    if (messages_itr != messageLibrary.end()) {
                        messages_itr->second.parseSignalInitialValue(in);
                    }
                    else {
                        throw std::invalid_argument("Cannot find message (ID: "
                                                    + std::to_string(messageId)
                                                    + ") for a given signal value description. Parse Failed.");
                    }
                }
            }
        }
		else {
			// Reserved cases for parsing other info in the DBC file
		}
		// Skip the rest of the line for uninterested data and make sure we can get a whole new line in the next iteration
		in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
		throw std::invalid_argument("Could not open CAN database file.");
		return false;
	}
	// Parse complete, mark as successful
	dbcFile.close();
	isEmptyLibrary = false;
	return true;
}

// If no specific signal name is requested, decode all signals by default
std::unordered_map<std::string, double> DbcParser::decode(unsigned long msgId, unsigned char payLoad[], unsigned int dlc) {
	messageLibrary_iterator data_itr_msg = messageLibrary.find(msgId);
	std::unordered_map<std::string, double> result;
	if (data_itr_msg == messageLibrary.end()) {
		std::cout << "No matching message found. Decode failed. An empty result is returned.\n" << std::endl;
	}
	else {
		result = messageLibrary[msgId].decode(payLoad, dlc);
	}
	return result;
}

// If specific signal name is requested, decode all signals but only displays decoded value of the requested signal
double DbcParser::decodeSignalOnRequest(unsigned long msgId, unsigned char payLoad[], unsigned int dlc, std::string sigName) {
	messageLibrary_iterator data_itr_msg = messageLibrary.find(msgId);
	if (data_itr_msg == messageLibrary.end()) {
		std::cout << "No matching message found. Decode failed. A NULL is returned.\n" << std::endl;
		return NULL;
	}
	else {
		std::unordered_map<std::string, double> result;
		result = messageLibrary[msgId].decode(payLoad, dlc);
		std::unordered_map<std::string, double>::iterator data_itr_sig = result.find(sigName);
		if (data_itr_sig == result.end()) {
			std::cout << "No matching signal found. Decode failed. A NULL is returned.\n" << std::endl;
			return NULL;
		}
		else {
			// Print decoded signal info
			// std::cout << "  Signal: " << msgName << " " << data_itr->second << std::endl;
			return data_itr_sig->second;
		}
	}
}

unsigned int DbcParser::encode(unsigned long msgId,
							   std::vector<std::pair<std::string, double> > signalsToEncode,
							   unsigned char encodedPayload[MAX_MSG_LEN]) {
	unsigned int dlc = 0;
	messageLibrary_iterator data_itr_msg = messageLibrary.find(msgId);
	if (data_itr_msg == messageLibrary.end()) {
		std::cout << "No matching message found. Encode failed. An empty result is returned.\n" << std::endl;
	}
	else {
		dlc = messageLibrary[msgId].encode(signalsToEncode, encodedPayload);
		// Print encoded message info
        // std::cout << "Encoded message[" << messageLibrary[msgId].getId() << "]: "
        //   << messageLibrary[msgId].getName() << std::endl;
        // for (short int i = 0; i < 8; i++) {
        //     std::cout << std::bitset<sizeof(encodedPayload[i])*8>(encodedPayload[i]) << std::endl;
        // }
	}
	return dlc;
}