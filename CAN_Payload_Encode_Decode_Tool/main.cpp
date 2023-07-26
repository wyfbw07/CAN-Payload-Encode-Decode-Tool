/*
 *  main.cpp
 *
 *  Created on: 05/09/2023
 *      Author: Yifan Wang
 */

#include <iostream>
#include <stdexcept>
#include "dbc_parser.hpp"

int main()
{
	int operationChoice = 2;
	// Create a class to store DBC info
	DbcParser dbcFile;
	try {
		// Load file from path. Parse and store the content
        dbcFile.parse("/Users/wyfbw07/Downloads/Volvo/Test_Database_Files/exampleCAN_medium.dbc");
		// dbcFile.parse("/Users/wyfbw07/Downloads/Volvo/Test_Database_Files/exampleCAN_large.dbc");
		// Display DBC info
        std::cout << dbcFile;
		// MARK: - Function call choices
		switch (operationChoice) {
		case 1:
		{
			// Decode
			int msgSize = 32;
            int msgId = 0x88;
            unsigned char rawPayload[] = { 0x0, 0x10, 0x48, 0x7e, 0x5c, 0x80, 0x0, 0x0};
			std::unordered_map<std::string, double> result = dbcFile.decode(msgId, msgSize, rawPayload);
			// Print decoded message info
			std::cout << "Decoded signal values: \n";
			for (auto& decodedSig : result) {
				std::cout << " (Signal) " << decodedSig.first << ": " << decodedSig.second << std::endl;
			}
		}
		break;
		case 2:
		{
			// Encode test case
            int msgId = 0x88;
            unsigned int MAX_MSG_LEN = 32;
			unsigned char encodedPayload[MAX_MSG_LEN];
			std::vector<std::pair<std::string, double> > signalsToEncode;
            unsigned int encodedMsgSize = dbcFile.encode(msgId, signalsToEncode, encodedPayload, MAX_MSG_LEN);
			// Print results
			std::cout << "Encoded message size: " << encodedMsgSize << '\n';
			std::cout << "Display encoded payload as array (leftmost is [0]): ";
			for (short i = 0; i < MAX_MSG_LEN; i++) {
				printf("%x ", encodedPayload[i]);
			}
			std::cout << std::endl;
		}
		break;
		default:
			break;
		}
	}
	catch (std::invalid_argument& err) {
		std::cout << "[Exception catched] " << err.what() << '\n';
	}
	std::cout << "--------------END--------------" << std::endl;
	return 0;
}
