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
#include "dbc_parser_dependencies/message.hpp"

class DbcParser {

public:

	// Construct using either a File or a Stream of a DBC-File
	// A bool is used to indicate whether parsing succeeds or not
	bool parse(const std::string& filePath);
	// Decode
	std::unordered_map<std::string, double> decode(unsigned long msgId, unsigned char payLoad[], unsigned int dlc);
	double decodeSignalOnRequest(unsigned long msgId, unsigned char payLoad[], unsigned int dlc, std::string sigName);
	// Encode
	unsigned int encode(unsigned long msgId,
						std::vector<std::pair<std::string, double> >& signalsToEncode,
						unsigned char encodedPayload[MAX_MSG_LEN]);
    // Print DBC Info
    friend std::ostream& operator<<(std::ostream& os, const DbcParser& dbcFile);

private:

	typedef std::unordered_map<unsigned long, Message>::iterator messageLibrary_iterator;
    // Default value Belongs to attribute name "GenSigStartValue"
    double sigGlobalInitialValue; // BA_DEF_DEF_  "GenSigStartValue"
    double sigGlobalInitialValueMin; // BA_DEF_ SG_  "GenSigStartValue"
    double sigGlobalInitialValueMax; // BA_DEF_ SG_  "GenSigStartValue"
	// This list contains all the messages which got parsed from the DBC-File
    std::vector<Message*> messagesInfo;
    // A bool to indicate whether DBC file has been loaded or not
    bool isEmptyLibrary = true;
	// Stores all info of messages. <Message id, Message object>
	std::unordered_map<unsigned long, Message> messageLibrary;
	// Function used to parse DBC file
	void loadAndParseFromFile(std::istream& in);
    void consistencyCheck();

};

#endif
