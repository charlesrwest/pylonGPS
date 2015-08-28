#ifndef  UTILITYFUNCTIONSHPP
#define  UTILITYFUNCTIONSHPP

#include "zmq.hpp"
#include "SOMException.hpp"
#include <tuple>
#include<memory>
#include<google/protobuf/message.h>

#include<iostream>

namespace pylongps
{

/**
This function compactly allows binding a ZMQ socket to inproc address without needing to specify an exact address.  The function will try binding to addresses in the format: inproc://inputBaseString.inputExtensionNumberAsString and will try repeatedly while incrementing inputExtensionNumber until it succeeds or the maximum number of tries has been exceeded.
@param inputSocket: The ZMQ socket to bind
@param inputBaseString: The base string to use
@param inputExtensionNumber: The extension number to start with
@param inputMaximumNumberOfTries: How many times to try binding before giving up
@return: A tuple of form <connectionString ("inproc://etc"), extensionNumberThatWorked>

@throws: This function can throw exceptions if the bind call throws something besides "address taken" or the number of tries are exceeded
*/
std::tuple<std::string, int> bindZMQSocketWithAutomaticAddressGeneration(zmq::socket_t &inputSocket, const std::string &inputBaseString = "", int inputExtensionNumber = 0, unsigned int inputMaximumNumberOfTries = 1000);

template<typename inputClass> void print(const inputClass &inputObject)
{
std::cout << inputObject << std::endl;
}

/**
This function is used to send a protobuf object as a ZMQ message, with optional preappended or postappended data.  It manages serialization and will throw an exception if required fields of the message are missing.
@param inputSocketToSendFrom: The socket to send from
@param inputMessage: The message to send
@param inputDataToPreappend: The data to place in the ZMQ message before the serialized protobuf object
@param inputDataToPostAppend: The data to place in the ZMQ message after the serialized protobuf object

@throws: This function can throw exceptions
*/
void sendProtobufMessage(zmq::socket_t &inputSocketToSendFrom, const google::protobuf::Message &inputMessage, const std::string &inputDataToPreappend = "", const std::string &inputDataToPostAppend = "");

/**
This function is used to receive and deserialize a protobuf object from a ZMQ socket.
@param inputSocketToReceiveFrom: This is the socket to receive the object from
@param inputMessageBuffer: This is the buffer to place the received object in
@param inputFlags: The flags to pass to the ZMQ socket
@param inputPreappendedDataBuffer: The buffer to place x bytes before the message in
@param inputPreappendedDataSize: How much data to expect to be preappended
@param inputPostappendedDataBuffer: The buffer to place x bytes before the message in
@param inputPostappendedDataSize: How much data to expect to be postappended
@return: <true if message received, true if message deserialized correctly>

@throws: This function can throw exceptions
*/
std::tuple<bool, bool> receiveProtobufMessage(zmq::socket_t &inputSocketToReceiveFrom, google::protobuf::Message &inputMessageBuffer, int inputFlags = 0, char *inputPreappendedDataBuffer = nullptr, int inputPreappendedDataSize = 0, char *inputPostappendedDataBuffer = nullptr, int inputPostappendedDataSize = 0);

/**
This function sends the given protobuf object as a request using the given socket.  It then waits for a reply and deserializes the received object, placing it in the buffer.
@param inputSocketToSendAndReceiveFrom: The ZMQ socket to send the request with and get back the reply from
@param inputRequestMessage: The message to send to the other side of the connection
@param inputMessageReplyBuffer: The buffer to place the deserialized reply in
@return: <true if message received, true if message deserialized correctly>

@throws: This function can throw exceptions
*/
std::tuple<bool, bool> remoteProcedureCall(zmq::socket_t &inputSocketToSendAndReceiveFrom, const google::protobuf::Message &inputRequestMessage, google::protobuf::Message &inputMessageReplyBuffer);

}

#endif
