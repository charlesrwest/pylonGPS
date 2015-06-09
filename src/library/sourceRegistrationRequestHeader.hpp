#ifndef  SOURCEREGISTRATIONREQUESTHEADERHPP
#define SOURCEREGISTRATIONREQUESTHEADERHPP

#include<string>
#include<vector>
#include<cstdio> //For debugging
#include<utility>

namespace pylongps
{

/**
\ingroup Serialization
This function is meant to allow easy serialization/deserialization of ntrip SOURCE registration headers (part of the "extension" of the HTTP protocol) to and from ASCII characters.
*/
class sourceRegistrationRequestHeader
{
public:
/**
This function reads in a serialized stream source  from the given stream and uses it to set the values of the object.
@param inputSerializedSourceRegistrationRequestHeader: The string to deserialize from
@return: How many characters were read from the string (negative if the string was invalid)
*/
int parse(const std::string &inputSerializedSourceRegistrationRequestHeader);

/**
This function returns a string representing the serialized version of this source registration request header.
@return: The serialized source registration request header
*/
std::string serialize();

std::string password;
std::string mountpoint;
std::string productComment; //Value of Source-Agent: NTRIP field value
std::vector<std::pair<std::string, std::string> > extraFields;
};




}

#endif 
