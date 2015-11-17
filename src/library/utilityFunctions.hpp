#pragma once

#include "zmq.hpp"
#include "SOMException.hpp"
#include <tuple>
#include<memory>
#include<google/protobuf/message.h>
#include <sodium.h>
#include<iostream>
#include<map>
#include "Poco/ByteOrder.h"

namespace pylongps
{

//Includes null terminating character, which is left out when it is written to a file
const int z85PublicKeySize = (((crypto_sign_PUBLICKEYBYTES*5)/4)+1)+(((crypto_sign_PUBLICKEYBYTES*5)/4))%4;
const int z85SecretKeySize = (((crypto_sign_SECRETKEYBYTES*5)/4)+1)+(((crypto_sign_SECRETKEYBYTES*5)/4))%4;

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

/**
This function calculates the signature for the given string/private signing key and preappends it to the message.
@param inputMessage: The message to sign
@param inputSigningSecretKey: The secret key to sign with (must be crypto_sign_SECRETKEYBYTES bytes)

@throws: This function can throw exceptions
*/
std::string calculateAndPreappendSignature(const std::string &inputMessage, const std::string &inputSigningSecretKey);

/**
This function generates a pair of libsodium signing keys, stored as binary strings.
@return: <public key, secret key>, sizes of crypto_sign_PUBLICKEYBYTES and crypto_sign_SECRETKEYBYTES respectively
*/
std::pair<std::string, std::string> generateSigningKeys();

/**
This function converts a string to Z85 format.
@param inputString: The string to convert
@return: The converted string

@throw: This function can throw exceptions
*/
std::string convertStringToZ85Format(const std::string &inputString);

/**
This function converts a string from Z85 format. 
@param inputZ85String: The string to convert (must be null terminated)
@return: The converted string

@throw: This function can throw exceptions
*/
std::string convertStringFromZ85Format(const std::string &inputZ85String);

/**
This function saves a string to the given file path
@param inputString: The string to save
@param inputPath: The path for the file to save to
@return: true if the string was written successfully and false otherwise
*/
bool saveStringToFile(const std::string &inputString, const std::string &inputPath);

/**
This function saves a string to the given file path
@param inputStringBuffer: The string to save the read data to
@param inputSizeOfStringToRead: The path for the file to save to
@param inputPath: The path for the file to save to
@return: true if the string was read successfully and false otherwise
*/
bool readStringFromFile(std::string &inputStringBuffer, unsigned int inputSizeOfStringToRead, const std::string &inputPath);

/**
This function loads a public key and decodes it from a Z85 encoded file.
@param inputPath: The path to load the key from
@return: a string of length crypto_sign_PUBLICKEYBYTES if successful and empty otherwise
*/
std::string loadPublicKeyFromFile(const std::string &inputPath);

/**
This function loads a secret key and decodes it from a Z85 encoded file.
@param inputPath: The path to load the key from
@return: a string of length crypto_sign_SECRETKEYBYTES if successful and empty otherwise
*/
std::string loadSecretKeyFromFile(const std::string &inputPath);

/**
This function loads a protobuf object from a file using the assumption that it is proceeded with a 64 bit Poco::Int64 in network byte order that holds the size of the object to be loaded and deserialized.
@param inputPath: The path to the file to load from
@param inputMessageBuffer: The protobuf object to deserialize to
@return: true if the message could be loaded and deserialized and false otherwise
*/
bool loadProtobufObjectFromFile(const std::string &inputPath, google::protobuf::Message &inputMessageBuffer);

/**
This function saves a protobuf object to a file with a proceeded 64 bit Poco::Int64 in network byte order that holds the size of the object to be loaded and deserialized.
@param inputPath: The path to the file to save the object to
@param inputMessageBuffer: The protobuf object to serialize
@return: true if the message could be serialized/saved and false otherwise
*/
bool saveProtobufObjectToFile(const std::string &inputPath, google::protobuf::Message &inputMessageBuffer);

/**
This function parses the passed strings and looks for arguments starting with "-".  If the next argument after a "-" argument doesn't start with "-", it stores the pair <"optionWithout-Character", "associatedValue">.  If it does start with "-", it simply stores <"optionWithout-Character", ""> and processes the next argument.
@param inputArguments: The set of strings to process
@param inputNumberOfArguments: The number of strings to process
@return: a map of form "option" -> value
*/
std::map<std::string, std::string> parseStringArguments(char **inputArguments, unsigned int inputNumberOfArguments);

/**
This function converts a string to an integer without having bad defaults or needing to throw exceptions.
@param inputString: The string to convert
@param inputIntegerBuffer: The integer variable to store the result in
@return: True if the conversion was successful and false otherwise
*/
bool convertStringToInteger(const std::string &inputString, long int &inputIntegerBuffer);

/**
This function converts a string to an integer without having bad defaults or needing to throw exceptions.
@param inputString: The string to convert
@param inputIntegerBuffer: The integer variable to store the result in
@return: True if the conversion was successful and false otherwise
*/
bool convertStringToInteger(const std::string &inputString, int &inputIntegerBuffer);

}

