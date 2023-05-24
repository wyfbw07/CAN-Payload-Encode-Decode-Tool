# CAN_DBC_DECODE_TOOL

## Build on macOS

Open the .xcodeproj project file, hit Build (Cmd+B).

No arguments is required to run. Edit properties in main to use this tool.

## Sample Usage

Not all input arguments must be provide to run. There are three use cases available.

- **Extract DBC File Info Only:**

	```c++
	DbcParser dbcFile;
	dbcFile.parse("/Users/filelocation/XVehicle.dbc");
	```
	
	By only providing the DBC file address, the program would parse and extract message and signals info in the file.
	
- **Decode an Entire Message:**

  ```c++
  int dlc = 6;
  int msgId = 168;
  unsigned char rawPayload[8] = {0xe8, 0x0, 0x0, 0x0, 0x0, 0x0};
  dbcFile.decode(msgId, rawPayload, dlc);
  ```

  Additionally a message ID in decimal and its payload can be provided. You cannot decode a messsage before parsing the DBC file.

- **Decode a Specific Signal Under a Message:**

	```c++
	int dlc = 6;
	int msgId = 168;
	unsigned char rawPayload[8] = {0xe8, 0x0, 0x0, 0x0, 0x0, 0x0};
	std::string sigName = "EngineTemp";
	dbcFile.decodeSignalOnRequest(msgId, rawPayload, dlc, sigName);
	```
	
	Lastly a signal name can be provided to filter out the decoded value of that signal. You cannot decode a messsage before parsing the DBC file.
	
	

## Function Calls

### Load and Parse DBC File

	bool parse(const std::string& filePath);

- Use: To load and parse a DBC file, given the file path in string
- Returns: A bool to indicate whether parsing succeeds (true) or not (false)

Sample usage of this function:

	DbcParser dbcFile;
	dbcFile.parse("/Users/filelocation/XVehicle.dbc");

A instance of the class DbcParser must be created first, and use the parse function to load and parse the DBC file. All messages and signals info will then be stored.

For message classes, these information will be stored: 
- Message name
- Message ID
- Message size (payload length)
- Transmitter name
- A std::vector of Signal classes

For signal classes, these information will be stored: 
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



### Print DBC File Info

	void printDbcInfo();

- Use: To display all message and signal info in the DBC File 
- Returns: std::cout in terminal

Sample usage of this function:

	dbcFile.printDbcInfo();

Use this function to display DBC file info once a DBC file is loaded and parsed. Messages and signals info would appear in terminal or Xcode debugger terminal.



### Decode an Entire Message

```c++
std::unordered_map<std::string, double> decode(unsigned char rawPayload[], unsigned short dlc);
```

- Use: To decode an entire message
- Returns: std::unordered_map<Signal name, decoded value>

Sample usage of this function:

```c++
unsigned char rawPayload[8] = {0xe8, 0x0, 0x0, 0x0, 0x0, 0x0};
dbcFile.decode(168, rawPayload, 6);
```

The function will return an unordered map: <Signal name, decoded value>, where first is signal name, and second is the decoded value for each signal. The function checks input payload length, returns decode result, and no longer prints decoded information by default.



### Decode a Specific Signal Under a Message

```c++
double DbcParser::decodeSignalOnRequest(uint32_t msgId, unsigned char payLoad[], unsigned short dlc, std::string sigName);
```

- Use: To decode a specific signal under a message
- Returns: A decoded value of type double

Sample usage of this function:

```c++
unsigned char rawPayload[8] = {0xe8, 0x0, 0x0, 0x0, 0x0, 0x0};
dbcFile.decodeSignalOnRequest(168, payLoad, 6, "EngineTemp");
```

To decode an specific signal under one message, call this function. The function will return the decoded value for that signal. The function now checks input payload length, returns decode result, and no longer prints decoded information by default.



## Resources

Here is the reference PDF for CAN bus [DBC File Format Documentation](http://mcu.so/Microcontroller/Automotive/dbc-file-format-documentation_compress.pdf) Version 1.0.5 by Vector Informatik GmbH



## What's New

- Input arguments are no longer required upon running
- The type of the input payload in decode functions are now changed to an fixed sized array of unsigned char, with an addition of the payload length called dlc.

