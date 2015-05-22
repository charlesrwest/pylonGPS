#ifndef  CASTERREQUESTHANDLERFACTORYHPP
#define CASTERREQUESTHANDLERFACTORYHPP

#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "RootHandler.hpp"

class casterRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
/*
This function is called each time the HTTP server receives a request.  It generates a different handler object to handle the depending on characteristics found in the given request variable.
@param inputRequest: The request to construct the handler for

@exceptions: This function can throw exceptions
*/
Poco::Net::HTTPRequestHandler * createRequestHandler(const Poco::Net::HTTPServerRequest &request);

};






#endif 
