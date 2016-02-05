#include "utilityFunctions.hpp"

using namespace pylongps;

const double pylongps::PI = 3.14159265358979323846;

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
if(readStringFromFile(sizeAndObjectString, sizeof(Poco::Int64)+objectSize, inputPath) == false) 
{
return false;
}

//Clip off size
std::string objectString = sizeAndObjectString.substr(sizeof(Poco::Int64));

//Deserialize the protobuf object
inputMessageBuffer.ParseFromString(objectString);

if(!inputMessageBuffer.IsInitialized())
{
printf("Message not initialized\n");
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
bool pylongps::convertStringToInteger(const std::string &inputString, int64_t &inputIntegerBuffer)
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
bool pylongps::convertStringToInteger(const std::string &inputString, int32_t &inputIntegerBuffer)
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
This function converts a string to an double without having bad defaults or needing to throw exceptions.
@param inputString: The string to convert
@param inputDoubleBuffer: The double variable to store the result in
@return: True if the conversion was successful and false otherwise
*/
bool pylongps::convertStringToDouble(const std::string &inputString, double &inputDoubleBuffer)
{
char *stringEnd = nullptr;
inputDoubleBuffer = strtod(inputString.c_str(), &stringEnd);

if(((const char *) stringEnd) == inputString.c_str())
{ //Couldn't convert correctly
return false;
}

return true;
}

/**
This function creates a sqlite statement, binds it with the given statement string and assigns it to the given unique_ptr.
@param inputStatement: The unique_ptr to assign statement ownership to
@param inputStatementString: The SQL string to construct the statement from
@param inputDatabaseConnection: The database connection to use

@throws: This function can throw exceptions
*/
void pylongps::prepareStatement(std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> &inputStatement, const std::string &inputStatementString, sqlite3 &inputDatabaseConnection)
{
//printf("PREPARING STATEMENT:\n%s\n", inputStatementString.c_str());

int returnValue = 0;
SOM_TRY
sqlite3_stmt *buffer;
returnValue = sqlite3_prepare_v2(&inputDatabaseConnection, inputStatementString.c_str(), inputStatementString.size(), &buffer, NULL);
inputStatement.reset(buffer);
SOM_CATCH("Error, unable to initialize SQLite prepared statement\n")

if(returnValue != SQLITE_OK)
{
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(sqlite3_extended_errcode(&inputDatabaseConnection)) + "\n");
throw SOMException(errorMessage.c_str(), INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
}

/**
This function takes a protobuf CppType and returns the string representing the type that it will be stored as the database.  Throws an exception if the type cannot be stored as a database primitive.
@param inputType: The type to convert
@return: The string for the database type

@throw: This function can throw exceptions
*/
std::string pylongps::cppTypeToDatabaseTypeString(google::protobuf::FieldDescriptor::CppType inputType)
{
switch(inputType)
{
case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
return "INTEGER";
break;

case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
return "REAL";
break;

case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
return "BLOB";
break;

default:
throw SOMException("Unrecognized data type\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
break;
}

}

/**
This function binds the given value to a SQLite statement.
@param inputStatement: The statement to bind the field value to
@param inputSQLStatementIndex: The index of the statement field to bind
@param inputValue: The value to bind

@throw: This function can throw exceptions
*/
void pylongps::bindFieldValueToStatement(sqlite3_stmt &inputStatement, uint32_t inputSQLStatementIndex, int64_t inputValue)
{
int returnValue = 0;
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, inputValue);

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error occurred (" + std::to_string(returnValue)+ ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

/**
This function binds the given value to a SQLite statement.
@param inputStatement: The statement to bind the field value to
@param inputSQLStatementIndex: The index of the statement field to bind
@param inputValue: The value to bind

@throw: This function can throw exceptions
*/
void pylongps::bindFieldValueToStatement(sqlite3_stmt &inputStatement, uint32_t inputSQLStatementIndex, double inputValue)
{
int returnValue = 0;
returnValue = sqlite3_bind_double(&inputStatement, inputSQLStatementIndex, inputValue);

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error occurred (" + std::to_string(returnValue)+ ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

/**
This function binds the given value to a SQLite statement.
@param inputStatement: The statement to bind the field value to
@param inputSQLStatementIndex: The index of the statement field to bind
@param inputValue: The value to bind

@throw: This function can throw exceptions
*/
void pylongps::bindFieldValueToStatement(sqlite3_stmt &inputStatement, uint32_t inputSQLStatementIndex, const std::string &inputValue)
{
int returnValue = 0;
returnValue = sqlite3_bind_blob64(&inputStatement, inputSQLStatementIndex, (const void*) inputValue.c_str(), inputValue.size(), SQLITE_TRANSIENT);

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error occurred (" + std::to_string(returnValue)+ ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

/**
This function binds NULL to a SQLite statement field.
@param inputStatement: The statement to bind the field value to
@param inputSQLStatementIndex: The index of the statement field to bind
@param inputValue: The value to bind

@throw: This function can throw exceptions
*/
void pylongps::bindFieldValueToStatement(sqlite3_stmt &inputStatement, uint32_t inputSQLStatementIndex)
{
int returnValue = 0;
returnValue = sqlite3_bind_null(&inputStatement, inputSQLStatementIndex);

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error occurred (" + std::to_string(returnValue)+ ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

/**
This function binds the value of a protobuf field to a SQLite statement according to the type of the field (can only be a primative type).
@param inputStatement: The statement to bind the field value to
@param inputSQLStatementIndex: The index of the statement field to bind
@param inputMessage: The message to get the value from
@param inputField: The field (from the message) to get the value from

@throw: This function can throw exceptions
*/
void pylongps::bindFieldValueToStatement(sqlite3_stmt &inputStatement, uint32_t inputSQLStatementIndex, const google::protobuf::Message &inputMessage, const google::protobuf::FieldDescriptor *inputField)
{
const google::protobuf::Reflection *messageReflection = inputMessage.GetReflection();

int returnValue = 0;

if(messageReflection->HasField(inputMessage, inputField) == true)
{
switch(inputField->cpp_type())
{
case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetInt32(inputMessage, inputField));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetInt64(inputMessage, inputField));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetUInt32(inputMessage, inputField));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetUInt64(inputMessage, inputField));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
returnValue = sqlite3_bind_double(&inputStatement, inputSQLStatementIndex, messageReflection->GetDouble(inputMessage, inputField));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
returnValue = sqlite3_bind_double(&inputStatement, inputSQLStatementIndex, messageReflection->GetDouble(inputMessage, inputField));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetBool(inputMessage, inputField));
break;

case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetEnum(inputMessage, inputField)->number());
break;

case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
{
std::string buffer;
buffer = messageReflection->GetStringReference(inputMessage, inputField, &buffer);
returnValue = sqlite3_bind_blob64(&inputStatement, inputSQLStatementIndex, (const void*) buffer.c_str(), buffer.size(), SQLITE_TRANSIENT);
}
break;

default:
throw SOMException("Unrecognized data type\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
break;
}
}
else
{ //Its a optional field that is not set, so store a NULL value
returnValue = sqlite3_bind_null(&inputStatement, inputSQLStatementIndex);
}

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error occurred (" + std::to_string(returnValue)+ ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

/**
This function binds the value of a protobuf field to a SQLite statement according to the type of the field (can only be a primative type).
@param inputStatement: The statement to bind the field value to
@param inputSQLStatementIndex: The index of the statement field to bind
@param inputMessage: The message to get the value from
@param inputField: The repeated field (from the message) to get the value from
@param inputIndex: The index of the value in the repeated field

@throw: This function can throw exceptions
*/
void pylongps::bindRepeatedFieldValueToStatement(sqlite3_stmt &inputStatement, uint32_t inputSQLStatementIndex, const google::protobuf::Message &inputMessage, const google::protobuf::FieldDescriptor *inputField, unsigned int inputIndex)
{
const google::protobuf::Reflection *messageReflection = inputMessage.GetReflection();

int returnValue = 0;

switch(inputField->cpp_type())
{
case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetRepeatedInt32(inputMessage, inputField, inputIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetRepeatedInt64(inputMessage, inputField, inputIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetRepeatedUInt32(inputMessage, inputField, inputIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetRepeatedUInt64(inputMessage, inputField, inputIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
returnValue = sqlite3_bind_double(&inputStatement, inputSQLStatementIndex, messageReflection->GetRepeatedDouble(inputMessage, inputField, inputIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
returnValue = sqlite3_bind_double(&inputStatement, inputSQLStatementIndex, messageReflection->GetRepeatedDouble(inputMessage, inputField, inputIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetRepeatedBool(inputMessage, inputField, inputIndex));
break;

case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
returnValue = sqlite3_bind_int64(&inputStatement, inputSQLStatementIndex, messageReflection->GetRepeatedEnum(inputMessage, inputField, inputIndex)->number());
break;

case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
{
std::string buffer;
buffer = messageReflection->GetRepeatedStringReference(inputMessage, inputField, inputIndex, &buffer);
returnValue = sqlite3_bind_blob64(&inputStatement, inputSQLStatementIndex, (const void*) buffer.c_str(), buffer.size(), SQLITE_TRANSIENT);
}
break;

default:
throw SOMException("Unrecognized data type\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
break;
}

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error occurred (" + std::to_string(returnValue)+ ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

/**
This function retrieves the the value of a protobuf field from a SQLite statement according to the type of the field (can only be a primative type).
@param inputStatement: The statement to retrieve the field value from (should already be stepped)
@param inputSQLStatementIndex: The index of the statement field to retrieve from
@param inputMessage: The message to store the value to
@param inputField: The field (from the message) to store the value in

@throw: This function can throw exceptions
*/
void pylongps::retrieveFieldValueFromStatement(sqlite3_stmt &inputStatement, uint32_t inputSQLStatementIndex, google::protobuf::Message &inputMessage, const google::protobuf::FieldDescriptor *inputField)
{
const google::protobuf::Reflection *messageReflection = inputMessage.GetReflection();

if(sqlite3_column_type(&inputStatement, inputSQLStatementIndex) != SQLITE_NULL)
{
switch(inputField->cpp_type())
{
case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
messageReflection->SetInt32(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
messageReflection->SetInt64(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
messageReflection->SetUInt32(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
messageReflection->SetUInt64(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
messageReflection->SetDouble(&inputMessage, inputField, sqlite3_column_double(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
messageReflection->SetFloat(&inputMessage, inputField, sqlite3_column_double(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
messageReflection->SetBool(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;

case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
messageReflection->SetEnum(&inputMessage, inputField, inputField->enum_type()->FindValueByNumber(sqlite3_column_int64(&inputStatement, inputSQLStatementIndex)));
break;

case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
{
int stringSize = sqlite3_column_bytes16(&inputStatement, inputSQLStatementIndex);
const void *stringData = sqlite3_column_blob(&inputStatement, inputSQLStatementIndex);
messageReflection->SetString(&inputMessage, inputField, std::string((const char *)stringData, stringSize));
}
break;

default:
throw SOMException("Unrecognized data type " + std::to_string(inputField->cpp_type()) + "\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
break;
}
}
else
{ //It was NULL in the database, so clear it
messageReflection->ClearField(&inputMessage, inputField);
}

}

/**
This function retrieves the the value of a protobuf field from a SQLite statement according to the type of the field (can only be a primative type).
@param inputStatement: The statement to retrieve the field value from (should already be stepped)
@param inputSQLStatementIndex: The index of the statement field to retrieve from
@param inputMessage: The message to store the value to
@param inputField: The field (from the message) to store the value in

@throw: This function can throw exceptions
*/
void pylongps::retrieveRepeatedFieldValueFromStatement(sqlite3_stmt &inputStatement, uint32_t inputSQLStatementIndex, google::protobuf::Message &inputMessage, const google::protobuf::FieldDescriptor *inputField)
{
const google::protobuf::Reflection *messageReflection = inputMessage.GetReflection();

switch(inputField->cpp_type())
{
case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
messageReflection->AddInt32(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
messageReflection->AddInt64(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
messageReflection->AddUInt32(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
messageReflection->AddUInt64(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
messageReflection->AddDouble(&inputMessage, inputField, sqlite3_column_double(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
messageReflection->AddFloat(&inputMessage, inputField, sqlite3_column_double(&inputStatement, inputSQLStatementIndex));
break;
case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
messageReflection->AddBool(&inputMessage, inputField, sqlite3_column_int64(&inputStatement, inputSQLStatementIndex));
break;

case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
messageReflection->AddEnum(&inputMessage, inputField, inputField->enum_type()->FindValueByNumber(sqlite3_column_int64(&inputStatement, inputSQLStatementIndex)));
break;

case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
{
int stringSize = sqlite3_column_bytes16(&inputStatement, inputSQLStatementIndex);
const void *stringData = sqlite3_column_blob(&inputStatement, inputSQLStatementIndex);
messageReflection->AddString(&inputMessage, inputField, std::string((const char *)stringData, stringSize));
}
break;

default:
throw SOMException("Unrecognized data type\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
break;
}

}

/**
This function steps a SQLite statement and then resets it so that it can be used again.
@param inputStatement: The statement to step/reset

@throw: This function can throw exceptions
*/
void pylongps::stepAndResetSQLiteStatement(sqlite3_stmt &inputStatement)
{
int returnValue = sqlite3_step(&inputStatement);
sqlite3_reset(&inputStatement);
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statement\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

/**
This function gets the (assumed to be set) integer field's value from the message and casts it to a int64_t value.
@param inputMessage: The message to retrieve the integer from
@param inputField: The specific field to retrieve it from

@throw: This function can throw exceptions
*/
int64_t pylongps::getIntegerFieldValue(const google::protobuf::Message &inputMessage, const google::protobuf::FieldDescriptor *inputField)
{
const google::protobuf::Reflection *messageReflection = inputMessage.GetReflection();

int64_t value = 0;

switch(inputField->cpp_type())
{
case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
value = messageReflection->GetInt32(inputMessage, inputField);
break;
case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
value = messageReflection->GetInt64(inputMessage, inputField);
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
value = messageReflection->GetUInt32(inputMessage, inputField);
break;
case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
value = messageReflection->GetUInt64(inputMessage, inputField);
break;
case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
value = messageReflection->GetEnum(inputMessage, inputField)->number();
break;

default:
throw SOMException("Non-integer data type\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
break;
}

return value;
}

/**
This function attempts to retrieve a Json value from a URL via an http request.  If the request/parsing is successful, the retrieved Json value is stored in the given buffer.  This function is not particularly efficient, so it might be worth considering streamlining the application if that is a concern.
@param inputURL: The URL to retrieve the Json object from
@param inputValueBuffer: The object to store the retrieved value in
@param inputTimeoutDurationInMicroseconds: How long to wait for the value to be return in microseconds
@return: True if successful and false otherwise
*/
bool pylongps::getJsonValueFromURL(const std::string &inputURL, Json::Value &inputValueBuffer, int64_t inputTimeoutDurationInMicroseconds)
{

try
{
Poco::Timestamp startTime;
Poco::Timestamp timeoutTimepoint = startTime+((Poco::Timestamp::TimeDiff) inputTimeoutDurationInMicroseconds);

//Parse URI
Poco::URI url(inputURL);

Poco::Net::HostEntry host;
host = Poco::Net::DNS::hostByName(url.getHost());

if(host.addresses().size() == 0)
{//No IP address for host, so exit silently
return false;
}


Poco::Net::StreamSocket connectionSocket;

connectionSocket.connect(Poco::Net::SocketAddress(host.addresses()[0], 80));

connectionSocket.setReceiveTimeout(timeoutTimepoint-Poco::Timestamp());

Poco::Net::HTTPRequest getRequest("GET", url.getPathAndQuery(), "HTTP/1.1");
getRequest.setHost(url.getHost());

std::stringstream serializedHeader;
getRequest.write(serializedHeader);

connectionSocket.sendBytes(serializedHeader.str().c_str(), serializedHeader.str().size());

std::array<char, 1024> receiveBuffer;
int amountOfDataReceived = 0;
std::string receivedData;
while(Poco::Timestamp() < timeoutTimepoint)
{
amountOfDataReceived = connectionSocket.receiveBytes(receiveBuffer.data(), receiveBuffer.size());

if(amountOfDataReceived == 0)
{
break;
}

receivedData += std::string(receiveBuffer.data(), amountOfDataReceived);

if(receivedData.find("}") != std::string::npos)
{
break;
}
}

if(receivedData.find("{") == std::string::npos)
{
return false;
}

receivedData = receivedData.substr(receivedData.find("{"));

Json::Reader reader;

if(reader.parse(receivedData, inputValueBuffer) != true)
{
return false; //Couldn't parse JSON
}

return true;
}
catch(const std::exception &inputException)
{
return false;
}

}

/**
This function calculates the land based distance between two points on earth using the great circle model.
@param inputPoint1Latitude: The latitude of the first point
@param inputPoint1Longitude: The longitude of the first point
@param inputPoint2Latitude: The latitude of the second point
@param inputPoint2Longitude: The longitude of the second point
@return: The estimated distance between the two points (centimeters)
*/
double pylongps::calculateGreatCircleDistance(double inputPoint1Latitude, double inputPoint1Longitude, double inputPoint2Latitude, double inputPoint2Longitude)
{
double arc1 = pow(sin(.5*(inputPoint1Latitude-inputPoint2Latitude)), 2.0);
double arc2 = pow(sin(.5*(inputPoint1Longitude-inputPoint2Longitude)), 2.0);
return 2.0*6372797.560856*asin(sqrt(arc1+cos(inputPoint1Latitude)*cos(inputPoint2Latitude)*arc2));
}

/**
This function returns the first IP address it can find for a given URL.
@param inputURL: The URL to resolve
@return: The found IP address

@throws: This function can throw exceptions
*/
std::string pylongps::getURLIPAddress(const std::string &inputURL)
{
Poco::Net::HostEntry host;

SOM_TRY
host = Poco::Net::DNS::hostByName(inputURL);
SOM_CATCH("Error, unable to resolve URL\n")

if(host.addresses().size() == 0)
{
throw SOMException("Error resolving URL", UNKNOWN, __FILE__, __LINE__);
}
return host.addresses()[0].toString();
}

