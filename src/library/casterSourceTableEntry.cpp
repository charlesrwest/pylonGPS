#include "casterSourceTableEntry.hpp"

/*
This function reads in a serialized caster entry entry from the given stream and uses it to set the values of the object.
@param inputSerializedCasterSourceEntry: The string to deserialize from
@return: How many characters were read from the string (negative if the string was invalid)
*/
int casterSourceTableEntry::parse(std::string inputSerializedCasterSourceEntry)
{
int buffer;
//Tokenize
std::vector<std::string> tokens;
size_t last = 0;
size_t next = 0;
while ((next = inputSerializedCasterSourceEntry.find(";", last)) != std::string::npos && tokens.size() < 9) 
{
tokens.push_back(inputSerializedCasterSourceEntry.substr(last, next-last)); 
last = next + 1; 
} 

next = inputSerializedCasterSourceEntry.find("\r\n", last);
if(next == std::string::npos)
{
return -1; //Couldn't find end of entry
}
tokens.push_back(inputSerializedCasterSourceEntry.substr(last, next-last));

if(tokens.size() != 10)
{
return -1; //Less than the expected number of tokens
}

if(tokens[0] != std::string("CAS"))
{
return -1; //Not a stream entry
}

type = tokens[0];
host = tokens[1];
if(host.size() > 128)
{
return -1;
}

try
{
buffer = std::stoi(tokens[2]);
}
catch(const std::exception &inputException)
{
return -1; //Couldn't read carrier
}
if(buffer < 0)
{
return -1; //Not in the proper number range
}
port = buffer;
identifier = tokens[3];
operatorID = tokens[4];

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
nmeaClientsSupported = (bool) buffer;
country = tokens[6];
if(country.size() != 3)
{
return -1;
}

try
{
latitude =  std::stod(tokens[7]);
}
catch(const std::exception &inputException)
{
return -1; //Couldn't read latitude
}

try
{
longitude =  std::stod(tokens[8]);
}
catch(const std::exception &inputException)
{
return -1; //Couldn't read longitude
}

misc = tokens[9];

return next+2;
}

/*
This function returns a string representing the serialized version of this table entry.
@return: The serialized table entry
*/
std::string casterSourceTableEntry::serialize()
{
std::string stringToReturn;

stringToReturn += type + ";";
stringToReturn += host + ";";
stringToReturn += std::to_string(port) + ";";
stringToReturn += identifier + ";";
stringToReturn += operatorID + ";";
stringToReturn += std::to_string( (int) nmeaClientsSupported) + ";";
stringToReturn += country + ";";
stringToReturn += std::to_string(latitude) + ";";
stringToReturn += std::to_string(longitude) + ";";
stringToReturn += misc + "\r\n";

return stringToReturn;
}
