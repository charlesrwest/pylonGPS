#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <iostream>
#include <string>
#include<sstream>

int main(int argc, char **argv)
{
try
{
//Create URI
Poco::URI uri("http://hive.geosystems.aero:9091/ITMO");
std::cout << "Target Host: " << uri.getHost() << std::endl;
std::cout << "Target Path: " << uri.getPath() << std::endl;
std::cout << "Target Port: " << uri.getPort() << std::endl;
std::cout << "Target Query: " << uri.getQuery() << std::endl;
std::cout << std::endl;

uri.setPath("ITMO");
std::cout << "Target Host: " << uri.getHost() << std::endl;
std::cout << "(modified) Target Path: " << uri.getPath() << std::endl;
std::cout << "Target Port: " << uri.getPort() << std::endl;
std::cout << "Target Query: " << uri.getQuery() << std::endl;
std::cout << "The whole URI: " << uri.toString() << std::endl;

//Create session
Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());


//Make request
Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.getPath(), Poco::Net::HTTPMessage::HTTP_1_0);
request.setCredentials("Basic", "charlie");
request.add("User-Agent", "NTRIP"); //Add required HTTP fields
request.add("Accept", "*/*");
request.add("Connection", "close");

//Write the request to stdout to see what is being sent
request.write(std::cout);

session.sendRequest(request);



//Process response
Poco::Net::HTTPResponse response;

//Write what we got to stdout
std::cout << "Getting response" << std::endl;
session.receiveResponse(response);


response.read(std::cin);
std::cout << "Really" << std::endl;

std::cout << std::cin.rdbuf() <<  std::endl;
std::cout << "Got response" << std::endl;

//See how the response object interpreted it
std::cout << "Response status: " << response.getStatus() << std::endl;
std::cout << "Response status reason: " << response.getReason() << std::endl;


}
catch (std::exception &inputException)
{
std::cerr << inputException.what() << std::endl;
return -1;
}

return 0;
}
