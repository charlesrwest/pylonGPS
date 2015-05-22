#ifndef  ROOTHANDLERHPP
#define ROOTHANDLERHPP

#include<Poco/Net/HTTPRequestHandler.h>
#include<Poco/Net/HTTPServerRequest.h>
#include<Poco/Net/HTTPServerResponse.h>
#include<Poco/Util/Application.h>
#include<Poco/Util/ServerApplication.h>
#include<Poco/StreamCopier.h>
#include<iostream>
#include<algorithm>
#include<iterator>

class RootHandler : public Poco::Net::HTTPRequestHandler
{
public:
void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
Poco::Util::Application &app = Poco::Util::Application::instance();
app.logger().information("Request from " + request.clientAddress().toString());

std::cout << "Content length: "<< request.getContentLength() << std::endl;

std::istream &requestBody = request.stream();

Poco::StreamCopier::copyStream(requestBody, std::cout);

// Print to standard output
//std::copy(std::istream_iterator<char>(requestBody),
//        std::istream_iterator<char>(),
//        std::ostream_iterator<char>(std::cout));

response.setChunkedTransferEncoding(true);
response.setContentType("text/html");

std::cout << "Got a request!\n";




std::ostream& ostr = response.send();



ostr << "<html><head><title>HTTP Server powered by POCO C++ Libraries</title></head>";
ostr << "<body>";
ostr << "Ghost Busters!";
ostr << "</body></html>";

std::cout << "Sent response\n";
}
};








#endif
