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
//#include "universal_typedefs.hpp"


class Message : DbcParserHelper {

	typedef std::vector<Signal*> signals_t;
	// Name of the Message
	std::string name{};
	// The CAN-ID assigned to this specific Message
	int id{};
	// The length of this message in Bytes. Allowed values are between 0 and 8
	unsigned int dataLength{};
	// String containing the name of the Sender of this Message if one exists in the DB
	std::string senderName{};
	// A hash table containing all Signals that are present in this Message
	std::unordered_map<std::string, Signal> signalsLibrary{};

public:

	// Overload of operator>> to enable parsing of Messages from streams of DBC-Files
	friend std::istream& operator>>(std::istream& in, Message& msg);
	// Getter functions for all the possible data one can request from a Message
	std::string getName() const { return name; }
	std::uint32_t getId() const { return id; }
	std::size_t getDlc() const { return dataLength; }
	std::string getSenderNames() const { return senderName; }
    std::unordered_map<std::string, Signal> getSignalsInfo() const { return signalsLibrary; }
	// Used to decode messages
	std::unordered_map<std::string, double> decode(std::string payload);

};
#endif
