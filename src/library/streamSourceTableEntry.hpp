#ifndef  STREAMSOURCETABLEENTRYHPP
#define STREAMSOURCETABLEENTRYHPP

#include<string>
#include<vector>
#include "commonEnums.hpp"
#include<cstdio> //For debugging

namespace pylongps
{

/**
\ingroup Serialization
This class is meant to allow easy serialization of source table entries to and from ASCII characters.
*/
class streamSourceTableEntry
{
public:
/**
This function reads in a serialized stream source entry from the given stream and uses it to set the values of the object.
@param inputSerializedStreamSourceEntry: The string to deserialize from
@return: How many characters were read from the string (negative if the string was invalid)
*/
int parse(const std::string &inputSerializedStreamSourceEntry);

/**
This function returns a string representing the serialized version of this table entry.
@return: The serialized table entry
*/
std::string serialize();

std::string type;
std::string mountpoint;
std::string identifier;
std::string format;
std::string formatDetails;
carrierOptionsType carrier;
std::string navigationSystems;
std::string network;
std::string countryCode;
double latitude;
double longitude;
bool NMEARequired; //True of client must seem NMEA format position
solutionOptionsType solution;
std::string generator;
std::string compressionMethod;
authenticationMethodType authentication;
feeType fee;
int bitRate;
std::string misc;
};







}
#endif
