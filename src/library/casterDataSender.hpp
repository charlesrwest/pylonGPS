#pragma once

#include<cstdint>
#include<memory>
#include<thread>
#include<string>
#include<cstdio>
#include<unistd.h>

#include "dataSender.hpp"
#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "zmq.hpp"
#include "reactor.hpp"
#include "utilityFunctions.hpp"
#include "data_receiver_status_notification.pb.h" 
#include "transmitter_registration_request.pb.h"
#include "transmitter_registration_reply.pb.h"
#include "credentials.pb.h"
#include <sodium.h>

namespace pylongps
{

const int CASTER_DATA_SENDER_MAX_WAIT_TIME = 1000; //Milliseconds

/**
This class takes data published on an inproc ZMQ PUB socket and forwards it to PylonGPS caster.
*/
class casterDataSender
{
public:
/**
This function initializes the casterDataSender to establish a connection and register an unauthenticated basestation with it.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputCasterRegistrationIPAddressAndPort: The IP address/port to use for registration with the caster in the form "IPAddress:Port"
@param inputLatitude: The approximate latitude of the basestation
@param inputLongitude: The approximate longitude of the basestation
@param inputMessageFormat: The message format used with the updates
@param inputInformalName: The name that should be displayed when the basestation appears in a list
@param inputExpectedUpdateRate: Expected updates per second 

@throws: This function can throw exceptions
*/
casterDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, const std::string &inputCasterRegistrationIPAddressAndPort, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, const std::string &inputInformalName, double inputExpectedUpdateRate = 0.0);

/**
This function initializes the casterDataSender to establish a connection and register an authenticated basestation with it.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputSecretSigningKey: The libsodium signing secret key (crypto_sign_SECRETKEYBYTES length)
@param inputCredentials: The credentials message that has been generated for use with this signing key
@param inputCasterRegistrationIPAddressAndPort: The IP address/port to use for registration with the caster in the form "IPAddress:Port"
@param inputLatitude: The approximate latitude of the basestation
@param inputLongitude: The approximate longitude of the basestation
@param inputMessageFormat: The message format used with the updates
@param inputInformalName: The name that should be displayed when the basestation appears in a list
@param inputExpectedUpdateRate: Expected updates per second 
@param inputIsAuthenticatedConnection: Flag to allow function to be used to create unauthenticated connections so that it can be used a delegate constructor

@throws: This function can throw exceptions
*/
casterDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, const std::string &inputSecretSigningKey, const credentials &inputCredentials, const std::string &inputCasterRegistrationIPAddressAndPort, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, const std::string &inputInformalName, double inputExpectedUpdateRate = 0.0, bool inputIsAuthenticatedConnection = true);




/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
virtual std::string notificationAddress();

zmq::context_t &context;
std::unique_ptr<zmq::socket_t> subscriberSocket;
std::unique_ptr<zmq::socket_t> sendingSocket;
std::unique_ptr<zmq::socket_t> notificationPublishingSocket;
std::string informationSourceConnectionString; //String used to connect to the data source
std::string notificationConnectionString; //String used to publish status changes (such as unrecoverable disconnects)
std::unique_ptr<reactor<casterDataSender> > senderReactor;

protected:
//Data used for authenticated connections
std::string secretKey;
credentials basestationCredentialsMessage;

/**
This function forwards any received messages to the "sending" socket
@param inputReactor: The reactor which called the function
@param inputSocket: The ZMQ socket to read from

@throw: This function can throw exceptions
*/
bool readAndWriteData(reactor<casterDataSender> &inputReactor, zmq::socket_t &inputSocket);
};

}
