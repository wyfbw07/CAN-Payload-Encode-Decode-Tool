# CAN_DBC_DECODE_TOOL

## Build on macOS

Open the .xcodeproj project file, hit Build (Cmd+B).

Input arguments must be given upon run time:

	- argv[1]: DBC file address
	- argv[2]: Message ID in DEC
	- argv[3]: Payload in HEX
	- argv[4]: Signal Name

## Sample Usage

Not all input arguments must be provide to run. There are three use cases available.

- **Extract DBC File Info Only:**

		- argv[1]: /Users/filelocation/XVehicle.dbc
		
	By only providing the DBC file address, the program would parse and extract message and signals info in the file.
	
- **Decode an Entire Message:**

		- argv[1]: /Users/filelocation/XVehicle.dbc
		- argv[2]: 336
		- argv[3]: D0,87,F0
		
	Additionally a message ID in decimal and its payload can be provided.

- **Decode a Specific Signal Under a Message:**

		- argv[1]: /Users/filelocation/XVehicle.dbc
		- argv[2]: 336
		- argv[3]: D0,87,F0
		- argv[4]: EngineTemp

	Lastly a signal name can be provided to filter out the decoded value of that signal:

## Function Calls

### Load and Parse DBC File

	bool parse(const std::string& filePath);

- Use: To load and parse a DBC file, given the file path in string
- Returns: A bool to indicate whether parsing succeeds (true) or not (false)

Sample usage of this function:

	DbcParser dbcFile;
	dbcFile.parse(argv[1]);

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

	std::unordered_map<std::string, double> decode(uint32_t msgId, std::string payload);
	
- Use: To decode an entire message
- Returns: std::unordered_map<Signal name, decoded value>

The function will return an unordered map: <Signal name, decoded value>, where first is signal name, and second is the decoded value for each signal. The function now checks input payload length, returns decode result, and no longer prints decoded information by default.

### Decode a Specific Signal Under a Message

	double decodeSignalOnRequest(uint32_t msgId, std::string payload, std::string msgName);

- Use: To decode a specific signal under a message
- Returns: double decodedValue

To decode an specific signal under one message, call this function. The function will return the decoded value for that signal. The function now checks input payload length, returns decode result, and no longer prints decoded information by default.

## Resources

The following is the reference PDF for CAN bus  [DBC File Format Documentation](http://mcu.so/Microcontroller/Automotive/dbc-file-format-documentation_compress.pdf) Version 1.0.5 by Vector Informatik GmbH

## What's New

- The program can parse DBC files of two different message definitions. However for now I don't know the underlying reason why Vector changed it for the new version. There's no documentation to refer to now.
- Fix a bug that, for a given message ID to decode, the program doesn't check if it is a valid message present in the DBC file. This type of checking also applies to signals.