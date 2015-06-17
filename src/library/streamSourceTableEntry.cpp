#include "streamSourceTableEntry.hpp"

using namespace pylongps; //Use pylongps classes without alteration for now

/**
This function reads in a serialized stream source entry from the given stream and uses it to set the values of the object.
@param inputSerializedStreamSourceEntry: The string to deserialize from
@param inputExpectTerminationCharacters: True if an entry should have a trailing \r\n and false if the whole string should be used without expecting end characters
@return: How many characters were read from the string (negative if the string was invalid)
*/
int streamSourceTableEntry::parse(const std::string &inputSerializedStreamSourceEntry, bool inputExpectTerminationCharacters)
{
int buffer;
//Tokenize
std::vector<std::string> tokens;
size_t last = 0;
size_t next = 0;
while ((next = inputSerializedStreamSourceEntry.find(";", last)) != std::string::npos && tokens.size() < 18) 
{
tokens.push_back(inputSerializedStreamSourceEntry.substr(last, next-last)); 
last = next + 1; 
} 

next = inputSerializedStreamSourceEntry.find("\r\n", last);
if(next == std::string::npos && inputExpectTerminationCharacters)
{
return -1; //Couldn't find end of entry
}
if(inputExpectTerminationCharacters)
{
tokens.push_back(inputSerializedStreamSourceEntry.substr(last, next-last));
}
else
{ //Grab till the end
tokens.push_back(inputSerializedStreamSourceEntry.substr(last));
next = last + tokens.back().size() - 2;
}

if(tokens.size() != 19)
{
return -1; //Less than the expected number of tokens
}

if(tokens[0] != std::string("STR"))
{
return -1; //Not a stream entry
}

type = tokens[0];
mountpoint = tokens[1];
if(mountpoint.size() > 100)
{
return -1; //Too long
}

identifier = tokens[2];
format = tokens[3];
formatDetails = tokens[4];

try
{
buffer = std::stoi(tokens[5]);
}
catch(const std::exception &inputException)
{
return -1; //Couldn't read carrier
}
if(buffer > 2 || buffer < 0)
{
return -1; //Not in the proper number range
}
carrier = (carrierOptionsType) buffer;

navigationSystems = tokens[6];
network = tokens[7];
countryCode = tokens[8];
if(countryCode.size() != 3)
{
return -1;
}

try
{
latitude =  std::stod(tokens[9]);
}
catch(const std::exception &inputException)
{
return -1; //Couldn't read latitude
}

try
{
longitude =  std::stod(tokens[10]);
}
catch(const std::exception &inputException)
{
return -1; //Couldn't read longitude
}

if(tokens[11] == std::string("1"))
{
NMEARequired = true;
}
else if(tokens[11] == std::string("0"))
{
NMEARequired = false;
}
else
{
return -1; //Couldn't read NMEA setting
}

if(tokens[12] == std::string("1"))
{
solution = NETWORK;
}
else if(tokens[12] == std::string("0"))
{
solution = SINGLE_BASESTATION;
}
else
{
return -1; //Couldn't read solution setting
}

generator = tokens[13];
compressionMethod = tokens[14];

if(tokens[15] == std::string("N"))
{
authentication = NONE;
}
else if(tokens[15] == std::string("B"))
{
authentication = BASIC;
}
else if(tokens[15] == std::string("D"))
{
authentication = DIGEST;
}
else
{
return -1; //Couldn't read authentication type
}

if(tokens[16] == "N")
{
fee = FREE;
}
else if(tokens[16] == "Y")
{
fee = FEE_REQUIRED;
}
else
{
return -1; //Couldn't read fee structure
}

try
{
buffer = std::stoi(tokens[17]);
}
catch(const std::exception &inputException)
{
return -1; //Couldn't read carrier
}
if(buffer < 0)
{
return -1; //Not in the proper number range
}
bitRate = buffer;
misc = tokens[18];

return next+2;
}

/**
This function returns a string representing the serialized version of this table entry.
@return: The serialized table entry
*/
std::string streamSourceTableEntry::serialize()
{
std::string stringToReturn;

stringToReturn += type + ";";
stringToReturn += mountpoint + ";";
stringToReturn += identifier + ";";
stringToReturn += format + ";";
stringToReturn += formatDetails + ";";
stringToReturn += std::to_string((int) carrier)+";";
stringToReturn += navigationSystems + ";";
stringToReturn += network + ";";
stringToReturn += countryCode + ";";
stringToReturn += std::to_string(latitude) + ";";
stringToReturn += std::to_string(longitude) + ";";
stringToReturn += std::to_string((int) NMEARequired) + ";"; 
stringToReturn += std::to_string((int) solution)+";";
stringToReturn += generator + ";";
stringToReturn += compressionMethod + ";";
stringToReturn.push_back((char) authentication);
stringToReturn += ";";
stringToReturn.push_back((char) fee);
stringToReturn += ";";
stringToReturn += std::to_string(bitRate) + ";";
stringToReturn += misc + "\r\n";

return stringToReturn;
}
