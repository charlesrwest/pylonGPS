#include "utilityFunctions.hpp"

 /**
This function compactly allows binding a ZMQ socket to inproc address without needing to specify an exact address.  The function will try binding to addresses in the format: inproc://inputBaseString.inputExtensionNumberAsString and will try repeatedly while incrementing inputExtensionNumber until it succeeds or the maximum number of tries has been exceeded.
@param inputSocket: The ZMQ socket to bind
@param inputBaseString: The base string to use
@param inputExtensionNumber: The extension number to start with
@param inputMaximumNumberOfTries: How many times to try binding before giving up
@return: A tuple of form <connectionString ("inproc://etc"), extensionNumberThatWorked>

@throws: This function can throw exceptions if the bind call throws something besides "address taken" or the number of tries are exceeded
*/
std::tuple<std::string, int> pylongps::bindZMQSocketWithAutomaticAddressGeneration(zmq::socket_t &inputSocket, const std::string &inputBaseString, int inputExtensionNumber, unsigned int inputMaximumNumberOfTries)
{
bool socketBindSuccessful = false;
std::string connectionString;
int extensionNumber = inputExtensionNumber;

for(int i=0; i<inputMaximumNumberOfTries; i++)
{
try //Attempt to bind the socket
{
connectionString = std::string("inproc://") + inputBaseString + std::string(".") + std::to_string(extensionNumber);
inputSocket.bind(connectionString.c_str());
socketBindSuccessful = true; //Got this far without throwing
break;
}
catch(const zmq::error_t &inputZMQError)
{
if(inputZMQError.num() == EADDRINUSE)
{
extensionNumber++; //Increment so the next attempted address won't conflict
}
else
{
throw SOMException(std::string("Error binding socket") + connectionString + std::string("\n"), ZMQ_ERROR, __FILE__, __LINE__);
}
}

}

if(!socketBindSuccessful)
{
throw SOMException(std::string("Socket bind did not succeed in ") + std::to_string(inputMaximumNumberOfTries) + std::string(" attempts\n"), UNKNOWN, __FILE__, __LINE__);
}

return make_tuple(connectionString, extensionNumber);
}

/**
This function is used to send a protobuf object as a ZMQ message, with optional preappended or postappended data.  It manages serialization and will throw an exception if required fields of the message are missing.
@param inputSocketToSendFrom: The socket to send from
@param inputMessage: The message to send
@param inputDataToPreappend: The data to place in the ZMQ message before the serialized protobuf object
@param inputDataToPostAppend: The data to place in the ZMQ message after the serialized protobuf object

@throws: This function can throw exceptions
*/
void pylongps::sendProtobufMessage(zmq::socket_t &inputSocketToSendFrom, const google::protobuf::Message &inputMessage, const std::string &inputDataToPreappend, const std::string &inputDataToPostAppend)
{
std::string serializedMessage;

SOM_TRY
inputMessage.SerializeToString(&serializedMessage);
SOM_CATCH("Error serializing message to string\n")

std::string dataToSend = inputDataToPreappend + serializedMessage + inputDataToPostAppend;

SOM_TRY
inputSocketToSendFrom.send(dataToSend.c_str(), dataToSend.size());
SOM_CATCH("Error, unable to send message\n")
}

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
std::tuple<bool, bool> pylongps::receiveProtobufMessage(zmq::socket_t &inputSocketToReceiveFrom, google::protobuf::Message &inputMessageBuffer, int inputFlags, char *inputPreappendedDataBuffer, int inputPreappendedDataSize, char *inputPostappendedDataBuffer, int inputPostappendedDataSize)
{
if(inputPreappendedDataSize < 0 || inputPostappendedDataSize < 0)
{
throw SOMException("Negative data size\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if((inputPreappendedDataSize > 0 && inputPreappendedDataBuffer == nullptr) || (inputPostappendedDataSize < 0 && inputPostappendedDataBuffer == nullptr))
{
throw SOMException("Data size > 0 but buffer is nullptr\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

bool messageReceived = false;
bool messageDeserialized = false;

std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY
messageReceived = inputSocketToReceiveFrom.recv(messageBuffer.get(), inputFlags);
SOM_CATCH("Error, unable to receive message\n")

if(messageReceived == false)
{ //Didn't get a message
return std::tuple<bool, bool>(messageReceived, messageDeserialized);
}

//Attempt to retrieve any preappended data
if(messageBuffer->size() < inputPreappendedDataSize)
{ //Message isn't valid (smaller than expected preappended data)
return std::tuple<bool, bool>(messageReceived, messageDeserialized);
}
else if(inputPreappendedDataSize > 0)
{ //Preappended data is expected and the message is large enough to get it
memcpy((void *) inputPreappendedDataBuffer, messageBuffer->data(), inputPreappendedDataSize);
}

//Deserialize message
inputMessageBuffer.ParseFromArray(((char *) messageBuffer->data())+inputPreappendedDataSize, messageBuffer->size()-inputPreappendedDataSize);

if(!inputMessageBuffer.IsInitialized())
{
return std::tuple<bool, bool>(messageReceived, messageDeserialized);
}

if(inputPostappendedDataSize > 0)
{
int protobufMessageSize = inputMessageBuffer.ByteSize();

if((protobufMessageSize+inputPreappendedDataSize+inputPostappendedDataSize) > messageBuffer->size())
{
return std::tuple<bool, bool>(messageReceived, messageDeserialized);
}
else
{
memcpy((void *) inputPostappendedDataBuffer, (void *) (((char *) messageBuffer->data()) + protobufMessageSize + inputPreappendedDataSize), inputPostappendedDataSize);
messageDeserialized = true;
}

}
else
{
messageDeserialized = true;
}

return std::tuple<bool, bool>(messageReceived, messageDeserialized);
}

/**
This function sends the given protobuf object as a request using the given socket.  It then waits for a reply and deserializes the received object, placing it in the buffer.
@param inputSocketToSendAndReceiveFrom: The ZMQ socket to send the request with and get back the reply from
@param inputRequestMessage: The message to send to the other side of the connection
@param inputMessageReplyBuffer: The buffer to place the deserialized reply in
@return: <true if message received, true if message deserialized correctly>

@throws: This function can throw exceptions
*/
std::tuple<bool, bool> pylongps::remoteProcedureCall(zmq::socket_t &inputSocketToSendAndReceiveFrom, const google::protobuf::Message &inputRequestMessage, google::protobuf::Message &inputMessageReplyBuffer)
{
SOM_TRY
sendProtobufMessage(inputSocketToSendAndReceiveFrom, inputRequestMessage);
SOM_CATCH("Error sending request object\n")

SOM_TRY
return receiveProtobufMessage(inputSocketToSendAndReceiveFrom, inputMessageReplyBuffer);
SOM_CATCH("Error, unable to get reply message\n")
}
