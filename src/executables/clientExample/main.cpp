#include<Poco/Net/StreamSocket.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <iostream>
#include <string>
#include<sstream>
#include<fstream>
#include <iterator>
#include "NMEAGGASentence.hpp"


int main(int argc, char **argv)
{
NMEAGGASentence bob;


try
{
//Create URI NC77_Raw
//Poco::URI uri("http://rtn.nc.gov:2101/VRS_RTCM3");
//Poco::URI uri("http://rtn.nc.gov:2101/NC77_Raw");
Poco::URI uri("http://localhost:9998/VRS_RTCM3");
std::cout << "Target Host: " << uri.getHost() << std::endl;
std::cout << "Target Path: " << uri.getPath() << std::endl;
std::cout << "Target Port: " << uri.getPort() << std::endl;
std::cout << "Target Query: " << uri.getQuery() << std::endl;
std::cout << std::endl;

//Create TCP socket with active connection
Poco::Net::StreamSocket connection(Poco::Net::SocketAddress(uri.getHost(), uri.getPort()));

//Create session
//Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());


//Make request
Poco::Net::HTTPRequest request(std::string("SOURCE"), uri.getPath(), Poco::Net::HTTPMessage::HTTP_1_0);
//Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_PUT, uri.getPath(), Poco::Net::HTTPMessage::HTTP_1_0);
//request.setCredentials("Basic", "ngat01:nextgen"); 
request.setCredentials("Basic", "bmdhdDAxOm5leHRnZW4=");
request.add("User-Agent", "NTRIP"); //Add required HTTP fields
request.add("Accept", "*/*");
request.add("Connection", "close");

//Add request to stringstream
std::stringstream stringStream;
//Add body
std::ifstream ifs("./README");
Poco::StreamCopier::copyStream(ifs, stringStream);

//Convert request to string
request.write(stringStream);
std::string requestString = stringStream.str(); 

//Write request to TCP socket 
connection.sendBytes(requestString.c_str(), requestString.size());

//std::ostream &requestBody = session.sendRequest(request);

//Generate NMEA line for virtual reference station to generate corrections for

//Write nmea line as request body
//requestBody << "$GPGGA,195728.40,3546.32873,N,07840.42472,W,2,09,0.87,115.2,M,-34.4,M,,0000*63\r\n";



//requestBody.flush();

//Process response
//Poco::Net::HTTPResponse response;

//Write what we got to stdout
//std::cout << "Getting response" << std::endl;


//std::istream& is = session.receiveResponse(response);

//See how the response object interpreted it
//std::cout << "Response status: " << response.getStatus() << std::endl;
//std::cout << "Response status reason: " << response.getReason() << std::endl;

// Print to standard output
//Poco::StreamCopier::copyStream(is, std::cout);



}
catch (std::exception &inputException)
{
std::cerr << inputException.what() << std::endl;
return -1;
}

return 0;
}
