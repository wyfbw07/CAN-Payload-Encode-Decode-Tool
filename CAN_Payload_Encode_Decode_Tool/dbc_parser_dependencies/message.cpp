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

std::istream& Message::parseSigInitialValue(std::istream& in) {
    // Read signal name
    std::string sigName;
    in >> sigName;
    // Find the signal
    signalsLibrary_iterator signals_itr = signalsLibrary.find(sigName);
    if (signals_itr != signalsLibrary.end()) {
        // Read and set the initial value for that signal
        double initialValue;
        in >> initialValue;
        signals_itr->second.setInitialValue(initialValue);
    }
    else {
        throw std::invalid_argument("Parse failed during parsing signal's initial value. "
                                    "Cannot find signal: " + sigName + " in CAN database.");
    }
    return in;
}

std::istream& Message::parseSigValueDescription(std::istream& in) {
    std::string sigName;
    in >> sigName;
    // Search for corresponding signal to parse value descriptions
    signalsLibrary_iterator signals_itr = signalsLibrary.find(sigName);
    if (signals_itr != signalsLibrary.end()) {
        signals_itr->second.parseSignalValueDescription(in);
    }
    else {
        throw std::invalid_argument("Parse failed during parsing signal value description. "
                                    "Cannot find signal: " + sigName + " in CAN database.");
    }
    return in;
}

std::istream& Message::parseAdditionalSigValueType(std::istream& in) {
    std::string sigName = utils::getline(in, ':');
    int sigValueTypeIdentifier;
    in >> sigValueTypeIdentifier;   // (1 = IEEE Float, 2 = IEEE Double)
    // Find the signal
    signalsLibrary_iterator signals_itr = signalsLibrary.find(sigName);
    if (signals_itr != signalsLibrary.end()) {
        // Set the value type
        signals_itr->second.setSigValueType(sigValueTypeIdentifier);
    }
    else {
        throw std::invalid_argument("Parse failed during parsing signal's value type. "
                                    "Cannot find signal: " + sigName + " in CAN database.");
    }
    return in;
}

std::unordered_map<std::string, double> Message::decode(
    unsigned char const rawPayload[],
    unsigned short const MAX_MSG_LEN,
    unsigned int const msgSize) {
    // Check input payload length
    if (msgSize != messageSize) {
        throw std::invalid_argument("Decode failed. The data length of the input payload does not match with DBC info.");
    }
    // Decode
    std::unordered_map<std::string, double> sigValues;
    for (auto& it : signalsLibrary) {
        sigValues.insert(std::make_pair(it.second.getName(), it.second.decodeSignal(rawPayload, MAX_MSG_LEN, messageSize)));
    }
    return sigValues;
}

unsigned int Message::encode(
    std::vector<std::pair<std::string, double> >& signalsToEncode,
    unsigned char encodedPayload[],
    unsigned short const MAX_MSG_LEN,
    const double defaultGlobalInitialValue) {
    for (size_t i = 0; i < MAX_MSG_LEN; i++) {
        encodedPayload[i] = 0;
    }
    // Check if all signals are valid under the message
    for (unsigned short i = 0; i < signalsToEncode.size(); i++) {
        signalsLibrary_iterator signals_itr = signalsLibrary.find(signalsToEncode[i].first);
        if (signals_itr == signalsLibrary.end()) {
            throw std::invalid_argument("Encode failed. Cannot find signal: " + signalsToEncode[i].first + " in CAN database.");
        }
        unsigned int rawValue = (signalsToEncode[i].second - signals_itr->second.getOffset()) / signals_itr->second.getFactor();
        // Check if the provided value is within its min and max range
        if (!(rawValue <= signals_itr->second.getMaxValue()
            && rawValue >= signals_itr->second.getMinValue())) {
            std::cerr << "<Warning> Trying to encode a value that is out of the min and max range of signal "
                      << std::quoted(name) << " is not allowed. This signal will encode with its initial value: "
                      << signals_itr->second.getInitialValue().value_or(defaultGlobalInitialValue) << '.' << std::endl;
            // DBC stores initial values as raw values, so convert to initial physical value
            double initialPhysicalValue = signals_itr->second.getInitialValue().value_or(defaultGlobalInitialValue)
                                        * signals_itr->second.getFactor()
                                        + signals_itr->second.getOffset();
            // Override the initial raw value
            signalsToEncode[i].second = initialPhysicalValue;
        }
    }
    // Find the signal, then encode
    for (auto& sig : signalsLibrary) {
        bool hasValuetoEncode = false;
        unsigned char encodedPayloadOfSingleSig[MAX_MSG_LEN];
        // Explicitly clear and set to 0 (recessive bit) to the array before encode
        for (size_t i = 0; i < MAX_MSG_LEN; i++) {
            encodedPayloadOfSingleSig[i] = 0;
        }
        // If value is provided, encode it
        for (unsigned short i = 0; i < signalsToEncode.size(); i++) {
            if (signalsToEncode[i].first == sig.first) {
                hasValuetoEncode = true;
                // Encode the requested value
                sig.second.encodeSignal(signalsToEncode[i].second,
                                        encodedPayloadOfSingleSig,
                                        MAX_MSG_LEN);
                break;
            }
        }
        // If no value is provided, use initial (default) values
        // If the signal does not have a initial value, use the global initial value
        if (!hasValuetoEncode) {
            double initialPhysicalValue = sig.second.getInitialValue().value_or(defaultGlobalInitialValue)
                                        * sig.second.getFactor()
                                        + sig.second.getOffset();
            // Encode with initial value
            sig.second.encodeSignal(initialPhysicalValue,
                                    encodedPayloadOfSingleSig,
                                    MAX_MSG_LEN);
        }
        // Merge single result with results from other signals
        for(size_t i = 0; i < MAX_MSG_LEN; i++){
            encodedPayload[i] |= encodedPayloadOfSingleSig[i];
        }
    }
    return messageSize;
}

std::istream& operator>>(std::istream& in, Message& msg) {
    // Read message ID
    in >> msg.id;
    // Read message name
    // Use getline since there could be two formats:
    // "Signal_name :" or "Signal_name:"
    std::string name = utils::getline(in, ':');
    msg.name = name;
    // Read message data length
    in >> msg.messageSize;
    // Read message sender name
    in >> msg.senderName;
    // Save istream positon before peeking next word
    std::streampos posBeforePeek = in.tellg();
    // Peek and look for signals under this message
    std::string preamble;
    while (in >> preamble && preamble == "SG_") {
        // Read signal info
        Signal sig;
        in >> sig;
        // Signal name uniqueness check. Signal names by definition need to be unqiue within each message
        Message::signalsLibrary_iterator data_itr = msg.signalsLibrary.find(sig.getName());
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
