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
    /*
     
     argv[1]:  DBC file address
     optional: argv[2]: Message ID in DEC
               argv[3]: Payload in HEX
               argv[4]: Signal Name
     
     Sample usage 1:
     argv[1]: /Users/filelocation/XVehicle.dbc
     
     Sample usage 2:
     argv[1]: /Users/filelocation/XVehicle.dbc
     argv[2]: 336
     argv[3]: D0,87,F0
     
     Sample usage 3:
     argv[1]: /Users/filelocation/XVehicle.dbc
     argv[2]: 336
     argv[3]: D0,87,F0
     argv[4]: EngineTemp
     
     */
    if (argc < 2){
        std::cout << "Too few args" << std::endl;
    }else{
        try {
            // Create a class to store DBC info
            DbcParser dbcFile;
            // Load file from path. Parse and store the content
            dbcFile.parse(argv[1]);
            // Display DBC info
            dbcFile.printDbcInfo();
            switch (argc) {
                case 4:
                {
                    std::istringstream iss2(argv[2]);
                    std::istringstream iss3(argv[3]);
                    int msgId;
                    std::string payload;
                    std::string payloadInHex;
                    iss2 >> msgId; iss3 >> payload;
                    dbcFile.decode(msgId, payload);
                    
                }
                    break;
                case 5:
                {
                    std::istringstream iss2(argv[2]);
                    std::istringstream iss3(argv[3]);
                    std::istringstream iss4(argv[4]);
                    int msgId;
                    std::string payload;
                    std::string sigName;
                    iss2 >> msgId; iss3 >> payload; iss4 >> sigName;
                    dbcFile.decodeSignalOnRequest(msgId, payload, sigName);
                }
                    break;
                default:
                    break;
            }
        }
        catch (std::invalid_argument& err) {
            std::cout << "Exception catched: "<< err.what();
            return 1;
        }
    }
    std::cout << "--------------END--------------" << std::endl;
    return 0;
}
