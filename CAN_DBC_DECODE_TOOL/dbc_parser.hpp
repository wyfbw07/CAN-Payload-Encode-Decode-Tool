/*
 *  dbc_parser.hpp
 *
 *  Created on: 04/28/2023
 *      Author: Yifan Wang
 */

#ifndef DBCPARSER_HPP
#define DBCPARSER_HPP

#include <iosfwd>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "message.hpp"

class DbcParser {

public:

	// Construct using either a File or a Stream of a DBC-File
	// A bool is used to indicate whether parsing succeeds or not
	bool parse(const std::string& filePath);
	// Decode
	std::unordered_map<std::string, double> decode(uint32_t msgId, unsigned char payLoad[], unsigned short dlc);
	double decodeSignalOnRequest(uint32_t msgId, unsigned char payLoad[], unsigned short dlc, std::string sigName);
	// Print DBC Info
	void printDbcInfo();

private:

    typedef std::unordered_map<uint32_t, Message>::iterator messageLibrary_iterator;
	// A bool to indicate whether DBC file has been loaded or not
	bool emptyLibrary = true;
	// This list contains all the messages which got parsed from the DBC-File
	typedef std::vector<Message*> messages_t;
	messages_t messagesInfo;
	// A hash table that stores all info of messages. <Message id, Message object>
	std::unordered_map<uint32_t, Message> messageLibrary;
	// Function used to parse DBC file
	void loadAndParseFromFile(std::istream& in);

};

#endif
