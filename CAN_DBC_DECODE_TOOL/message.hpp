/*
 *  message.hpp
 *
 *  Created on: 04/28/2023
 *      Author: Yifan Wang
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <set>
#include <string>
#include <vector>
#include <iosfwd>
#include <cstdint>
#include <unordered_map>
#include "signal.hpp"

class Message : DbcParserHelper {

	typedef std::vector<Signal*> signals_t;
    typedef std::unordered_map<std::string, Signal>::iterator signalsLibrary_iterator;
	// Name of the Message
	std::string name{};
	// The CAN-ID assigned to this specific Message
    unsigned int id{};
	// The length of this message in Bytes. Allowed values are between 0 and 8
	unsigned int messageSize{};
	// String containing the name of the Sender of this Message if one exists in the DB
	std::string senderName{};
	// A hash table containing all Signals that are present in this Message
	std::unordered_map<std::string, Signal> signalsLibrary{};

public:

	// Overload of operator>> to enable parsing of Messages from streams of DBC-Files
	friend std::istream& operator>>(std::istream& in, Message& msg);
	// Getter functions for all the possible data one can request from a Message
    unsigned int getId() const { return id; }
    unsigned int getDlc() const { return messageSize; }
	std::string getName() const { return name; }
    std::string getSenderNames() const { return senderName; }
	std::unordered_map<std::string, Signal> getSignalsInfo() const { return signalsLibrary; }
    // Parse signal value descrption
    std::istream& parseSignalValueDescription(std::istream& in);
	// Used to decode messages
	std::unordered_map<std::string, double> decode(unsigned char rawPayload[], unsigned int dlc);

};
#endif
