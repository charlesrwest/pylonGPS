#include "utilityFunctions.hpp"

using namespace pylongps;

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

/**
This function calculates the signature for the given string/private signing key and preappends it to the message.
@param inputMessage: The message to sign
@param inputSigningSecretKey: The secret key to sign with (must be crypto_sign_SECRETKEYBYTES bytes)

@throws: This function can throw exceptions
*/
std::string pylongps::calculateAndPreappendSignature(const std::string &inputMessage, const std::string &inputSigningSecretKey)
{
if(inputSigningSecretKey.size() != crypto_sign_SECRETKEYBYTES)
{
throw SOMException("Invalid signing secret key\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Generate signature
unsigned char cryptoSignature[crypto_sign_BYTES];

crypto_sign_detached(cryptoSignature, nullptr, (const unsigned char *) inputMessage.c_str(), inputMessage.size(), (const unsigned char *) inputSigningSecretKey.c_str());

return std::string((const char *) cryptoSignature, crypto_sign_BYTES) + inputMessage;
}

/**
This function generates a pair of libsodium signing keys, stored as binary strings.
@return: <public key, secret key>, sizes of crypto_sign_PUBLICKEYBYTES and crypto_sign_SECRETKEYBYTES respectively
*/
std::pair<std::string, std::string> pylongps::generateSigningKeys()
{
//Generate signing key pair
unsigned char publicKeyArray[crypto_sign_PUBLICKEYBYTES];
unsigned char secretKeyArray[crypto_sign_SECRETKEYBYTES];
crypto_sign_keypair(publicKeyArray, secretKeyArray);

std::string publicKey((const char *) publicKeyArray, crypto_sign_PUBLICKEYBYTES);
std::string secretKey((const char *) secretKeyArray, crypto_sign_SECRETKEYBYTES);

return std::pair<std::string, std::string>(publicKey, secretKey);
}

/**
This function converts a string to Z85 format.
@param inputString: The string to convert
@return: The converted string

@throw: This function can throw exceptions
*/
std::string pylongps::convertStringToZ85Format(const std::string &inputString)
{
int z85StringSize = (((inputString.size()*5)/4)+1)+(((inputString.size()*5)/4))%4;

//Allocate character array to use on stack
std::vector<char> Z85Characters(z85StringSize); 

if(zmq_z85_encode(Z85Characters.data(), (unsigned char*) inputString.c_str(), inputString.size()) == nullptr)
{
throw SOMException("Could not encode to Z85\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

return std::string((char *) Z85Characters.data(), Z85Characters.size());
}

/**
This function converts a string from Z85 format.
@param inputZ85String: The string to convert (must be null terminated)
@return: The converted string

@throw: This function can throw exceptions
*/
std::string pylongps::convertStringFromZ85Format(const std::string &inputZ85String)
{
long int binaryStringSize = (inputZ85String.size()-1)*4/5;
std::vector<char> binaryCharacters(binaryStringSize);

//Convert back to binary
if(zmq_z85_decode((unsigned char *) binaryCharacters.data(), (char *)  inputZ85String.c_str()) == nullptr)
{
throw SOMException("Could not decode from Z85\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

return std::string(binaryCharacters.data(), binaryCharacters.size());
}

/**
This function saves a string to the given file path
@param inputString: The string to save
@param inputPath: The path for the file to save to
@return: true if the string was written successfully and false otherwise
*/
bool pylongps::saveStringToFile(const std::string &inputString, const std::string &inputPath)
{
auto fileFreeingLambda = [](FILE *inputFile){fclose(inputFile);};

FILE *buffer = fopen(inputPath.c_str(), "wb");
if(buffer == nullptr)
{
return false;
}
std::unique_ptr<FILE, decltype(fileFreeingLambda)> file(buffer, fileFreeingLambda);

if(fwrite(inputString.c_str(), inputString.size(), 1, file.get()) != 1)
{
return false;
}

return true;
}

/**
This function saves a string to the given file path
@param inputStringBuffer: The string to save the read data to
@param inputSizeOfStringToRead: The path for the file to save to
@param inputPath: The path for the file to save to
@return: true if the string was read successfully and false otherwise
*/
bool pylongps::readStringFromFile(std::string &inputStringBuffer, unsigned int inputSizeOfStringToRead, const std::string &inputPath)
{
auto fileFreeingLambda = [](FILE *inputFile){fclose(inputFile);};

FILE *buffer = fopen(inputPath.c_str(), "rb");
if(buffer == nullptr)
{
return false;
}
std::unique_ptr<FILE, decltype(fileFreeingLambda)> file(buffer, fileFreeingLambda);

std::vector<char> characterArray(inputSizeOfStringToRead);

if(fread(characterArray.data(), 1, inputSizeOfStringToRead, file.get()) != inputSizeOfStringToRead)
{
return false;
}

inputStringBuffer = std::string(characterArray.data(), characterArray.size());

return true;
}

/**
This function loads a public key and decodes it from a Z85 encoded file.
@param inputPath: The path to load the key from
@return: a string of length crypto_sign_PUBLICKEYBYTES if successful and empty otherwise
*/
std::string pylongps::loadPublicKeyFromFile(const std::string &inputPath)
{
std::string z85Key;
if(readStringFromFile(z85Key, (unsigned int) (z85PublicKeySize-1), inputPath) == false)
{
return "";
}
//Add terminating null character
z85Key[z85PublicKeySize-1] = '\0';

unsigned char publicKeyBuffer[crypto_sign_PUBLICKEYBYTES];

//Convert back to binary
if(zmq_z85_decode(publicKeyBuffer, (char *) z85Key.c_str()) == nullptr)
{
return "";
}

return std::string((char *) publicKeyBuffer, crypto_sign_PUBLICKEYBYTES);
}

/**
This function loads a secret key and decodes it from a Z85 encoded file.
@param inputPath: The path to load the key from
@return: a string of length crypto_sign_SECRETKEYBYTES if successful and empty otherwise
*/
std::string pylongps::loadSecretKeyFromFile(const std::string &inputPath)
{
std::string z85Key;
if(readStringFromFile(z85Key, (unsigned int) (z85SecretKeySize-1), inputPath) == false)
{
return "";
}
//Add terminating null character
z85Key[z85SecretKeySize-1] = '\0';

unsigned char secretKeyBuffer[crypto_sign_SECRETKEYBYTES];

//Convert back to binary
if(zmq_z85_decode(secretKeyBuffer, (char *) z85Key.c_str()) == nullptr)
{
return "";
}

return std::string((char *) secretKeyBuffer, crypto_sign_SECRETKEYBYTES);
}

/**
This function loads a protobuf object from a file using the assumption that it is proceeded with a 64 bit Poco::Int64 in network byte order that holds the size of the object to be loaded and deserialized.
@param inputPath: The path to the file to load from
@param inputMessageBuffer: The protobuf object to deserialize to
@return: true if the message could be loaded and deserialized and false otherwise
*/
bool pylongps::loadProtobufObjectFromFile(const std::string &inputPath, google::protobuf::Message &inputMessageBuffer)
{
//Read size of object in file
std::string sizeString;
if(readStringFromFile(sizeString, sizeof(Poco::Int64), inputPath) == false) 
{
return false;
}

Poco::Int64 objectSizeNetworkOrder = *((Poco::Int64 *) sizeString.c_str());
Poco::Int64 objectSize = Poco::ByteOrder::fromNetwork(objectSizeNetworkOrder);

//Read size+object
std::string sizeAndObjectString;
if(readStringFromFile(sizeAndObjectString, sizeof(Poco::Int64)+objectSize, inputPath) == false || objectSize == 0) 
{
return false;
}

//Clip off size
std::string objectString = sizeAndObjectString.substr(sizeof(Poco::Int64));

//Deserialize the protobuf object
inputMessageBuffer.ParseFromString(objectString);

if(!inputMessageBuffer.IsInitialized())
{
return false;
}

return true;
}

/**
This function saves a protobuf object to a file with a proceeded 64 bit Poco::Int64 in network byte order that holds the size of the object to be loaded and deserialized.
@param inputPath: The path to the file to save the object to
@param inputMessageBuffer: The protobuf object to serialize
@return: true if the message could be serialized/saved and false otherwise
*/
bool pylongps::saveProtobufObjectToFile(const std::string &inputPath, google::protobuf::Message &inputMessageBuffer)
{
std::string serializedObject;
try
{
inputMessageBuffer.SerializeToString(&serializedObject);
}
catch(const std::exception &inputException)
{
return false;
}

//Create serialized object size
Poco::Int64 objectSizeNetworkOrder = Poco::ByteOrder::toNetwork(Poco::Int64(serializedObject.size()));

//Save to file
if(!saveStringToFile(std::string((char *) &objectSizeNetworkOrder, sizeof(Poco::Int64)) + serializedObject, inputPath))
{
return false;
}

return true;
}

/**
This function parses the passed strings and looks for arguments starting with "-".  If the next argument after a "-" argument doesn't start with "-", it stores the pair <"optionWithout-Character", "associatedValue">.  If it does start with "-", it simply stores <"optionWithout-Character", ""> and processes the next argument.
@param inputArguments: The set of strings to process
@param inputNumberOfArguments: The number of strings to process
@return: a map of form "option" -> value
*/
std::map<std::string, std::string> pylongps::parseStringArguments(char **inputArguments, unsigned int inputNumberOfArguments)
{
//Convert arguments to strings
std::vector<std::string> arguments;
for(int i=0; i<inputNumberOfArguments; i++)
{
arguments.push_back(std::string(inputArguments[i]));
}

std::map<std::string, std::string> results;
for(int i=0; i<arguments.size(); i++)
{
if(arguments[i].find("-") == 0)
{
if((i+1) < arguments.size())
{//Check if next argument isn't an option
if(arguments[i+1].find("-") != 0)
{
results[arguments[i].substr(1)] = arguments[i+1];
continue;
}
}

results[arguments[i].substr(1)] = "";
}
}

return results;
}

/**
This function converts a string to an integer without having bad defaults or needing to throw exceptions.
@param inputString: The string to convert
@param inputIntegerBuffer: The integer variable to store the result in
@return: True if the conversion was successful and false otherwise
*/
bool pylongps::convertStringToInteger(const std::string &inputString, long int &inputIntegerBuffer)
{
char *stringEnd = nullptr;
inputIntegerBuffer = strtol(inputString.c_str(), &stringEnd, 0);

if(((const char *) stringEnd) == inputString.c_str())
{ //Couldn't convert correctly
return false;
}

return true;
}

/**
This function converts a string to an integer without having bad defaults or needing to throw exceptions.
@param inputString: The string to convert
@param inputIntegerBuffer: The integer variable to store the result in
@return: True if the conversion was successful and false otherwise
*/
bool pylongps::convertStringToInteger(const std::string &inputString, int &inputIntegerBuffer)
{
char *stringEnd = nullptr;
inputIntegerBuffer = strtol(inputString.c_str(), &stringEnd, 0);

if(((const char *) stringEnd) == inputString.c_str())
{ //Couldn't convert correctly
return false;
}

return true;
}
