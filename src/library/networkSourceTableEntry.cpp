#include "networkSourceTableEntry.hpp"

/*
This function reads in a serialized network entry entry from the given stream and uses it to set the values of the object.
@param inputSerializedNetworkSourceEntry: The string to deserialize from
@return: How many characters were read from the string (negative if the string was invalid)
*/
int networkSourceTableEntry::parse(std::string inputSerializedNetworkSourceEntry)
{
//Tokenize
std::vector<std::string> tokens;
size_t last = 0;
size_t next = 0;
while ((next = inputSerializedNetworkSourceEntry.find(";", last)) != std::string::npos && tokens.size() < 8) 
{
tokens.push_back(inputSerializedNetworkSourceEntry.substr(last, next-last)); 
last = next + 1; 
} 

next = inputSerializedNetworkSourceEntry.find("\r\n", last);
if(next == std::string::npos)
{
return -1; //Couldn't find end of entry
}
tokens.push_back(inputSerializedNetworkSourceEntry.substr(last, next-last));


if(tokens.size() != 9)
{
return -1; //Less than the expected number of tokens
}

if(tokens[0] != std::string("NET"))
{
return -1; //Not a network entry
}

type = tokens[0];
identifier = tokens[1];
operatorID = tokens[2];

if(tokens[3] == std::string("N"))
{
authenticationMethod = NONE;
}
else if(tokens[3] == std::string("B"))
{
authenticationMethod = BASIC;
}
else if(tokens[3] == std::string("D"))
{
authenticationMethod = DIGEST;
}
else
{
return -1; //Couldn't read authentication type
}

if(tokens[4] == "N")
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

networkInformationWebAddress = tokens[5];
streamInformationWebAddress = tokens[6];
webOrMailAddressForRegistration = tokens[7];
misc = tokens[8];

return next+2;
}

/*
This function returns a string representing the serialized version of this table entry.
@return: The serialized table entry
*/
std::string networkSourceTableEntry::serialize()
{
std::string stringToReturn;

stringToReturn += type + ";";
stringToReturn += identifier + ";";
stringToReturn += operatorID + ";";
stringToReturn.push_back((char) authenticationMethod);
stringToReturn += ";";
stringToReturn.push_back((char) fee);
stringToReturn += ";";
stringToReturn += networkInformationWebAddress + ";";
stringToReturn += streamInformationWebAddress + ";";
stringToReturn += webOrMailAddressForRegistration + ";";
stringToReturn += misc + "\r\n";

return stringToReturn;
}
