/*
 *  main.cpp
 *
 *  Created on: 05/09/2023
 *      Author: Yifan Wang
 */

#include <iostream>
#include <stdexcept>
#include "dbc_parser.hpp"

 // defining main with arguments
int main(int argc, char* argv[])
{
	int operationChoice = 3;
	// Create a class to store DBC info
	DbcParser dbcFile;
	try {
		// Load file from path. Parse and store the content
		dbcFile.parse("/Users/wyfbw07/Downloads/Test_Database_Files/VehicleSystem.dbc");
		// Display DBC info
		dbcFile.printDbcInfo();
		// MARK: - Function call choices
		switch (operationChoice) {
		case 1:
		{
			// Decode
			int dlc = 8;
			int msgId = 258;
			unsigned char rawPayload[8] = { 0x2d, 0xff, 0xc8, 0x0f, 0x0, 0x0, 0x0, 0x0 };
			std::unordered_map<std::string, double> result = dbcFile.decode(msgId, rawPayload, dlc);
			// Print decoded message info
			std::cout << "Decoded signal values: \n";
			for (auto& decodedSig : result) {
				std::cout << " (Signal) " << decodedSig.first << ": " << decodedSig.second << std::endl;
			}
		}
		break;
		case 2:
		{
			// Decode and show decoded value of requested signal only
			int dlc = 8;
			int msgId = 258;
			unsigned char rawPayload[8] = { 0x2d, 0xff, 0xc8, 0x0f, 0x0, 0x0, 0x0, 0x0 };
			std::string sigName = "EngSpeed";
			double decodedValue = 0;
			decodedValue = dbcFile.decodeSignalOnRequest(msgId, rawPayload, dlc, sigName);
			std::cout << "Decoded signal value: " << decodedValue << '\n';
		}
		break;
		case 3:
		{
			// Encode test case
			int msgId = 258;
			unsigned int encodedDlc = 0;
			unsigned char encodedPayload[8];
			std::vector<std::pair<std::string, double> > signalsToEncode;
			signalsToEncode.push_back(std::make_pair("EngSpeed", 4040));
			signalsToEncode.push_back(std::make_pair("PetrolLevel", 255));
			signalsToEncode.push_back(std::make_pair("EngTemp", 90));
			encodedDlc = dbcFile.encode(msgId, signalsToEncode, encodedPayload);
			// Print results
			std::cout << "Encoded message size: " << encodedDlc << '\n';
			std::cout << "Display encoded payload as array (leftmost is [0]): ";
			for (short i = 0; i < 8; i++) {
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
