#include "commandLineTransceiver.hpp"

using namespace pylongps;

/**
This function initializes the transceiver.
@param inputArgumentStrings: The strings forming the program arguments
@param inputNumberOfArguments: How many argument strings there are
*/
commandLineTransceiver::commandLineTransceiver(char **inputArgumentStrings, int inputNumberOfArguments) : dataTransceiver(context)
{
shouldWait = false;
addOptionSettingsToParser();

parser.parse((const char **) inputArgumentStrings, inputNumberOfArguments);

//parser.printRetrievedOptionArgumentPairs();

//Things that need to be determined:
//What caster are we using for input/output? -> done
//Are we printing out the results of a query?
//Setup input
//Setup output

if(parser.has("help"))
{
parser.printHelpMessage();
return;
}


if(!parseCasterInfoOrGetFromDefaultURL("input_caster", inputCasterConnectionInfo))
{ //Couldn't get information for caster
throw SOMException("Error resolving URL", UNKNOWN, __FILE__, __LINE__);
}


if(!parseCasterInfoOrGetFromDefaultURL("output_caster", outputCasterConnectionInfo))
{ //Couldn't get information for caster
throw SOMException("Error resolving URL", UNKNOWN, __FILE__, __LINE__);
}

if(!parser.hasOneOrMore({"C","autoconnect","read_file", "read_tcp", "read_zmq", "read_basestation"}))
{ //Not told to get any data source, so return list of basestations and then exit
if(!sendQueryAndPrintResults())
{
throw SOMException("Error resolving URL", UNKNOWN, __FILE__, __LINE__);
}
}


//Not just returning a query, so find and setup data source
transceiver dataTransceiver(context);
std::string dataReceiverString;
if(parser.has("C"))
{ //Load configuration file
if(parser.optionToAssociatedArguments.at("C").size() != 1)
{
parser.printErrorMessageRegardingOption("C");
throw SOMException("Error resolving URL", UNKNOWN, __FILE__, __LINE__);
}

transceiver_configuration configuration;
if(!loadProtobufObjectFromFile(parser.optionToAssociatedArguments.at("C")[0], configuration))
{//Unable to load configuration
parser.printErrorMessageRegardingOption("C");
throw SOMException("Error resolving URL", UNKNOWN, __FILE__, __LINE__);
}

SOM_TRY
dataTransceiver.load(configuration);
SOM_CATCH("Error, Unable to load configuration");
}
else if(parser.has("autoconnect"))
{ //Find basestation and start receiving from it
if(!constructBasestationDataReceiver())
{
throw SOMException("Unable to autoconnect", UNKNOWN, __FILE__, __LINE__);
}
shouldWait = true;
}
else if(parser.has("read_basestation"))
{ //Receive from particular basestation
if(!constructParticularBasestationDataReceiver())
{
throw SOMException("Unable connect to basestation", UNKNOWN, __FILE__, __LINE__);
}
shouldWait = true;
}
else if(parser.has("read_file"))
{ //Receive from a file
if(!constructFileDataReceiver())
{
throw SOMException("Unable to read file", UNKNOWN, __FILE__, __LINE__);
}
shouldWait = true;
}
else if(parser.has("read_tcp"))
{ //Receive from TCP server
if(!constructTCPDataReceiver())
{
throw SOMException("Unable connect to tcp server", UNKNOWN, __FILE__, __LINE__);
}
shouldWait = true;
}
else if(parser.has("read_zmq"))
{ //Receive from TCP basestation
if(!constructZMQDataReceiver())
{
throw SOMException("Unable connect to zmq publisher", UNKNOWN, __FILE__, __LINE__);
}
shouldWait = true;
}


if(parser.has("output_file"))
{
if(!constructFileDataSender())
{
throw SOMException("Unable to create file data sender", UNKNOWN, __FILE__, __LINE__);
}
}
else if(parser.has("output_tcp"))
{
if(!constructTCPDataSender())
{
throw SOMException("Unable to create tcp data sender", UNKNOWN, __FILE__, __LINE__);
}
}
else if(parser.has("output_zmq"))
{
if(!constructZMQDataSender())
{
throw SOMException("Unable to create zmq data sender", UNKNOWN, __FILE__, __LINE__);
}
}
else if(parser.has("output_basestation"))
{
if(!constructBasestationDataSender())
{
throw SOMException("Unable to create basestation data sender", UNKNOWN, __FILE__, __LINE__);
}
}

} 

/**
This function sets a parser up to restrict the number of arguments for each option and enable the parser to generate intelligent error messages when an option is invalid.
*/
void commandLineTransceiver::addOptionSettingsToParser()
{
parser.addSuggestedUsage("C", {"pathToConfigurationFile"});
parser.addSuggestedUsage("help");
parser.addSuggestedUsage("near", {"latitude", "longitude"});
parser.addSuggestedUsage("max_range", {"rangeFromPointOfInterestToSearchIn(Meters)"});
parser.addSuggestedUsage("official");
parser.addSuggestedUsage("registered");
parser.addSuggestedUsage("autoconnect");
parser.addSuggestedUsage("read_file", {"pathToReadFrom"});
parser.addSuggestedUsage("read_tcp", {"serverIP", "serverPortNumber"});
parser.addSuggestedUsage("read_zmq", {"publisherIP", "publisherPortNumber"});
parser.addSuggestedUsage("read_basestation", {"casterID", "basestationID"});
parser.addSuggestedUsage("input_caster", {"casterIPAddress", "registrationPortNumber", "clientRequestPortNumber", "clientStreamPublishingPortNumber", "proxyStreamPublishingPortNumber", "streamStatusPublishingPortNumber", "keyManagementPortNumber"});
parser.addSuggestedUsage("output_caster", {"casterIPAddress", "registrationPortNumber", "clientRequestPortNumber", "clientStreamPublishingPortNumber", "proxyStreamPublishingPortNumber", "streamStatusPublishingPortNumber", "keyManagementPortNumber"});
parser.addSuggestedUsage("output_file", {"pathToFileToWriteTo"});
parser.addSuggestedUsage("output_tcp", {"portNumber"});
parser.addSuggestedUsage("output_zmq", {"portNumber"});
parser.addSuggestedUsage("output_basestation", {"latitude",  "longitude", "messageFormat", "informalName", "expectedUpdateRate"});
parser.addSuggestedUsage("private_key", {"pathToPrivateKeyFile"});
parser.addSuggestedUsage("credential_file", {"pathToCredentialsFile"});
}

/**
This function initializes a structure containing the information required to connect to a caster using information associated with a option to the command line parser or the URL/default information associated with pylongps.com if the option is not found.
@param inputCasterOptionString: The option to get it from
@param inputCasterInfoBuffer: The structure to store it in
@return: True if the data was able to be received successfully, false otherwise
*/
bool commandLineTransceiver::parseCasterInfoOrGetFromDefaultURL(const std::string &inputCasterOptionString,  casterConnectionInfo &inputCasterInfoBuffer)
{
if(parser.optionToAssociatedArguments.count(inputCasterOptionString) == 1)
{ //Using a non-pylongps caster
if(parser.optionToAssociatedArguments.at(inputCasterOptionString).size() != 7)
{
parser.printErrorMessageRegardingOption(inputCasterOptionString);
return false;
}

const std::vector<std::string> &optionArguments = parser.optionToAssociatedArguments.at(inputCasterOptionString);
std::get<0>(inputCasterInfoBuffer) = optionArguments[0];

std::array<int64_t, 6> &portNumbers = std::get<1>(inputCasterInfoBuffer);
for(int i=1; i<optionArguments.size(); i++)
{
int64_t buffer = 0;
if(!convertStringToInteger(optionArguments[i], buffer))
{
parser.printErrorMessageRegardingOption(inputCasterOptionString);
return false;
}
portNumbers[i-1] = buffer;
}
}
else
{ //Assume default ports and get IP from DNS query
std::string pylonGPSIPAddress;

try
{
pylonGPSIPAddress = getURLIPAddress("pylongps.com");
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Error, unable to resolve pylongps.com");
return false;
}

std::get<0>(inputCasterInfoBuffer) = pylonGPSIPAddress;

std::array<int64_t, 6> &portNumbers = std::get<1>(inputCasterInfoBuffer);
portNumbers = std::array<int64_t, 6>{DEFAULT_CASTER_REGISTRATION_PORT_NUMBER, DEFAULT_CASTER_CLIENT_REQUEST_PORT_NUMBER, DEFAULT_CASTER_PROXY_STREAM_PORT_NUMBER, DEFAULT_CASTER_CLIENT_STREAM_PORT_NUMBER, DEFAULT_CASTER_STREAM_STATUS_PORT_NUMBER, DEFAULT_CASTER_KEY_MANAGEMENT_PORT_NUMBER};
}

return true;
}

/**
This function intitializes a range subquery based on the arguments in the parser.
@param inputSubqueryBuffer: The subquery to initialize
@return: true if successful and false otherwise
*/
bool commandLineTransceiver::parseAndInitializeSubquery(client_subquery &inputSubqueryBuffer)
{
if(parser.has("official"))
{
inputSubqueryBuffer.add_acceptable_classes(OFFICIAL);
}

if(parser.has("registered"))
{
inputSubqueryBuffer.add_acceptable_classes(REGISTERED_COMMUNITY);
}

if(parser.has("near"))
{
if(parser.optionToAssociatedArguments.at("near").size() != 2)
{
parser.printErrorMessageRegardingOption("near");
return false;
}
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("near");
base_station_radius_subquery *radiusSubquery = inputSubqueryBuffer.mutable_circular_search_region();

double latitudeBuffer = 0.0;
double longitudeBuffer = 0.0;
double radiusBuffer = 100000000.0; //Default 100,000 km
if(!convertStringToDouble(arguments[0], latitudeBuffer) || !convertStringToDouble(arguments[1], longitudeBuffer))
{
parser.printErrorMessageRegardingOption("near");
return false;
}
radiusSubquery->set_latitude(latitudeBuffer);
radiusSubquery->set_longitude(longitudeBuffer);

if(parser.has("max_range"))
{
const std::vector<std::string> &rangeArguments = parser.optionToAssociatedArguments.at("max_range");
if(rangeArguments.size() == 0)
{
parser.printErrorMessageRegardingOption("max_range");
return false;
}

if(!convertStringToDouble(parser.optionToAssociatedArguments.at("max_range")[0], radiusBuffer))
{
parser.printErrorMessageRegardingOption("max_range");
return false;
}
}//Max range
radiusSubquery->set_radius(radiusBuffer);

} //Near

return true;
}

/**
This function prints the results of a query to stdout.
@param inputReply: The reply to print
@param inputLatitude: The latitude the center results are sorted from, if any
@param inputLongitude: The longitude the center results are sorted from, if any
@param inputCenterPointIsValid: true if the lat/lon is valid
*/
void commandLineTransceiver::printQueryResults(const client_query_reply &inputReply, double inputLatitude, double inputLongitude, bool inputCenterPointIsValid)
{
int64_t casterID = inputReply.caster_id();

if(inputCenterPointIsValid)
{
printf("Dist: ");
}
printf("Caster: ID: Name: Format: Class: Rate: Lat: Lon: \n");

for(int i=0; i<inputReply.base_stations_size(); i++)
{
const base_station_stream_information &basestation = inputReply.base_stations(i);
double distanceFromCenter = calculateGreatCircleDistance(inputLatitude, inputLongitude, basestation.latitude(), basestation.longitude());

if(inputCenterPointIsValid)
{
printf("%.2lf km ", distanceFromCenter/100000.0);
}

std::cout <<  casterID << " " << basestation.base_station_id() << " " << basestation.informal_name() << " " << corrections_message_format_Name(basestation.message_format()) << " " << base_station_class_Name(basestation.station_class()) << " " << basestation.expected_update_rate() << " " << basestation.latitude() << " " << basestation.longitude() << std::endl;
}
}

/**
This function sends a query to the caster and prints the results of the query to stdout.
@return: true if successful and false if an error occurred
*/
bool commandLineTransceiver::sendQueryAndPrintResults()
{
client_query_reply reply;
if(!constructAndSendQuery(reply))
{
return false;
}

double latitudeBuffer = 0.0;
double longitudeBuffer = 0.0;
bool haveCenter = false;

if(parser.has("near"))
{//Already checked option validity
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("near");

convertStringToDouble(arguments[0], latitudeBuffer);
convertStringToDouble(arguments[1], longitudeBuffer);
haveCenter = true;
}

printQueryResults(reply, latitudeBuffer, longitudeBuffer, haveCenter);
return true;
}

/**
This function constructs a proper query based on the command arguments.
@param inputQueryBuffer: The buffer to store the constructed
@return: true if successful
*/
bool commandLineTransceiver::constructQuery(client_query_request &inputQueryBuffer)
{
if(parser.hasOneOrMore({"official", "registered", "near"}))
{//Need to add subquery
client_subquery *subquery = inputQueryBuffer.add_subqueries();

if(!parseAndInitializeSubquery(*subquery))
{ //Invalid options for subquery
return false;
}
}

return true;
}

/**
This function constructs a proper query based on the command arguments.
@param inputReplyBuffer: The buffer to store the received reply in
@return: true if successful
*/
bool commandLineTransceiver::constructAndSendQuery(client_query_reply &inputReplyBuffer)
{
client_query_request query;
constructQuery(query);

try
{
std::string clientPortConnectionString = std::get<0>(inputCasterConnectionInfo) + ":" + std::to_string(std::get<1>(inputCasterConnectionInfo)[1]);
inputReplyBuffer = transceiver::queryPylonGPSV2Caster(query, clientPortConnectionString, MILLISECONDS_TO_WAIT_FOR_QUERY_RESPONSE, context);
} 
catch(const std::exception &inputException)
{
fprintf(stderr, "Error, unable to query caster at %s:%lld\n", std::get<0>(inputCasterConnectionInfo).c_str(), (long long int) std::get<1>(inputCasterConnectionInfo)[1]);
}

if(inputReplyBuffer.has_failure_reason() || !inputReplyBuffer.has_caster_id())
{
fprintf(stderr, "Error, unable to query caster\n");
return false;
}

return true;
}

/*
This function queries the caster, finds first basestation that matches the query (often sorted by distance) and connects to receive its updates.
@return: true if successful
*/
bool commandLineTransceiver::constructBasestationDataReceiver()
{
client_query_reply reply;
if(!constructAndSendQuery(reply))
{
fprintf(stderr, "Error querying caster\n");
return false;
}

if(reply.base_stations_size() == 0)
{
fprintf(stderr, "Error, no basestations matching criteria found\n");
return false;
}

std::string clientPortConnectionString = std::get<0>(inputCasterConnectionInfo) + ":" + std::to_string(std::get<1>(inputCasterConnectionInfo)[3]);

try
{
dataReceiverConnectionString = dataTransceiver.createPylonGPSV2DataReceiver(clientPortConnectionString, reply.caster_id(), reply.base_stations(0).base_station_id());
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to create basestation data receiver\n");
return false;
}
return true;
}

/**
This function constructs a basestation that listens for the caster/basestation combination given using the read_basestation option.
@return: true if successful
*/
bool commandLineTransceiver::constructParticularBasestationDataReceiver()
{
std::string clientPortConnectionString = std::get<0>(inputCasterConnectionInfo) + ":" + std::to_string(std::get<1>(inputCasterConnectionInfo)[3]);

if(!parser.has("read_basestation"))
{
return false; //Shouldn't be called if option isn't there
}
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("read_basestation");
if(arguments.size() != 2)
{
parser.printErrorMessageRegardingOption("read_basestation");
return false;
}

int64_t casterID = 0;
int64_t basestationID = 0;
if(!convertStringToInteger(arguments[0], casterID) || !convertStringToInteger(arguments[1], basestationID))
{
parser.printErrorMessageRegardingOption("read_basestation");
return false;
}

try
{
dataReceiverConnectionString = dataTransceiver.createPylonGPSV2DataReceiver(clientPortConnectionString, casterID, basestationID);
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to create basestation data receiver\n");
return false;
}
return true;
}

/**
This function constructs a data receiver that reads from a file.
@return: true if successful
*/
bool commandLineTransceiver::constructFileDataReceiver()
{
if(!parser.has("read_file"))
{
return false; //Shouldn't be called if option isn't there
}
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("read_file");
if(arguments.size() != 1)
{
parser.printErrorMessageRegardingOption("read_file");
return false;
}

try
{
dataReceiverConnectionString = dataTransceiver.createFileDataReceiver(arguments[0]);
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to create file data receiver\n");
return false;
}
return true;
}

/**
This function constructs a data receiver that reads from a tcp server.
@return: true if successful
*/
bool commandLineTransceiver::constructTCPDataReceiver()
{
if(!parser.has("read_tcp"))
{
return false; //Shouldn't be called if option isn't there
}
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("read_tcp");
if(arguments.size() != 2)
{
parser.printErrorMessageRegardingOption("read_tcp");
return false;
}

try
{
dataReceiverConnectionString = dataTransceiver.createTCPDataReceiver(arguments[0] + ":" + arguments[1]);
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to create tcp data receiver\n");
return false;
}
return true;
}

/**
This function constructs a data receiver that reads from a ZMQ publisher.
@return: true if successful
*/
bool commandLineTransceiver::constructZMQDataReceiver()
{
if(!parser.has("read_zmq"))
{
return false; //Shouldn't be called if option isn't there
}
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("read_zmq");
if(arguments.size() != 2)
{
parser.printErrorMessageRegardingOption("read_zmq");
return false;
}

try
{
dataReceiverConnectionString = dataTransceiver.createTCPDataReceiver(arguments[0] + ":" + arguments[1]);
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to create zmq data receiver\n");
return false;
}
return true;
}


/**
This function constructs a data sender that writes to a file.
@return: true if successful
*/
bool commandLineTransceiver::constructFileDataSender()
{
if(!parser.has("output_file") || dataReceiverConnectionString.size() == 0)
{
return false; //Shouldn't be called if option isn't there or there isn't a data receiver
}
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("output_file");
if(arguments.size() != 1)
{
parser.printErrorMessageRegardingOption("output_file");
return false;
}

try
{
dataSenderID = dataTransceiver.createFileDataSender(dataReceiverConnectionString, arguments[0]);
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to create file data sender\n");
return false;
}
return true;
}

/**
This function constructs a data sender that creates a tcp server.
@return: true if successful
*/
bool commandLineTransceiver::constructTCPDataSender()
{
if(!parser.has("output_tcp") || dataReceiverConnectionString.size() == 0)
{
return false; //Shouldn't be called if option isn't there or there isn't a data receiver
}
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("output_tcp");
if(arguments.size() != 1)
{
parser.printErrorMessageRegardingOption("output_tcp");
return false;
}

int64_t portNumber = 0;
if(!convertStringToInteger(arguments[0], portNumber))
{
parser.printErrorMessageRegardingOption("output_tcp");
return false;
}

try
{
dataSenderID = dataTransceiver.createTCPDataSender(dataReceiverConnectionString, portNumber);
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to create tcp data sender\n");
return false;
}
return true;
}

/**
This function constructs a data sender that creates a ZMQ publisher.
@return: true if successful
*/
bool commandLineTransceiver::constructZMQDataSender()
{
if(!parser.has("output_zmq") || dataReceiverConnectionString.size() == 0)
{
return false; //Shouldn't be called if option isn't there or there isn't a data receiver
}
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("output_zmq");
if(arguments.size() != 1)
{
parser.printErrorMessageRegardingOption("output_zmq");
return false;
}

int64_t portNumber = 0;
if(!convertStringToInteger(arguments[0], portNumber))
{
parser.printErrorMessageRegardingOption("output_zmq");
return false;
}

try
{
dataSenderID = dataTransceiver.createTCPDataSender(dataReceiverConnectionString, portNumber);
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to create zmq data sender\n");
return false;
}
return true;
}

/*
This function constructs a data sender that connects and shares the data as a basestation.
@return: true if successful
*/
bool commandLineTransceiver::constructBasestationDataSender()
{
if(!parser.has("output_basestation") || dataReceiverConnectionString.size() == 0)
{
return false; //Shouldn't be called if option isn't there or there isn't a data receiver
}
const std::vector<std::string> &arguments = parser.optionToAssociatedArguments.at("output_basestation");
if(arguments.size() != 5)
{
parser.printErrorMessageRegardingOption("output_basestation");
return false;
}

double latitude = 0.0;
double longitude = 0.0;
corrections_message_format format;
std::string informalName = arguments[3];
double expectedUpdateRate = 0.0;
std::string privateKey;
credentials basestationCredentials;

if(!convertStringToDouble(arguments[0], latitude) || !convertStringToDouble(arguments[1], longitude) || !convertStringToDouble(arguments[4], expectedUpdateRate) || !corrections_message_format_Parse(arguments[2], &format))
{
parser.printErrorMessageRegardingOption("output_basestation");
return false;
}

if(parser.has("private_key") && parser.has("credentials_file"))
{
const std::vector<std::string> &privateKeyArguments = parser.optionToAssociatedArguments.at("private_key");
const std::vector<std::string> &credentialsArguments = parser.optionToAssociatedArguments.at("credentials_file");
if(privateKeyArguments.size() != 1)
{
parser.printErrorMessageRegardingOption("private_key");
return false;
}

if(credentialsArguments.size() != 1)
{
parser.printErrorMessageRegardingOption("credentials_file");
return false;
}

privateKey = loadPublicKeyFromFile(privateKeyArguments[0]);
if(privateKey.size() == 0)
{
parser.printErrorMessageRegardingOption("private_key");
return false;
}

if(!loadProtobufObjectFromFile(credentialsArguments[0], basestationCredentials))
{
parser.printErrorMessageRegardingOption("private_key");
return false;
}

}

std::string registrationPortConnectionString = std::get<0>(outputCasterConnectionInfo) + ":" + std::to_string(std::get<1>(outputCasterConnectionInfo)[0]);
try
{
if(privateKey.size() > 0)
{//Make authenticated basestation
dataSenderID = dataTransceiver.createPylonGPSV2DataSender(dataReceiverConnectionString, privateKey, basestationCredentials, registrationPortConnectionString, latitude, longitude, format, informalName, expectedUpdateRate);
}
else
{//Make community basestation
dataSenderID = dataTransceiver.createPylonGPSV2DataSender(dataReceiverConnectionString, registrationPortConnectionString, latitude, longitude, format, informalName, expectedUpdateRate);
}
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Unable to create basestation data sender\n");
return false;
}
return true;
}
