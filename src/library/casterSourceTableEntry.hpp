#ifndef  CASTERSOURCETABLEENTRYHPP
#define CASTERSOURCETABLEENTRYHPP

#include<string>
#include<vector>
#include<cstdio> //For debugging

namespace pylongps
{

/**
\ingroup Serialization
This class is meant to allow easy serialization of source table entries to and from ASCII characters.
*/
class casterSourceTableEntry
{
public:
/**
This function reads in a serialized caster entry entry from the given stream and uses it to set the values of the object.
@param inputSerializedCasterSourceEntry: The string to deserialize from
@return: How many characters were read from the string (negative if the string was invalid)
*/
int parse(std::string inputSerializedCasterSourceEntry);

/**
This function returns a string representing the serialized version of this table entry.
@return: The serialized table entry
*/
std::string serialize();

std::string type;
std::string host;
int port;
std::string identifier;
std::string operatorID;
bool nmeaClientsSupported;
std::string country;
double latitude;
double longitude;
std::string misc;
};











}
#endif 
