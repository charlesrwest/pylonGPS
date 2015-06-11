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

