/*
 *  main.cpp
 *
 *  Created on: 05/09/2023
 *      Author: Yifan Wang
 */

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include "dbc_parser.hpp"

// defining main with arguments
int main(int argc, char* argv[])
{
    // MARK: - Properties Section
    int dlc = 8;
    int msgId = 258;
    unsigned char rawPayload[8] = {0x2d, 0xff, 0xf0, 0xf, 0x0, 0x0, 0x0, 0x0};
    std::string sigName = "EngineTemp";
    // Create a class to store DBC info
    DbcParser dbcFile;
    try {
        // Load file from path. Parse and store the content
        dbcFile.parse("/Users/wyfbw07/Downloads/Test_DBC_Files/VehicleSystem_Format2.dbc");
        // Display DBC info
        dbcFile.printDbcInfo();
        // MARK: - Function call choices
        int choice = 1;
        switch (choice) {
            case 1:
                dbcFile.decode(msgId, rawPayload, dlc);
                break;
            case 2:
                dbcFile.decodeSignalOnRequest(msgId, rawPayload, dlc, sigName);
                break;
            default:
                break;
        }
    }
    catch (std::invalid_argument& err) {
        std::cout << "Exception catched: "<< err.what();
    }
    std::cout << "--------------END--------------" << std::endl;
    return 0;
}
