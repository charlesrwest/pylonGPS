#include "NMEAGGASentence.hpp"

/*
This function initializes the object with sensible defaults where possible and expects the rest to be set after the fact.
*/
NMEAGGASentence::NMEAGGASentence() : talkerID("GP")
{
hours = 0;
minutes = 0;
seconds = 0.0;
latitude = 0.0;
longitude = 0.0;
qualityType = INVALID;
numberOfSatellitesInUse = 0;
horizontalDilutionOfPrecision = 0.0;
antennaAltitude = 0.0;
geoidalSeparation = 0.0;
differentialUpdateAge = -1.0;
}

/*
This function parses the given string and sets the values of this object to reflect those in the string.
@param inputNMEAStringToParse: The string to read values from
@return: false if the parse failed, possibly leaving the object with some values from the parse
*/
bool NMEAGGASentence::parse(const std::string &inputNMEAStringToParse)
{

//Check $ is present and start there
auto dollarSignIndex =  inputNMEAStringToParse.find("$");
if(dollarSignIndex == std::string::npos)
{
return false;
}


//Tokenize
std::vector<std::string> tokens;
size_t last = 1;
size_t next = 1;
while ((next = inputNMEAStringToParse.find(",", last)) != std::string::npos) 
{
tokens.push_back(inputNMEAStringToParse.substr(last, next-last)); 
last = next + 1; 
} 
tokens.push_back(inputNMEAStringToParse.substr(last));

if(tokens.size() != 15)
{
return false; //Less than the expected number of tokens
}

//Check this is actually a GGA sentence
if(tokens[0].size() != 5)
{
return false; //Doesn't have full sentence ID
}

if(tokens[0].substr(2).find("GGA") == std::string::npos && tokens[0].substr(2).find("gga") == std::string::npos)
{
return false;
}

//Get device class
talkerID = tokens[0].substr(0,2);


//Get current time
if(tokens[1].size() < 6)
{
return false;
}


try //Get hours
{
hours = stoi(tokens[1].substr(0, 2));
}
catch(const std::exception &inputException)
{
return false;
}
if(hours < 0 || hours > 23)
{
return false;
}

try //Get minutes
{
minutes = stoi(tokens[1].substr(2, 2));
}
catch(const std::exception &inputException)
{
return false;
}
if(minutes < 0 || minutes > 59)
{
return false;
}

try //Get seconds
{
seconds = stod(tokens[1].substr(4));
}
catch(const std::exception &inputException)
{
return false;
}
if(seconds < 0 || seconds >= 60.0)
{
return false;
}


//Convert all normal integer fields
std::vector<int> integerFields;
std::vector<int> integerIndices = {6, 7};

for(int i=0; i<integerIndices.size(); i++)
{
try
{
integerFields.push_back(stoi(tokens[integerIndices[i]]));
}
catch(const std::exception &inputException)
{
return false;
}
}

//Convert all normal double fields
std::vector<double> doubleFields;
std::vector<double> doubleIndices = {2,4,8,9,11,13};

for(int i=0; i<doubleIndices.size(); i++)
{
try
{
doubleFields.push_back(stod(tokens[doubleIndices[i]]));
}
catch(const std::exception &inputException)
{
if(!doubleIndices[i] == 13)
{
return false;
}
else
{
doubleFields.push_back(-1.0); //No differential update
}
}
}

//Verify integer values
if(integerFields[0] < 0 || integerFields[0] > 2) //GPS quality
{
return false;
}
qualityType = (GPSQualityType) integerFields[0];



if(integerFields[1] < 0 || integerFields[1] > 12) //Number of satellites visible
{
return false;
}
numberOfSatellitesInUse = integerFields[1];

//Verify double values
int numberOfDegrees = ((int) (doubleFields[0]/100.0)); //Latitude
latitude = numberOfDegrees + (doubleFields[0]-numberOfDegrees*100.0)/60.0;
if(doubleFields[0] < 0.0 || latitude > 90.0) 
{
return false;
}

numberOfDegrees = ((int) (doubleFields[1]/100.0)); //Longitude
longitude = numberOfDegrees + (doubleFields[1]-numberOfDegrees*100.0)/60.0;
if(doubleFields[1] < 0.0 || longitude > 180.0) 
{
return false;
}

if(doubleFields[2] < 0 || doubleFields[2] > 10.0) //dilution
{
return false;
}
horizontalDilutionOfPrecision = doubleFields[2];

//antenna altitude
antennaAltitude = doubleFields[3];

//Geoidal seperation
geoidalSeparation = doubleFields[4];

//Differential update age
differentialUpdateAge = doubleFields[5];



//Check N/S and change sign accordingly
if(tokens[3].find("S") != std::string::npos || tokens[3].find("s") != std::string::npos)
{
latitude = -latitude;
}
else
{
if(!(tokens[3].find("N") != std::string::npos || tokens[3].find("n") != std::string::npos))
{
return false; //No N/S letter
}
}

if(tokens[5].find("W") != std::string::npos || tokens[5].find("w") != std::string::npos)
{
longitude = -longitude;
}
else
{
if(!(tokens[5].find("E") != std::string::npos || tokens[5].find("e") != std::string::npos))
{
return false; //No E/W letter
}
}

auto checksumDelimiterPosition = tokens[14].find("*");
if(checksumDelimiterPosition == std::string::npos)
{
return false; //No checksum delimiter
}

differentialBasestationID = tokens[14].substr(0,checksumDelimiterPosition);

if(tokens[14].size() == checksumDelimiterPosition+1)
{
return false; //No checksum
}

std::string checksumCharacters = tokens[14].substr(checksumDelimiterPosition+1, 2);

int receivedChecksum;
try
{
receivedChecksum = stoi(checksumCharacters, nullptr, 16);
}
catch(const std::exception &inputException)
{
return false;
}
if(receivedChecksum < 0 || receivedChecksum > 255)
{
return false;
}


//Calculate expected checksum
if(inputNMEAStringToParse.size() < 2)
{
return false;
}
std::string charactersToCheck =
inputNMEAStringToParse.substr(dollarSignIndex+1, inputNMEAStringToParse.find("*"));

unsigned char expectedChecksum = 0;
for(int i=0; i<charactersToCheck.size()-1; i++)
{
expectedChecksum = expectedChecksum ^ charactersToCheck[i];
}


if(expectedChecksum != receivedChecksum)
{
return false;
}

return true;
}

/*
This function serializes this object the returned NMEA string.
@return: The nmea GGA sentence string representing this object
*/
std::string NMEAGGASentence::serialize()
{
std::string nmeaString;
nmeaString = std::string("$") + talkerID + std::string("GGA,");

//Convert seconds to string with 2 digits after the period
char buffer[80];
snprintf(buffer, 80, "%2.2lf", seconds);

nmeaString += std::to_string(hours) + std::to_string(minutes) + std::string(buffer)+std::string(","); //Add time

//Determine if latitude is north or south
std::string latitudeDirectionString = latitude < 0 ? "S" : "N"; //S if negative, N if positive

//Save latitude in degrees minutes.minutesInDecimal format
int latitudeDegrees = (int) fabs(latitude); //Truncate to get degrees
double latitudeMinutesRemainder = (fabs(latitude) - latitudeDegrees)*60.0;
int latitudeMinutesRemainderTrunctated = (int) latitudeMinutesRemainder;
double latitudeMinutesRemainderDecimalPart = latitudeMinutesRemainder-latitudeMinutesRemainderTrunctated;
snprintf(buffer, 80, "%02d", latitudeMinutesRemainderTrunctated);
std::string wholeMinutesPart(buffer);
snprintf(buffer, 80, "%.12lf", latitudeMinutesRemainderDecimalPart);
std::string minutesDecimalPart(buffer);

//Add latitude
nmeaString += std::to_string(latitudeDegrees) + wholeMinutesPart + minutesDecimalPart.substr(1)+ std::string(",") + latitudeDirectionString + std::string(",");

//Determine if longitude is east or west
std::string longitudeDirectionString = longitude < 0 ? "W" : "E"; //W if negative, E if positive


//Save longitude in degrees minutes.minutesInDecimal format
int longitudeDegrees = (int) fabs(longitude); //Truncate to get degrees
double longitudeMinutesRemainder = (fabs(longitude) - longitudeDegrees)*60.0;
int longitudeMinutesRemainderTruncated = (int) longitudeMinutesRemainder;
snprintf(buffer, 80, "%02d", longitudeMinutesRemainderTruncated);//We'll be skipping the leading zero
std::string minutesTruncated(buffer);

snprintf(buffer, 80, "%.12lf", longitudeMinutesRemainder - longitudeMinutesRemainderTruncated);//We'll be skipping the leading zero
std::string minutesDecimalRemainder(buffer);

//Add longitude
nmeaString += std::to_string(longitudeDegrees) +minutesTruncated+minutesDecimalRemainder.substr(1)+ std::string(",") + longitudeDirectionString + std::string(",");

//Add GPS quality indicator
nmeaString += std::to_string((int) qualityType) + std::string(","); 

//Add number of satellites
nmeaString += std::to_string(numberOfSatellitesInUse) + std::string(",");

//Add dilution of precision
snprintf(buffer, 80, "%lf", horizontalDilutionOfPrecision);
nmeaString += std::string(buffer) + std::string(",");

//Add antenna altitude
nmeaString += std::to_string(antennaAltitude) + std::string(",");

//Add meters label
nmeaString += std::string("M,");

//Add geoidal seperation
snprintf(buffer, 80, "%lf", geoidalSeparation);
nmeaString += std::string(buffer) + std::string(",M,"); //Add meters label too

//Add age of differential GPS updates (seconds)
if(differentialUpdateAge >= 0.0)
{
snprintf(buffer, 80, "%lf", differentialUpdateAge);
nmeaString += std::string(buffer); 
}
nmeaString += std::string(",");


//Add Differential Station ID
nmeaString += differentialBasestationID;

//Calculate message checksum
unsigned char checksum = 0;
for(int i=1; i<nmeaString.size(); i++)
{
checksum = checksum ^ nmeaString[i]; //XOR with all characters between $ and * (not including $ and *).
}

//Add checksum in hexadecimal
snprintf(buffer, 80, "%X", checksum);
nmeaString += std::string("*") + std::string(buffer);

//printf("%s\n", nmeaString.c_str());

return nmeaString; //Return the generated string
}
