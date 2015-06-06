#ifndef  NETWORKSOURCETABLEENTRYHPP
#define NETWORKSOURCETABLEENTRYHPP

#include<vector>
#include<string>
#include "commonEnums.hpp"
#include<cstdio>

/**
\ingroup Serialization
This class is meant to allow easy serialization of source table entries to and from ASCII characters.
*/
class networkSourceTableEntry
{
public:
/**
This function reads in a serialized network entry entry from the given stream and uses it to set the values of the object.
@param inputSerializedNetworkSourceEntry: The string to deserialize from
@return: How many characters were read from the string (negative if the string was invalid)
*/
int parse(std::string inputSerializedNetworkSourceEntry);

/**
This function returns a string representing the serialized version of this table entry.
@return: The serialized table entry
*/
std::string serialize();

std::string type;
std::string identifier;
std::string operatorID;
authenticationMethodType authenticationMethod;
feeType fee;
std::string networkInformationWebAddress; //web-net
std::string streamInformationWebAddress; //web-str
std::string webOrMailAddressForRegistration; //web-reg
std::string misc;
};








#endif
