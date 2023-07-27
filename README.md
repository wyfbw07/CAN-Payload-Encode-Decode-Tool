# Payload Encoder and Decoder based on CAN Database (DBC) File

A C/C++ program that can parse DBC files, and decode or encode message payloads.

The parser supports CAN and CAN FD.

The parser does not currently support parsing multiplexer and multiplexed signals.



## Build and Run

### On macOS

Open the .xcodeproj project file, hit Build (Cmd+B).

No arguments is required to run. Edit properties in main to try this tool.



### On Other Operating Systems

You will need to use the source files and create a new project on your own to build. This tool has been tested on Visual Studio 17.6.0 and Qt Creator 6.4.3 on Windows 11 64bit.



## Error Handling

This tool uses exceptions to eraise errors. As a result, running and calling functions should be wrapped into a try-catch block. If nothing is added to catch potential errors, it could cause program termination.



## TL;DR

### Parse and Decode

```c++
// Create a class instance to store DBC info
DbcParser dbcFile
try {
        // Load file from path. Parse and store the content
        dbcFile.parse("/Users/FilePath/exampleCAN_Medium.dbc");
        
        // Optional: Display DBC info
        std::cout << dbcFile;
        
        // Prepare data
        unsigned long msgId = 258;
        const unsigned int msgSize = 8;
        unsigned char rawPayload[] = {0x2d, 0xff, 0xc8, 0x0f, 0x0, 0x0, 0x0, 0x0};
        
        // Decode message payload
        std::unordered_map<std::string, double> result;
        result = dbcFile.decode(msgId, msgSize, rawPayload);
        // >>> {"Signal_1": 1234.5, "Signal_2": 0, "Signal_3": 12}
}
catch (std::invalid_argument& err) {
		std::cout << "[Exception catched] " << err.what() << '\n';
}
```

### Parse and Encode

```c++
// Create a class instance to store DBC info
DbcParser dbcFile;
try {
        // Load file from path. Parse and store the content
        dbcFile.parse("/Users/FilePath/exampleCAN_Medium.dbc");
        
        // Optional: Display DBC info
        std::cout << dbcFile;
        
        // Prepare data
        unsigned long msgId = 258;
        std::vector<std::pair<std::string, double> > signalsToEncode;
        signalsToEncode.push_back(std::make_pair("EngSpeed", 4040));
        signalsToEncode.push_back(std::make_pair("PetrolLevel", 255));
        signalsToEncode.push_back(std::make_pair("EngTemp", 90));
        
        // Encode message payload
        const unsigned int MAX_MSG_LEN = 32;
        unsigned char encodedPayload[MAX_MSG_LEN];
        unsigned int encodedMsgSize = dbcFile.encode(msgId, signalsToEncode, encodedPayload, MAX_MSG_LEN);
        // >>> encodedMsgSize: 4
        // >>> rawPayload: {0x2d, 0xff, 0xc8, 0x0f, 0x0, 0x0, 0x0, 0x0};
}
catch (std::invalid_argument& err) {
        std::cout << "[Exception catched] " << err.what() << '\n';
}
```



## Function Calls

### Load and Parse DBC File

```c++
bool DbcParser::parse(const std::string& filePath);
```

#### Use Case

To load and parse a DBC file, given the file path in string.

#### Input Parameters

**filePath**

The file path of the dbc file.

#### Return value

Returns a bool to indicate whether parsing succeeds (true) or not (false).

#### Sample usage of this function

```c++
DbcParser dbcFile;
dbcFile.parse("/Users/FilePath/exampleCAN_Medium.dbc");
```

#### Description

A instance of the class DbcParser must be created first, and use the parse function to load and parse the DBC file. All messages and signals info will then be stored. You can NOT decode or encode a messsage before parsing the DBC file.

Repeated calls of this function are not allowed and will be ignored. There is little use cases that you would want to re-parse the file. If you do want to re-parse, destroy the existing class instance and create a new one to parse again.

The parser supports CAN and CAN FD.

The parser does not currently support parsing multiplexer and multiplexed signals.

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
- Initial value
- Maximum value
- Minimum value
- Byte order (Intel, Motorola)
- Value type (Signed, Unsigned, IEEE Float, IEEE Double)
- Signal type (Normal, Multiplexed)
- Receivers (node name)
- Signal value descriptions

For attributes, these will be parsed: 

- BA_DEF_ SG_ "GenSigStartValue"
- BA_DEF_DEF_ "GenSigStartValue"
- BA_ "BusType"
- BA_ "GenSigStartValue" SG_



### Print DBC File Info

```c++
friend std::ostream& operator<<(std::ostream& os, const DbcParser& dbcFile);
```

#### Use Case

To display all message and signal info in the CAN database File.

#### Input Parameters

**dbcFile**

A DbcParser class instance that has already parsed an DBC file.

#### Return value

Output stream

#### Sample usage of this function

```c++
std::cout << dbcFile;
```

#### Description

Display DBC file info once a DBC file is loaded and parsed. Messages and signals info would appear in terminal.



### Decode a Message Payload

```c++
std::unordered_map<std::string, double> DbcParser::decode(
    unsigned long msgId,
    unsigned int msgSize,
    unsigned char payload[]
);
```

#### Use Case

To decode a message payload.

#### Input Parameters

**msgId**

The message's CAN-ID.

**msgSize**

Specifies the size of the message in bytes.

**payload**

The message payload that need to be decoded.

#### Return value

Returns an unordered map: <Signal name, decoded value>, where the first is signal name, and the second is the decoded value for each signal. 

#### Sample usage of this function

```c++
// Prepare data
unsigned char rawPayload[] = {0xe8, 0x0, 0x0, 0x0, 0x0, 0x0};
// Decode
std::unordered_map<std::string, double> result = dbcFile.decode(168, 4, rawPayload);
```



### Encode a Message Payload

```c++
unsigned int DbcParser::encode(
    unsigned long msgId,
    std::vector<std::pair<std::string, double> >& signalsToEncode,
    unsigned char encodedPayload[],
    unsigned int encodedPayloadSize
);
```

#### Use Case

To encode a CAN message payload

#### Input Parameters

**msgId**

The message's CAN-ID.

**signalsToEncode**

An vector of pairs that contains signal names and physical values. In each pair the first is signal name, and the second is the physical value for that signal. 

**encodedPayloadSize**

Specifies the max size of the encoded message payload. For classical CAN this will be 8. For CAN FD this will be 64. Provide any other size values would still work, but the encoded message payload could be truncated or prolonged.

#### Output Parameters

**encodedPayload**

Contains the encoded message payload.

#### Return value

Returns an unsigned int that specifies the encoded message size.

#### Sample usage of this function

```c++
// Prepare data
unsigned long msgId = 258;
std::vector<std::pair<std::string, double> > signalsToEncode;
signalsToEncode.push_back(std::make_pair("EngSpeed", 50));
// Encode
const unsigned int MAX_MSG_LEN = 2;
unsigned char encodedPayload[MAX_MSG_LEN];
unsigned int encodedMsgSize = dbcFile.encode(msgId, signalsToEncode, encodedPayload, MAX_MSG_LEN);
```

The function encodes one or more signals at once into a single message payload. The fixed size encodedPayload array contains the encoded payload once the function has been called. An additional value is returned to specify the message size of the encoded payload.



## Resources

This tool is developed with XL-Driver-Library in mind. The free-of-charge XL-Driver-Library is a universal programming interface you can use to create your own applications while accessing Vector’s powerful hardware interfaces. 

Reference PDF for CAN bus [DBC File Format Documentation](http://mcu.so/Microcontroller/Automotive/dbc-file-format-documentation_compress.pdf) Version 1.0.5 by Vector Informatik GmbH.

[User Manual](https://cdn.vector.com/cms/content/products/XL_Driver_Library/Docs/XL_Driver_Library_Manual_EN.pdf) of [XL-Driver-Library](https://www.vector.com/int/en/products/products-a-z/libraries-drivers/xl-driver-library/#) by Vector Informatik GmbH.



## What's New

- CAN FD support.

