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

constexpr unsigned short MAX_MSG_LEN_CAN = 8;
constexpr unsigned short MAX_MSG_LEN_CAN_FD = 64;

enum class BusType {
    NotSet,
    Unknown,
    CAN,
    CAN_FD
};

class DbcParser {

public:

    // Construct using either a File or a Stream of a DBC-File
    // A bool is used to indicate whether parsing succeeds or not
    bool parse(const std::string& filePath);
    // Decode
    std::unordered_map<std::string, double> decode(
        unsigned long msgId,
        unsigned int msgSize,
        unsigned char payLoad[]
    );
    // Encode
    unsigned int encode(
        unsigned long msgId,
        std::vector<std::pair<std::string, double> >& signalsToEncode,
        unsigned char encodedPayload[],
        unsigned int encodedPayloadSize
    );
    // Print DBC Info
    friend std::ostream& operator<<(std::ostream& os, const DbcParser& dbcFile);

private:

    typedef std::unordered_map<unsigned long, Message>::iterator messageLibrary_iterator;
    bool isEmptyLibrary = true; // A bool to indicate whether DBC file has been loaded or not
    double sigGlobalInitialValue; // BA_DEF_DEF_  "GenSigStartValue"
    double sigGlobalInitialValueMin; // BA_DEF_ SG_  "GenSigStartValue"
    double sigGlobalInitialValueMax; // BA_DEF_ SG_  "GenSigStartValue"
    BusType databaseBusType = BusType::NotSet; // CAN or CAN FD
    // Stores all info of messages. <Message id, Message object>
    std::unordered_map<unsigned long, Message> messageLibrary;
    // Contains all the messages which got parsed from the DBC-File
    std::vector<Message*> messagesInfo;
    // Function used to parse DBC file
    void loadAndParseFromFile(std::istream& in);
    void consistencyCheck();

};

#endif
