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



//Create session
Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());


//Make request
Poco::Net::HTTPRequest request(std::string("SOURCE"), uri.getPath(), Poco::Net::HTTPMessage::HTTP_1_0);
//Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_PUT, uri.getPath(), Poco::Net::HTTPMessage::HTTP_1_0);
//request.setCredentials("Basic", "ngat01:nextgen"); 
//request.setCredentials("Basic", "bmdhdDAxOm5leHRnZW4=");
//request.add("User-Agent", "NTRIP"); //Add required HTTP fields
//request.add("Accept", "*/*");
//request.add("Connection", "close");

//Write the request to stdout to see what is being sent
request.write(std::cout);

std::ostream &requestBody = session.sendRequest(request);

//Generate NMEA line for virtual reference station to generate corrections for

//Write nmea line as request body
//requestBody << "$GPGGA,195728.40,3546.32873,N,07840.42472,W,2,09,0.87,115.2,M,-34.4,M,,0000*63\r\n";

std::ifstream ifs("./README");
Poco::StreamCopier::copyStream(ifs, requestBody);

requestBody.flush();

//Process response
Poco::Net::HTTPResponse response;

//Write what we got to stdout
std::cout << "Getting response" << std::endl;
//session.receiveResponse(response);

std::istream& is = session.receiveResponse(response);

//See how the response object interpreted it
std::cout << "Response status: " << response.getStatus() << std::endl;
std::cout << "Response status reason: " << response.getReason() << std::endl;

// Print to standard output
//std::copy(std::istream_iterator<char>(is),
//        std::istream_iterator<char>(),
//        std::ostream_iterator<char>(std::cout));

Poco::StreamCopier::copyStream(is, std::cout);



}
catch (std::exception &inputException)
{
std::cerr << inputException.what() << std::endl;
return -1;
}

return 0;
}
