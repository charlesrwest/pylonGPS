#ifndef  NMEAGGASENTENCEHPP
#define NMEAGGASENTENCEHPP

#include<string>
#include<vector>
#include<cstdint>
#include<cmath>
#include<tuple>
#include<cstdio> 

namespace pylongps
{

enum GPSQualityType
{
INVALID = 0,
GPS_FIX = 1,
DIFFERENTIAL_GPS_FIX = 2
};

/**
\ingroup Serialization
This class represents a NMEA GGA sentence, which records information from a single GPS reading.  It supports serialization to and from a string and can be used as part of a NTRIP request.
*/
class NMEAGGASentence
{
public:
/**
This function initializes the object with sensible defaults where possible and expects the rest to be set after the fact.
*/
NMEAGGASentence();

/**
This function parses the given string and sets the values of this object to reflect those in the string.
@param inputNMEAStringToParse: The string to read values from
@return: false if the parse failed, possibly leaving the object with some values from the parse
*/
bool parse(const std::string &inputNMEAStringToParse);

/**
This function serializes this object the returned NMEA string.
@return: The nmea GGA sentence string representing this object
*/
std::string serialize();



std::string talkerID; //Device class of the sender (defaults to GP for GPS)
uint32_t hours; //Hours part of reading UTC time
uint32_t minutes; //Minutes part of reading UTC time 
double seconds; //Seconds part of reading UTC time
double latitude; //In degrees (-90 to 90), South is negative
double longitude; //In degrees (-180 to 180), West is negative
GPSQualityType qualityType;
uint32_t numberOfSatellitesInUse;
double horizontalDilutionOfPrecision;
double antennaAltitude; //Altitude in Meters
double geoidalSeparation; //Separation in meters between WSG-84 earth ellipsoid and mean sea level (negative if geoid is below)
double differentialUpdateAge; //Time in seconds since the last update from differential basestation (negative if not applicable)
std::string differentialBasestationID; //ID of the basestation that is sending updates (if any)
};






}
#endif 
