#pragma once

#include<map>
#include<set>
#include "dataReceiver.hpp"
#include "dataSender.hpp"
#include "casterDataSender.hpp"
#include "fileDataReceiver.hpp"
#include "fileDataSender.hpp"
#include "tcpDataReceiver.hpp"
#include "tcpDataSender.hpp"
#include "zmqDataReceiver.hpp"
#include "zmqDataSender.hpp"
#include "client_query_request.pb.h"
#include "client_query_reply.pb.h"
#include "transceiver_configuration.pb.h"

namespace pylongps
{

/**
This class makes it easy to use/manage dataReceiver and dataSender objects to forward data from information sources to their destinations.  It also has a static method which can be used to send a client query request to a PylonGPS 2.0 caster.

Each of the createXXXXDataReceiver functions returns a ZMQ inproc connection string that data associated with that information source is published to.  These connection strings can be passed to one or more createXXXXDataSender functions to have that data forwarded to a particular interface.
*/
class transceiver
{
public:
/**
This function initializes the transceiver to use the given ZMQ context.
@param inputContext: The ZMQ context for the transceiver to use
*/
transceiver(zmq::context_t &inputContext);

/**
This function removes all data receivers and data senders from the transceiver.
*/
void clear();

/**
This function loads a configuration and attempts to set this transceiver's receivers/senders accordingly
@param inputConfiguration: The configuration to load

@throws: This function can throw exceptions
*/
void load(const transceiver_configuration &inputConfiguration);

/**
This function creates a zmqDataReceiver which can listen to a data stream from a Pylon GPS 2.0 caster.
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@param inputCasterID: The ID of the caster to listen to (host format)
@param inputStreamID: The stream ID associated with the stream to listen to (host format)
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string createPylonGPSV2DataReceiver(const std::string &inputIPAddressAndPort, int64_t inputCasterID, int64_t inputStreamID);

/**
This function creates a zmqDataReceiver which can listen to a data stream published from a raw ZMQ PUB socket (just publishing the data).
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string createZMQPubDataReceiver(const std::string &inputIPAddressAndPort);

/**
This function creates a fileDataReceiver to retrieve data from the given file descriptor.  It takes ownership of the file descriptor and will close it when the receiver is destroyed.
@param inputFilePointer: A file stream pointer to retrieve data from
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string createFileDataReceiver(FILE *inputFilePointer);

/**
This function creates a fileDataReceiver to retrieve data from the given file descriptor.
@param inputFilePath: The path to the file to retrieve data from
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string createFileDataReceiver(const std::string &inputFilePath);

/**
This function creates a tcpDataReceiver to retrieve data from the given raw TCP server (establishes connection and then expects a data stream).
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@return: The ZMQ connection string to use to connect to this information stream (used with createXXXXDataSender functions).

@throws: This function can throw exceptions
*/
std::string createTCPDataReceiver(const std::string &inputIPAddressAndPort);

/**
This function initializes a tcpDataSender to send data to anyone who connects to the given port using TCP.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputPortNumberToPublishOn: The port number to publish on
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string createTCPDataSender(const std::string &inputSourceConnectionString, int inputPortNumberToPublishOn);

/**
This function initializes a zmqDataSender to send data to anyone who connects to the given port using a ZMQ SUB socket.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputPortNumberToPublishOn: The port number to publish on
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string createZMQDataSender(const std::string &inputSourceConnectionString, int inputPortNumberToPublishOn);

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
std::string createPylonGPSV2DataSender(const std::string &inputSourceConnectionString, const std::string &inputCasterRegistrationIPAddressAndPort, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, const std::string &inputInformalName, double inputExpectedUpdateRate = 0.0);

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
std::string createPylonGPSV2DataSender(const std::string &inputSourceConnectionString, const std::string &inputSecretSigningKey, const credentials &inputCredentials, const std::string &inputCasterRegistrationIPAddressAndPort, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, const std::string &inputInformalName, double inputExpectedUpdateRate = 0.0);

/**
This function initializes a fileDataSender to send data to the given file.  The object takes ownership of the file and closes the pointer on destruction.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputFilePointer: A file stream pointer to retrieve data from
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string createFileDataSender(const std::string &inputSourceConnectionString, FILE *inputFilePointer);

/**
This function initializes a fileDataSender to send data to the given file.  
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputFilePath: The path to the file to send data to
@return: The data sender ID to use for operations on the data sender

@throws: This function can throw exceptions
*/
std::string createFileDataSender(const std::string &inputSourceConnectionString, const std::string &inputFilePath);


/**
This function shuts down and removes the data receiver associated with the given connection string.  It also shuts down and removes all data senders in the transceiver that are listening to that data receiver.
@param inputDataReceiverConnectionString: The connection string associated with the data receiver
*/
void removeDataReceiver(const std::string &inputDataReceiverConnectionID);

/**
This function shuts down and removes the data sender associated with the given connection string.
@param inputDataSenderIDString: The ID string associated with the data sender
*/
void removeDataSender(const std::string &inputDataSenderIDString);

/**
This function can be used to send a query to a caster to get a list of all basestations that meet the query's requirements.  If an empty request is sent, the metadata for all basestations in the caster is returned.
@param inputRequest: The request to send to the caster
@param inputClientRequestIPAddressAndPort: The IP address/port to use for querying the caster in the form "IPAddress:Port"
@param inputTimeoutDuration: How many milliseconds to wait for a response (-1 for infinite wait)
@param inputContext: The ZMQ context to use for this request
@throw: This function can throw exceptions
*/
static client_query_reply queryPylonGPSV2Caster(const client_query_request &inputRequest, const std::string &inputClientRequestIPAddressAndPort, int inputTimeoutDuration, zmq::context_t &inputContext);

zmq::context_t &context;
std::map<std::string, std::unique_ptr<dataReceiver> > dataReceiverConnectionStringToDataReceiver;
std::map<std::string, std::unique_ptr<dataSender> > dataSenderIDToDataSender;
std::map<std::string, std::set<std::string> > dataReceiverConnectionStringToListeningDataSenderIDs; 

protected:
int URINumberGenerator = 0;

/**
This function adds the given data sender reference to the transceiver's maps and generates/returns the associated URI.
@param inputSourceConnectionString: The connection string that the sender is connected to
@param inputDataSender: A reference to the unique_ptr which currently owns the data sender (ownership is transferred to the dataReceiverConnectionStringToDataReceiver map)

@return: The URI that has been assigned to this sender
*/
std::string addDataSender(const std::string &inputSourceConnectionString, std::unique_ptr<dataSender> &inputDataSender);
};

























} 
