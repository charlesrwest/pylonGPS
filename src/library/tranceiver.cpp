#include "transceiver.hpp"

using namespace pylongps;

/**
This function initializes the transceiver to use the given ZMQ context.
@param inputContext: The ZMQ context for the transceiver to use
*/
transceiver::transceiver(zmq::context_t &inputContext) : context(inputContext)
{
}

/**
This function removes all data receivers and data senders from the transceiver.
*/
void transceiver::clear()
{
while(dataReceiverConnectionStringToDataReceiver.size() > 0)
{
for(const auto &instance : dataReceiverConnectionStringToDataReceiver)
{
SOM_TRY
removeDataReceiver(instance.first);
SOM_CATCH("Error removing data receiver\n")
break;
}
}
}

/**
This function loads a configuration and attempts to set this transceiver's receivers/senders accordingly
@param inputConfiguration: The configuration to load

@throws: This function can throw exceptions
*/
void transceiver::load(const transceiver_configuration &inputConfiguration)
{
clear(); //Clear the current configuration
std::string casterIPAddress;
if(inputConfiguration.basestation_receivers_size() > 0 || inputConfiguration.basestation_senders_size() > 0)
{
SOM_TRY
casterIPAddress = getURLIPAddress("pylongps.com");
SOM_CATCH("Error, unable to resolve pylongps.com")
}

std::map<int64_t, std::string> receiverIDToConnectionString;

for(int i=0; i<inputConfiguration.basestation_receivers_size(); i++)
{ //Load basestation receiver
const basestation_data_receiver_configuration &dataReceiverConfiguration = inputConfiguration.basestation_receivers(i);

if(receiverIDToConnectionString.count(dataReceiverConfiguration.receiver_id()) != 0 || !dataReceiverConfiguration.has_basestation_details())
{ //Skip receiver if one with the same ID has already been loaded or we don't have details to display
continue;
}

std::string connectionString;
SOM_TRY
connectionString = createPylonGPSV2DataReceiver(casterIPAddress + ":" + std::to_string(DEFAULT_CASTER_CLIENT_STREAM_PORT_NUMBER),dataReceiverConfiguration.caster_id(), dataReceiverConfiguration.stream_id());
SOM_CATCH("Error, unable to create basestation data receiver\n")

receiverIDToConnectionString[dataReceiverConfiguration.receiver_id()] = connectionString;
}

for(int i=0; i<inputConfiguration.file_receivers_size(); i++)
{ //Load file receiver
const file_data_receiver_configuration &dataReceiverConfiguration = inputConfiguration.file_receivers(i);

if(receiverIDToConnectionString.count(dataReceiverConfiguration.receiver_id()) != 0)
{ //Skip receiver if one with the same ID has already been loaded
continue;
}

std::string connectionString;
SOM_TRY
connectionString = createFileDataReceiver(dataReceiverConfiguration.file_path());
SOM_CATCH("Error, unable to create file data receiver\n")

receiverIDToConnectionString[dataReceiverConfiguration.receiver_id()] = connectionString;
}

for(int i=0; i<inputConfiguration.tcp_receivers_size(); i++)
{ //Load tcp receiver
const tcp_data_receiver_configuration &dataReceiverConfiguration = inputConfiguration.tcp_receivers(i);

if(receiverIDToConnectionString.count(dataReceiverConfiguration.receiver_id()) != 0)
{ //Skip receiver if one with the same ID has already been loaded
continue;
}

std::string connectionString;
SOM_TRY
connectionString = createTCPDataReceiver(dataReceiverConfiguration.server_ip_address() + ":" + std::to_string(dataReceiverConfiguration.server_port()));
SOM_CATCH("Error creating tcp data receiver\n")

receiverIDToConnectionString[dataReceiverConfiguration.receiver_id()] = connectionString;
}

for(int i=0; i<inputConfiguration.zmq_receivers_size(); i++)
{ //Load zmq receiver
const zmq_data_receiver_configuration &dataReceiverConfiguration = inputConfiguration.zmq_receivers(i);

if(receiverIDToConnectionString.count(dataReceiverConfiguration.receiver_id()) != 0)
{ //Skip receiver if one with the same ID has already been loaded
continue;
}

std::string connectionString;
SOM_TRY
connectionString = createZMQPubDataReceiver(dataReceiverConfiguration.server_ip_address() + ":" + std::to_string(dataReceiverConfiguration.server_port()));
SOM_CATCH("Error, unable to make ZMQ data receiver\n")
if(connectionString.size() == 0)
{
continue;
}

receiverIDToConnectionString[dataReceiverConfiguration.receiver_id()] = connectionString;
}


//Add data senders
for(int i=0; i<inputConfiguration.basestation_senders_size(); i++)
{ //Load basestation sender
const basestation_data_sender_configuration &dataSenderConfiguration = inputConfiguration.basestation_senders(i);

if(receiverIDToConnectionString.count(dataSenderConfiguration.receiver_id()) == 0)
{ //Skip sender if the associated receiver hasn't been loaded
continue;
}

const std::string &receiverConnectionString = receiverIDToConnectionString.at(dataSenderConfiguration.receiver_id());
const std::string casterConnectionString = casterIPAddress + ":" + std::to_string(DEFAULT_CASTER_REGISTRATION_PORT_NUMBER);

if(dataSenderConfiguration.has_registration_credentials())
{
SOM_TRY
createPylonGPSV2DataSender(receiverConnectionString, dataSenderConfiguration.secret_key(), dataSenderConfiguration.registration_credentials(), casterConnectionString, dataSenderConfiguration.latitude(), dataSenderConfiguration.longitude(), dataSenderConfiguration.message_format(), dataSenderConfiguration.informal_basestation_name(), dataSenderConfiguration.expected_update_rate());
SOM_CATCH("Error creating basestation sender\n")
}
else
{
SOM_TRY
createPylonGPSV2DataSender(receiverConnectionString, casterConnectionString, dataSenderConfiguration.latitude(), dataSenderConfiguration.longitude(), dataSenderConfiguration.message_format(), dataSenderConfiguration.informal_basestation_name(), dataSenderConfiguration.expected_update_rate());
SOM_CATCH("Error creating basestation sender\n")
}
}

for(int i=0; i<inputConfiguration.file_senders_size(); i++)
{ //Load file sender
const file_data_sender_configuration &dataSenderConfiguration = inputConfiguration.file_senders(i);

if(receiverIDToConnectionString.count(dataSenderConfiguration.receiver_id()) == 0)
{ //Skip sender if the associated receiver hasn't been loaded
continue;
}

const std::string &receiverConnectionString = receiverIDToConnectionString.at(dataSenderConfiguration.receiver_id());

SOM_TRY
createFileDataSender(receiverConnectionString, dataSenderConfiguration.file_path_to_write_to());
SOM_CATCH("Error, unable to make file data sender\n")
}

for(int i=0; i<inputConfiguration.tcp_senders_size(); i++)
{ //Load tcp sender
const tcp_data_sender_configuration &dataSenderConfiguration = inputConfiguration.tcp_senders(i);

if(receiverIDToConnectionString.count(dataSenderConfiguration.receiver_id()) == 0)
{ //Skip sender if the associated receiver hasn't been loaded
continue;
}

const std::string &receiverConnectionString = receiverIDToConnectionString.at(dataSenderConfiguration.receiver_id());

SOM_TRY
createTCPDataSender(receiverConnectionString, dataSenderConfiguration.server_port());
SOM_CATCH("Error, unable to create data sender\n")
}

for(int i=0; i<inputConfiguration.zmq_senders_size(); i++)
{ //Load zmq sender
const zmq_data_sender_configuration &dataSenderConfiguration = inputConfiguration.zmq_senders(i);

if(receiverIDToConnectionString.count(dataSenderConfiguration.receiver_id()) == 0)
{ //Skip sender if the associated receiver hasn't been loaded
continue;
}

const std::string &receiverConnectionString = receiverIDToConnectionString.at(dataSenderConfiguration.receiver_id());

SOM_TRY
createZMQDataSender(receiverConnectionString, dataSenderConfiguration.server_port());
SOM_CATCH("Error, unable to create ZMQ data sender\n")
}

}

/**
This function creates a zmqDataReceiver which can listen to a data stream from a Pylon GPS 2.0 caster.
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@param inputCasterID: The ID of the caster to listen to (host format)
@param inputStreamID: The stream ID associated with the stream to listen to (host format)
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string transceiver::createPylonGPSV2DataReceiver(const std::string &inputIPAddressAndPort, int64_t inputCasterID, int64_t inputStreamID)
{
std::unique_ptr<dataReceiver> receiver;

SOM_TRY
receiver.reset((dataReceiver *) new zmqDataReceiver(inputIPAddressAndPort, inputCasterID, inputStreamID, context));
SOM_CATCH("Error, unable to initialize zmqDataReceiver\n")

std::string address = receiver->address();

dataReceiverConnectionStringToDataReceiver.emplace(address, std::move(receiver));

return address;
}

/**
This function creates a zmqDataReceiver which can listen to a data stream published from a raw ZMQ PUB socket (just publishing the data).
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string transceiver::createZMQPubDataReceiver(const std::string &inputIPAddressAndPort)
{
std::unique_ptr<zmqDataReceiver> receiver;

SOM_TRY
receiver.reset(new zmqDataReceiver(inputIPAddressAndPort, context));
SOM_CATCH("Error, unable to initialize zmqDataReceiver\n")

std::string address = receiver->address();

dataReceiverConnectionStringToDataReceiver.emplace(address, std::move(receiver));

return address;
}

/**
This function creates a fileDataReceiver to retrieve data from the given file descriptor.  It takes ownership of the file descriptor and will close it when the receiver is destroyed.
@param inputFilePointer: A file stream pointer to retrieve data from
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string transceiver::createFileDataReceiver(FILE *inputFilePointer)
{
std::unique_ptr<fileDataReceiver> receiver;

SOM_TRY
receiver.reset(new fileDataReceiver(inputFilePointer, context));
SOM_CATCH("Error, unable to initialize fileDataReceiver\n")

std::string address = receiver->address();

dataReceiverConnectionStringToDataReceiver.emplace(address, std::move(receiver));

return address;
}

/**
This function creates a fileDataReceiver to retrieve data from the given file descriptor.
@param inputFilePath: The path to the file to retrieve data from
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string transceiver::createFileDataReceiver(const std::string &inputFilePath)
{
std::unique_ptr<fileDataReceiver> receiver;

SOM_TRY
receiver.reset(new fileDataReceiver(inputFilePath, context));
SOM_CATCH("Error, unable to initialize fileDataReceiver\n")

std::string address = receiver->address();

dataReceiverConnectionStringToDataReceiver.emplace(address, std::move(receiver));

return address;
}

/**
This function creates a tcpDataReceiver to retrieve data from the given raw TCP server (establishes connection and then expects a data stream).
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string transceiver::createTCPDataReceiver(const std::string &inputIPAddressAndPort)
{
std::unique_ptr<tcpDataReceiver> receiver;

SOM_TRY
receiver.reset(new tcpDataReceiver(inputIPAddressAndPort, context));
SOM_CATCH("Error, unable to initialize tcpDataReceiver\n")

std::string address = receiver->address();

dataReceiverConnectionStringToDataReceiver.emplace(address, std::move(receiver));

return address;
}


/**
This function initializes a tcpDataSender to send data to anyone who connects to the given port using TCP.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputPortNumberToPublishOn: The port number to publish on
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string transceiver::createTCPDataSender(const std::string &inputSourceConnectionString, int inputPortNumberToPublishOn)
{
std::unique_ptr<dataSender> sender;

SOM_TRY
sender.reset((dataSender *) new tcpDataSender(inputSourceConnectionString, context, inputPortNumberToPublishOn));
SOM_CATCH("Error, unable to initialize tcpDataSender\n")

return addDataSender(inputSourceConnectionString, sender);
}

/**
This function initializes a zmqDataSender to send data to anyone who connects to the given port using a ZMQ SUB socket.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputPortNumberToPublishOn: The port number to publish on
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string transceiver::createZMQDataSender(const std::string &inputSourceConnectionString, int inputPortNumberToPublishOn)
{
std::unique_ptr<dataSender> sender;

SOM_TRY
sender.reset((dataSender *) new zmqDataSender(inputSourceConnectionString, context, inputPortNumberToPublishOn));
SOM_CATCH("Error, unable to initialize tcpDataSender\n")

return addDataSender(inputSourceConnectionString, sender);
}


/**
This function initializes a casterDataSender to establish a connection and register an unauthenticated basestation with it.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputCasterRegistrationIPAddressAndPort: The IP address/port to use for registration with the caster in the form "IPAddress:Port"
@param inputLatitude: The approximate latitude of the basestation
@param inputLongitude: The approximate longitude of the basestation
@param inputMessageFormat: The message format used with the updates
@param inputInformalName: The name that should be displayed when the basestation appears in a list
@param inputExpectedUpdateRate: Expected updates per second 
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string transceiver::createPylonGPSV2DataSender(const std::string &inputSourceConnectionString, const std::string &inputCasterRegistrationIPAddressAndPort, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, const std::string &inputInformalName, double inputExpectedUpdateRate)
{
std::unique_ptr<dataSender> sender;

SOM_TRY
sender.reset((dataSender *) new casterDataSender(inputSourceConnectionString, context, inputCasterRegistrationIPAddressAndPort, inputLatitude, inputLongitude, inputMessageFormat, inputInformalName, inputExpectedUpdateRate));
SOM_CATCH("Error, unable to initialize casterDataSender\n")

return addDataSender(inputSourceConnectionString, sender);
}

/**
This function initializes a casterDataSender to establish a connection and register an authenticated basestation with it.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputSecretSigningKey: The libsodium signing secret key (crypto_sign_SECRETKEYBYTES length) to use
@param inputCredentials: The credentials message that has been generated for use with this signing key
@param inputCasterRegistrationIPAddressAndPort: The IP address/port to use for registration with the caster in the form "IPAddress:Port"
@param inputLatitude: The approximate latitude of the basestation
@param inputLongitude: The approximate longitude of the basestation
@param inputMessageFormat: The message format used with the updates
@param inputInformalName: The name that should be displayed when the basestation appears in a list
@param inputExpectedUpdateRate: Expected updates per second 
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string transceiver::createPylonGPSV2DataSender(const std::string &inputSourceConnectionString, const std::string &inputSecretSigningKey, const credentials &inputCredentials, const std::string &inputCasterRegistrationIPAddressAndPort, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, const std::string &inputInformalName, double inputExpectedUpdateRate)
{
std::unique_ptr<dataSender> sender;

SOM_TRY
sender.reset((dataSender *) new casterDataSender(inputSourceConnectionString, context, inputSecretSigningKey, inputCredentials, inputCasterRegistrationIPAddressAndPort, inputLatitude, inputLongitude, inputMessageFormat, inputInformalName, inputExpectedUpdateRate));
SOM_CATCH("Error, unable to initialize casterDataSender\n")

return addDataSender(inputSourceConnectionString, sender);
}

/**
This function initializes a fileDataSender to send data to the given file.  The object takes ownership of the file and closes the pointer on destruction.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputFilePointer: A file stream pointer to retrieve data from
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string transceiver::createFileDataSender(const std::string &inputSourceConnectionString, FILE *inputFilePointer)
{
std::unique_ptr<dataSender> sender;

SOM_TRY
sender.reset((dataSender *) new fileDataSender(inputSourceConnectionString, context, inputFilePointer));
SOM_CATCH("Error, unable to initialize fileDataSender\n")

return addDataSender(inputSourceConnectionString, sender);
}

/**
This function initializes a fileDataSender to send data to the given file.  
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputFilePath: The path to the file to send data to
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string transceiver::createFileDataSender(const std::string &inputSourceConnectionString, const std::string &inputFilePath)
{
std::unique_ptr<dataSender> sender;

SOM_TRY
sender.reset((dataSender *) new fileDataSender(inputSourceConnectionString, context, inputFilePath));
SOM_CATCH("Error, unable to initialize fileDataSender\n")

return addDataSender(inputSourceConnectionString, sender);
}

/**
This function shuts down and removes the data receiver associated with the given connection string.  It also shuts down and removes all data senders in the transceiver that are listening to that data receiver.
@param inputDataReceiverConnectionString: The connection string associated with the data receiver
*/
void transceiver::removeDataReceiver(const std::string &inputDataReceiverConnectionID)
{
if(dataReceiverConnectionStringToListeningDataSenderIDs.count(inputDataReceiverConnectionID) > 0)
{//Remove all affiliated data senders
std::set<std::string> &associatedSenderIDs = dataReceiverConnectionStringToListeningDataSenderIDs.at(inputDataReceiverConnectionID);

for(const std::string &senderID : associatedSenderIDs)
{
dataSenderIDToDataSender.erase(senderID);
}
}

dataReceiverConnectionStringToListeningDataSenderIDs.erase(inputDataReceiverConnectionID);
dataReceiverConnectionStringToDataReceiver.erase(inputDataReceiverConnectionID);
}

/**
This function shuts down and removes the data sender associated with the given connection string.
@param inputDataSenderIDString: The ID string associated with the data sender
*/
void transceiver::removeDataSender(const std::string &inputDataSenderIDString)
{
dataSenderIDToDataSender.erase(inputDataSenderIDString);

for(auto instance : dataReceiverConnectionStringToListeningDataSenderIDs)
{
if(instance.second.count(inputDataSenderIDString) > 0)
{
instance.second.erase(inputDataSenderIDString);
break;
}
}
}

/**
This function can be used to send a query to a caster to get a list of all basestations that meet the query's requirements.  If an empty request is sent, the metadata for all basestations in the caster is returned.
@param inputRequest: The request to send to the caster
@param inputClientRequestIPAddressAndPort: The IP address/port to use for querying the caster in the form "IPAddress:Port"
@param inputTimeoutDuration: How many milliseconds to wait for a response (-1 for infinite wait)
@param inputContext: The ZMQ context to use for this request

@throw: This function can throw exceptions
*/
client_query_reply transceiver::queryPylonGPSV2Caster(const client_query_request &inputRequest, const std::string &inputClientRequestIPAddressAndPort, int inputTimeoutDuration, zmq::context_t &inputContext)
{
std::unique_ptr<zmq::socket_t> clientSocket;

SOM_TRY //Init socket
clientSocket.reset(new zmq::socket_t(inputContext, ZMQ_REQ));
SOM_CATCH("Error making socket\n")

SOM_TRY
clientSocket->setsockopt(ZMQ_RCVTIMEO, (void *) &inputTimeoutDuration, sizeof(inputTimeoutDuration));
SOM_CATCH("Error setting socket timeout\n")

std::string connectionString = "tcp://" +inputClientRequestIPAddressAndPort;
SOM_TRY //Connect to caster
clientSocket->connect(connectionString.c_str());
SOM_CATCH("Error connecting socket for registration with caster\n")

SOM_TRY
sendProtobufMessage(*clientSocket, inputRequest);
SOM_CATCH("Error, unable to send client request\n")

bool messageReceived = false;
bool messageDeserialized = false;
client_query_reply reply;

SOM_TRY
std::tie(messageReceived, messageDeserialized) = receiveProtobufMessage(*clientSocket, reply);
SOM_CATCH("Error getting reply\n")

if(messageReceived == false || messageDeserialized == false)
{
throw SOMException("Invalid response from caster\n", INCORRECT_SERVER_RESPONSE, __FILE__, __LINE__);
}

return reply;
}

/**
This function adds the given data sender reference to the transceiver's maps and generates/returns the associated URI.
@param inputSourceConnectionString: The connection string that the sender is connected to
@param inputDataSender: A reference to the unique_ptr which currently owns the data sender (ownership is transferred to the dataReceiverConnectionStringToDataReceiver map)

@return: The URI that has been assigned to this sender
*/
std::string transceiver::addDataSender(const std::string &inputSourceConnectionString, std::unique_ptr<dataSender> &inputDataSender)
{
std::string URI = std::to_string(URINumberGenerator);
URINumberGenerator++;

dataSenderIDToDataSender.emplace(URI, std::move(inputDataSender));
if(dataReceiverConnectionStringToListeningDataSenderIDs.count(inputSourceConnectionString) == 0)
{
dataReceiverConnectionStringToListeningDataSenderIDs.emplace(inputSourceConnectionString, std::set<std::string>());
}

dataReceiverConnectionStringToListeningDataSenderIDs.at(inputSourceConnectionString).insert(URI);

return URI;
}
