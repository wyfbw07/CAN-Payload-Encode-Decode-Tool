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

std::ostream& operator<<(std::ostream& os, const DbcParser& dbcFile) {
    if (dbcFile.isEmptyLibrary) {
        std::cout << "Empty Library. Load and parse DBC file first." << std::endl;
        return os;
    }
    // Print details for each signal and message
    for (auto message : dbcFile.messagesInfo) {
        std::cout << "-------------------------------" << std::endl;
        std::cout << "<Message> "
                  << message->getName()
                  << " "
                  << (*message).getId()
                  << " "
                  << (*message).getDlc()
                  << std::endl;
        for (auto& sig : message->getSignalsInfo()) {
            std::cout << "<Signal> "
                      << sig.second.getName()
                      << "  "
                      << std::endl;
            std::cout << "\t\tStart bit/Sig size: "
                      << sig.second.getStartBit()
                      << ","
                      << sig.second.getSignalSize()
                      << std::endl;
            std::cout << "\t\tFactor/Offset: ("
                      << sig.second.getFactor()
                      << ", "
                      << sig.second.getOffset()
                      << ")" << std::endl;
            std::cout << "\t\tMin/Max: ["
                      << sig.second.getMinValue()
                      << ","
                      << sig.second.getMaxValue()
                      << "]"
                      << std::endl;
            if (sig.second.getByteOrder() == ByteOrder::Intel) {
                std::cout << "\t\tINTEL" << std::endl;
            }
            else { std::cout << "\t\tMOTO" << std::endl; }
            switch (sig.second.getValueTypes()) {
            case ValueType::Unsigned:
                std::cout << "\t\tUNSIGNED" << std::endl;
                break;
            case ValueType::Signed:
                std::cout << "\t\tSIGNED" << std::endl;
                break;
            case ValueType::IeeeFloat:
                std::cout << "\t\tIEEE Float" << std::endl;
                break;
            case ValueType::IeeeDouble:
                std::cout << "\t\tIEEE Double" << std::endl;
                break;
            default:
                break;
            }
            if (sig.second.getUnit() != "") {
                std::cout << "\t\t" << sig.second.getUnit() << std::endl;
            }
            if (sig.second.getInitialValue().has_value()) {
                std::cout << "\t\tInitial value: "
                          << sig.second.getInitialValue().value()
                          << std::endl;
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
                throw std::invalid_argument("Parse Failed. Message \"" + msg.getName() + "\" has a duplicate.");
            }
        }
        // Value descriptions
        else if (lineInitial == "VAL_") {
            // There are two types of value descriptions: Environment variable value descriptions and Signal value descriptions
            // Environment variable value descriptions provide textual representations of specific values of the variable.
            // Signal value descriptions define encodings for specific signal raw values.
            // Check "DBC File Format Documentation" if confused
            unsigned int messageId;
            in >> messageId;
            if (messageId != 0) {
                // If there exists a message ID, this is a signal value description
                messageLibrary_iterator messages_itr = messageLibrary.find(messageId);
                if (messages_itr != messageLibrary.end()) {
                    // Search for signals to store signal value description
                    messages_itr->second.parseSigValueDescription(in);
                }
                else {
                    throw std::invalid_argument("Parse Failed. Cannot find message (ID: "
                        + std::to_string(messageId)
                        + ") for a given signal value description.");
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
                    in >> sigGlobalInitialValueMin;
                    in >> sigGlobalInitialValueMax;
                }
            }
        }
        // Attribute defaults
        else if (lineInitial == "BA_DEF_DEF_") {
            std::string attributeName;
            getline(in, attributeName, '\"');
            getline(in, attributeName, '\"');
            if (attributeName == "GenSigStartValue") {
                in >> sigGlobalInitialValue;
            }
        }
        // Attribute values
        else if (lineInitial == "BA_") {
            std::string attributeName;
            getline(in, attributeName, '\"');
            getline(in, attributeName, '\"');
            // Detect bus type
            if (attributeName == "BusType") {
                std::string busTypeName;
                getline(in, busTypeName, '\"');
                getline(in, busTypeName, '\"');
                if (busTypeName == "CAN") {
                    databaseBusType = BusType::CAN;
                }
                else if (busTypeName == "CAN FD") {
                    databaseBusType = BusType::CAN_FD;
                }
                else {
                    databaseBusType = BusType::Unknown;
                    throw std::invalid_argument("Parse Failed. Unknown bus type.");
                }
            }
            // Signal specific initial values
            if (attributeName == "GenSigStartValue") {
                std::string objectType;
                in >> objectType;
                if (objectType == "SG_") {
                    unsigned int messageId;
                    in >> messageId;
                    messageLibrary_iterator messages_itr = messageLibrary.find(messageId);
                    if (messages_itr != messageLibrary.end()) {
                        messages_itr->second.parseSigInitialValue(in);
                    }
                    else {
                        throw std::invalid_argument("Parse Failed. Cannot find message (ID: "
                            + std::to_string(messageId)
                            + ") for a given signal value description.");
                    }
                }
            }
        }
        // Additional signal value type
        else if (lineInitial == "SIG_VALTYPE_") {
            unsigned int messageId;
            in >> messageId;
            messageLibrary_iterator messages_itr = messageLibrary.find(messageId);
            if (messages_itr != messageLibrary.end()) {
                messages_itr->second.parseAdditionalSigValueType(in);
            }
            else {
                throw std::invalid_argument("Parse Failed. Cannot find message (ID: "
                    + std::to_string(messageId)
                    + ") for a given signal value type attribute.");
            }
        }
        else {
            // Reserved cases for parsing other info in the DBC file
        }
        // Skip the rest of the line for uninterested data and make sure we can get a whole new line in the next iteration
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    isEmptyLibrary = false;
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
        throw std::invalid_argument("Parse Failed. Could not open CAN database file.");
        return false;
    }
    consistencyCheck();
    dbcFile.close();
    return true;
}

void DbcParser::consistencyCheck() {
    if (!((sigGlobalInitialValue <= sigGlobalInitialValueMax)
        && (sigGlobalInitialValue >= sigGlobalInitialValueMin))
        && !(sigGlobalInitialValueMax == 0 && sigGlobalInitialValueMin == 0)) {
        throw std::invalid_argument("<Consistency check> Default signal initial value is not within its min and max range.");
    }
    for (auto message : messageLibrary) {
        for (auto& sig : message.second.getSignalsInfo()) {
            if (sig.second.getInitialValue().has_value()) {
                if (!((sig.second.getInitialValue().value() <= sig.second.getMaxValue())
                    && (sig.second.getInitialValue().value() >= sig.second.getMinValue()))) {
                    // Refer to attribute BA_ "GenSigStartValue" SG_ in DBC file
                    throw std::invalid_argument("<Consistency check> Signal initial value is not within min and max range of signal \""
                        + sig.second.getName() + "\".");
                }
            }
            else {
                if (!((sigGlobalInitialValue <= sig.second.getMaxValue())
                    && (sigGlobalInitialValue >= sig.second.getMinValue()))) {
                    // Refer to attribute BA_DEF_DEF_  "GenSigStartValue" in DBC file
                    // This value should usually be 0 to avoid this warning
                    throw std::invalid_argument("<Consistency check> Global signal initial value is not within min and max range of signal \""
                        + sig.second.getName() + "\".");
                }
            }
        }
    }
}

// If no specific signal name is requested, decode all signals by default
std::unordered_map<std::string, double> DbcParser::decode(
    unsigned long msgId,
    unsigned int msgSize,
    unsigned char payload[]) {
    
    std::unordered_map<std::string, double> result;
    messageLibrary_iterator data_itr_msg = messageLibrary.find(msgId);
    if (data_itr_msg == messageLibrary.end()) {
        std::cerr << "Decode failed. No matching message found. An empty result is returned." << std::endl;
    }
    else {
        if (databaseBusType == BusType::CAN) {
            result = messageLibrary[msgId].decode(payload,
                                                  MAX_MSG_LEN_CAN,
                                                  msgSize);
        }
        else if (databaseBusType == BusType::CAN_FD) {
            result = messageLibrary[msgId].decode(payload,
                                                  MAX_MSG_LEN_CAN_FD,
                                                  msgSize);
        }
        else {
            std::cerr << "Decode failed. Unknown bus type. An empty result is returned." << std::endl;
        }
    }
    return result;
}

unsigned int DbcParser::encode(
    unsigned long msgId,
    std::vector<std::pair<std::string, double> >& signalsToEncode,
    unsigned char encodedPayload[],
    unsigned int encodedPayloadSize) {
    
    // Find the message and encode based on bus type
    unsigned int msgSize = 0;
    messageLibrary_iterator data_itr_msg = messageLibrary.find(msgId);
    if (data_itr_msg == messageLibrary.end()) {
        std::cerr << "Encode failed. No matching message found."
                  << "An empty result is returned." << std::endl;
    }
    else {
        if (databaseBusType == BusType::CAN) {
            msgSize = messageLibrary[msgId].encode(signalsToEncode,
                                                   encodedPayload,
                                                   MAX_MSG_LEN_CAN,
                                                   sigGlobalInitialValue);
        }
        else if (databaseBusType == BusType::CAN_FD) {
            msgSize = messageLibrary[msgId].encode(signalsToEncode,
                                                   encodedPayload,
                                                   MAX_MSG_LEN_CAN_FD,
                                                   sigGlobalInitialValue);
        }
        else {
            std::cerr << "Encode failed. Unknown bus type."
                      << "An empty result is returned." << std::endl;
            return msgSize;
        }
    }
    if (msgSize > encodedPayloadSize) {
        std::cerr << "The provided array size is smaller than message size "
                  << "and may not be enough to hold encoded values. "
                  << "Results could be truncated." << std::endl;
    }
    return msgSize;
}
