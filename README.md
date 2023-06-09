# CAN Database Encode Decode Tool



## Build and Run

### On macOS

Open the .xcodeproj project file, hit Build (Cmd+B).

No arguments is required to run. Edit properties in main to use this tool.

### On Other Operating Systems

You will need to use the these source files and create a new project on your own to build. This tool has been tested on Visual Studio 17.6.0 and Qt Creator 6.4.3 on Windows 11 64bit.



## Error Handling

This tool uses exceptions to eraise errors. As a result, running and calling functions should be wrapped into a try-catch block. If nothing is added to catch potential errors, it could cause program termination.



## Function Calls

### Load and Parse DBC File

```c++
bool DbcParser::parse(const std::string& filePath);
```

| About this function | Description                                                  |
| :------------------ | :----------------------------------------------------------- |
| Use case            | To load and parse a DBC file, given the file path in string  |
| Return value        | A bool to indicate whether parsing succeeds (true) or not (false) |

Sample usage of this function:

```c++
DbcParser dbcFile;
dbcFile.parse("/Users/filelocation/XVehicle.dbc");
```

A instance of the class DbcParser must be created first, and use the parse function to load and parse the DBC file. All messages and signals info will then be stored. You cannot decode or encode a messsage before parsing the DBC file.

Again, <u>this function can be called ONLY once.</u> It does not allow repeated calls of this function. There is little use cases that you would want to re-parse the file. However if you do want to re-parse, destroy the existing class instance and create a new one to parse again.

For message classes, these information will be parsed: 
- Message name
- Message ID
- Message size (payload length)
- Transmitter name
- A std::vector of Signal classes

For signal classes, these information will be parsed: 
- Signal name
- Signal unit
- Signal start bit
- Signal size (data length)
- Factor
- Offset
- Maximum value
- Minimum value
- Byte order (* 0=big endian, 1=little endian *)
- Value type (* +=unsigned, -=signed *)
- Receivers (node name)
- Signal value descriptions



### Print DBC File Info

```c++
void DbcParser::printDbcInfo();
```

| About this function | Description                                                  |
| :------------------ | :----------------------------------------------------------- |
| Use case            | To display all message and signal info in the CAN database File |
| Return value        | std::cout in terminal                                        |

Sample usage of this function:

```c++
dbcFile.printDbcInfo();
```

Use this function to display DBC file info once a DBC file is loaded and parsed. Messages and signals info would appear in terminal or Xcode debugger terminal.



### Decode an Entire Message

```c++
std::unordered_map<std::string, double> DbcParser::decode(unsigned long msgId, unsigned char payLoad[], unsigned int dlc);
```

| About this function | Description                                                  |
| :------------------ | :----------------------------------------------------------- |
| Use case            | To decode a message payload                                  |
| Input parameters    | (Message ID in decimal, An array of message payload, Message size) |
| Return value        | std::unordered_map<Signal name, decoded value>               |

Sample usage of this function:

```c++
// Prepare data
unsigned char rawPayload[8] = {0xe8, 0x0, 0x0, 0x0, 0x0, 0x0};
// Decode
dbcFile.decode(168, rawPayload, 6);
```

The function will return an unordered map: <Signal name, decoded value>, where first is signal name, and second is the decoded value for each signal. The function checks input payload length, returns decode result, and no longer prints decoded information by default.



### Decode and Show Specific Signal Value Under a Message

```c++
double DbcParser::decodeSignalOnRequest(unsigned long msgId, unsigned char payLoad[], unsigned int dlc, std::string sigName);
```

| About this function | Description                                                  |
| :------------------ | :----------------------------------------------------------- |
| Use case            | To decode a specific signal under a message payload          |
| Input parameters    | (Message ID in decimal, An array of message payload, Message size, Signal name) |
| Return value        | A decoded value of type double                               |

Sample usage of this function:

```c++
// Prepare data
unsigned char rawPayload[8] = {0xe8, 0x0, 0x0, 0x0, 0x0, 0x0};
// Decode
dbcFile.decodeSignalOnRequest(168, payLoad, 6, "EngineTemp");
```

Call this function to decode an specific signal under one message. The function will return the decoded value for that signal. The function now checks input payload length, returns decode result, and no longer prints decoded information by default.



### Encode a Message

```c++
unsigned int DbcParser::encode(unsigned long msgId,
                               std::vector<std::pair<std::string, double> > signalsToEncode,
                               unsigned char encodedPayload[MAX_MSG_LEN])
```

| About this function | Description                                                  |
| :------------------ | :----------------------------------------------------------- |
| Use case            | To encode a CAN message payload                              |
| Input parameters    | (Message ID in decimal, An vector of pair of signal name and physical value, A fixed size 8 slots array that contains the encoded payload) |
| Return value        | Message size of the encoded payload                          |

Sample usage of this function:

```c++
// Prepare data
unsigned int encodedDlc = 0;
unsigned char encodedPayload[8];
std::vector<std::pair<std::string, double> > signalsToEncode;
signalsToEncode.push_back(std::make_pair("EngSpeed", 50));
signalsToEncode.push_back(std::make_pair("EngSpeed_Second", 1535));
// Encode
encodedDlc = dbcFile.encode(168, signalsToEncode, encodedPayload);
```

The function encodes one or more signals at once into a single message payload. The fixed size encodedPayload array contains the generated payload once the function has been called. An additional value is returned to specify the message size of the encoded payload.



## Resources

This tool is developed with XL-Driver-Library in mind. The free-of-charge XL-Driver-Library is a universal programming interface you can use to create your own applications while accessing Vector’s powerful hardware interfaces. 

Reference PDF for CAN bus [DBC File Format Documentation](http://mcu.so/Microcontroller/Automotive/dbc-file-format-documentation_compress.pdf) Version 1.0.5 by Vector Informatik GmbH.

[User Manual](https://cdn.vector.com/cms/content/products/XL_Driver_Library/Docs/XL_Driver_Library_Manual_EN.pdf) of [XL-Driver-Library](https://www.vector.com/int/en/products/products-a-z/libraries-drivers/xl-driver-library/#) by Vector Informatik GmbH.



## What's New

- This tool now supports encoding CAN bus messages.  One or more signals can be encoded at once into a single message payload. 

