#include "sourceRegistrationRequestHeader.hpp"

using namespace pylongps;

/**
This function reads in a serialized stream source  from the given stream and uses it to set the values of the object.
@param inputSerializedSourceRegistrationRequestHeader: The string to deserialize from
@return: How many characters were read from the string (negative if the string was invalid)
*/
int sourceRegistrationRequestHeader::parse(const std::string &inputSerializedSourceRegistrationRequestHeader)
{
int buffer;
//Tokenize
std::vector<std::string> tokens;
size_t last = 0;
size_t next = 0;
while ((next = inputSerializedSourceRegistrationRequestHeader.find("\r\n", last)) != std::string::npos) 
{
tokens.push_back(inputSerializedSourceRegistrationRequestHeader.substr(last, next-last));
if(tokens.back().size() == 0) //Reached end of header
{
break;
} 
last = next + 2; 
} 

int numberOfCharactersFromHeader = next+2;

if(tokens.size() < 3) //Need at least 3 lines (1 empty)
{
return -1;
}


if(tokens.back().size() != 0)
{
return -1; //Expect header to end with "\r\n\r\n"
}


//Check that the first token starts with "SOURCE" 
if(tokens[0].find("SOURCE") != 0)
{
return -1; //Didn't start with "SOURCE"
}


//Tokenize first line based on spaces
std::vector<std::string> firstLineTokens;

last = 0;
next = 0;
while ((next = tokens[0].find(" ", last)) != std::string::npos) 
{
firstLineTokens.push_back(tokens[0].substr(last, next-last)); 
last = next + 1; 
} 
if(tokens[0].size() == last)
{
return -1; //no mountpoint field
}

//Add mountpoint
firstLineTokens.push_back(tokens[0].substr(last)); 

if(firstLineTokens.size() < 3)
{
return -1; //Not enough fields in the first line
}

password = firstLineTokens[1];
mountpoint = firstLineTokens[2];

std::string headerString = "Source-Agent: NTRIP ";
if(tokens[1].find(headerString) != 0)
{
return -1; //Second line didn't have required start string
}

productComment = tokens[1].substr(headerString.size(), tokens[1].size() - headerString.size());

//tokenize any remaining fields
for(int i=2; i<tokens.size(); i++)
{
auto splitPoint = tokens[i].find(": ");
if(splitPoint == std::string::npos)
{
continue; //Skip any lines that aren't name:value pairs
}

std::pair<std::string, std::string>  nameValuePair;
nameValuePair.first = tokens[i].substr(0, splitPoint);
if(tokens[i].size() > splitPoint+2)
{
nameValuePair.second = tokens[i].substr(splitPoint+2);
}
else
{
nameValuePair.second = "";
}

extraFields.push_back(nameValuePair);
}

return numberOfCharactersFromHeader;
}

/**
This function returns a string representing the serialized version of this source registration request header.
@return: The serialized source registration request header
*/
std::string sourceRegistrationRequestHeader::serialize()
{
std::string stringToReturn;

stringToReturn += "SOURCE " + password + " "+mountpoint + "\r\n";
stringToReturn += "Source-Agent: NTRIP " + productComment  +"\r\n";

for(int i=0; i<extraFields.size(); i++)
{
stringToReturn += extraFields[i].first + ": " + extraFields[i].second + "\r\n";
}

stringToReturn += "\r\n";

return stringToReturn;
} 
