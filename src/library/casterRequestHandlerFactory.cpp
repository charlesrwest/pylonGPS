#include "casterRequestHandlerFactory.hpp"

/*
This function is called each time the HTTP server receives a request.  It generates a different handler object to handle the depending on characteristics found in the given request variable.
@param inputRequest: The request to construct the handler for

@exceptions: This function can throw exceptions
*/
Poco::Net::HTTPRequestHandler * casterRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &request)
{

//if(request.getURI() == "/")
//{
return new RootHandler();  //Only one handler right now
//}
//else
//{
//return new DataHandler();
//}
}
