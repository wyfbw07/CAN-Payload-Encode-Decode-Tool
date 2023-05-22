# CAN_DBC_DECODE_TOOL

## Usage

argv[1]:  DBC file address

argv[2]: Message ID in DEC

argv[3]: Payload in HEX

argv[4]: Signal Name

## Sample Usage

### Sample usage 1:

argv[1]: /Users/filelocation/XVehicle.dbc

### Sample usage 2:

argv[1]: /Users/filelocation/XVehicle.dbc

argv[2]: 336

argv[3]: D0,87,F0

### Sample usage 3:

argv[1]: /Users/filelocation/XVehicle.dbc

argv[2]: 336

argv[3]: D0,87,F0

argv[4]: EngineTemp


## Notes

Decode now checks input payload length, returns decode result, and no longer prints decoded information by default.

To decode an entire message, call this function. The function will return an unordered map: <Signal name, decoded value>, where first is signal name, and second is the decoded value for each signal. 

	 std::unordered_map<std::string, double> decode(uint32_t msgId, std::string payload);

To decode an specific signal under one message, call this function. The function will return the decoded value for that signal.

	double decodeSignalOnRequest(uint32_t msgId, std::string payload, std::string msgName);
