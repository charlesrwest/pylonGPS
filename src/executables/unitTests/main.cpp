#define CATCH_CONFIG_MAIN //Make main function automatically
#include "catch.hpp"
#include "NMEAGGASentence.hpp"
#include "streamSourceTableEntry.hpp"
#include "casterSourceTableEntry.hpp"
#include "networkSourceTableEntry.hpp"
#include "sourceRegistrationRequestHeader.hpp"
#include "sourceManager.hpp"
#include<chrono>
#include<Poco/Net/TCPServer.h>
#include<Poco/Util/ServerApplication.h>
#include<Poco/Net/TCPServerConnectionFactory.h>
#include "casterTCPServerConnectionFactoryImplementation.hpp"
#include "sourceManager.hpp"
#include "serverTCPServerConnectionFactoryImplementation.hpp"
#include "serverTCPConnectionHandler.hpp"

using namespace pylongps; //Use pylongps classes without alteration for now

TEST_CASE( "NMEA GGA Sentences are parsed", "[NMEAGGASentence]")
{
NMEAGGASentence testSentence;

SECTION( "Example1 Parse")
{
REQUIRE( testSentence.parse("$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F")  == true);
SECTION( "Testing values" )
{
REQUIRE( testSentence.talkerID == "GP");
REQUIRE( testSentence.hours == 17);
REQUIRE( testSentence.minutes == 28);
REQUIRE( testSentence.seconds == Approx(14.0));
REQUIRE( testSentence.latitude == Approx(37.39109795));
REQUIRE( testSentence.longitude == Approx(-122.0378263));
REQUIRE( testSentence.qualityType == DIFFERENTIAL_GPS_FIX);
REQUIRE( testSentence.numberOfSatellitesInUse == 6);
REQUIRE( testSentence.horizontalDilutionOfPrecision == Approx(1.2));
REQUIRE( testSentence.antennaAltitude == Approx(18.893));
REQUIRE( testSentence.geoidalSeparation == Approx(-25.669));
REQUIRE( testSentence.differentialUpdateAge == Approx(2.0));
REQUIRE( testSentence.differentialBasestationID == std::string("0031"));
}

SECTION("Serialization to Parse Test")
{
std::string serializedString = testSentence.serialize();
NMEAGGASentence testSentence2;

REQUIRE(testSentence2.parse(serializedString) == true);

SECTION( "Testing values" )
{
REQUIRE( testSentence2.talkerID == "GP");
REQUIRE( testSentence2.hours == 17);
REQUIRE( testSentence2.minutes == 28);
REQUIRE( testSentence2.seconds == Approx(14.0));
REQUIRE( testSentence2.latitude == Approx(37.39109795));
REQUIRE( testSentence2.longitude == Approx(-122.0378263));
REQUIRE( testSentence2.qualityType == DIFFERENTIAL_GPS_FIX);
REQUIRE( testSentence2.numberOfSatellitesInUse == 6);
REQUIRE( testSentence2.horizontalDilutionOfPrecision == Approx(1.2));
REQUIRE( testSentence2.antennaAltitude == Approx(18.893));
REQUIRE( testSentence2.geoidalSeparation == Approx(-25.669));
REQUIRE( testSentence2.differentialUpdateAge == Approx(2.0));
REQUIRE( testSentence2.differentialBasestationID == std::string("0031"));
}
}
}
}

TEST_CASE( "streamSourceTableEntries are parsed", "[streamSourceTableEntry]")
{
streamSourceTableEntry entry;
SECTION( "Example1 Parse")
{
std::string example1 = "STR;TEST_BINEX;TEST;ZERO;binex(1);2;GPS;BARD;USA;37.8743;237.7403;0;0;TPS NET-G3A;None;B;N;5000;none\r\n";
int numberOfCharactersUsed = entry.parse(example1);

REQUIRE(numberOfCharactersUsed == example1.size());

//Test members to make sure it parsed currectly
REQUIRE(entry.type == std::string("STR"));
REQUIRE(entry.mountpoint == std::string("TEST_BINEX"));
REQUIRE(entry.identifier == std::string("TEST"));
REQUIRE(entry.format == std::string("ZERO"));
REQUIRE(entry.formatDetails == std::string("binex(1)"));
REQUIRE(entry.carrier == L1_AND_L2_PHASE_INFO);
REQUIRE(entry.navigationSystems == std::string("GPS"));
REQUIRE(entry.network == std::string("BARD"));
REQUIRE(entry.countryCode == std::string("USA"));
REQUIRE(entry.latitude == Approx(37.8743));
REQUIRE(entry.longitude == Approx(237.7403));
REQUIRE(entry.NMEARequired == false); 
REQUIRE(entry.solution == SINGLE_BASESTATION);
REQUIRE(entry.generator == std::string("TPS NET-G3A"));
REQUIRE(entry.compressionMethod == std::string("None"));
REQUIRE(entry.authentication == BASIC);
REQUIRE(entry.fee == FREE);
REQUIRE(entry.bitRate == 5000);
REQUIRE(entry.misc == std::string("none"));

//Serialize and then parse
SECTION( "Example1 serialization/reparse")
{
std::string serializedVersion = entry.serialize();
streamSourceTableEntry entry1;

REQUIRE(entry1.parse(serializedVersion) == serializedVersion.size());

//Test members to make sure it parsed currectly
REQUIRE(entry1.type == std::string("STR"));
REQUIRE(entry1.mountpoint == std::string("TEST_BINEX"));
REQUIRE(entry1.identifier == std::string("TEST"));
REQUIRE(entry1.format == std::string("ZERO"));
REQUIRE(entry1.formatDetails == std::string("binex(1)"));
REQUIRE(entry1.carrier == L1_AND_L2_PHASE_INFO);
REQUIRE(entry1.navigationSystems == std::string("GPS"));
REQUIRE(entry1.network == std::string("BARD"));
REQUIRE(entry1.countryCode == std::string("USA"));
REQUIRE(entry1.latitude == Approx(37.8743));
REQUIRE(entry1.longitude == Approx(237.7403));
REQUIRE(entry1.NMEARequired == false); 
REQUIRE(entry1.solution == SINGLE_BASESTATION);
REQUIRE(entry1.generator == std::string("TPS NET-G3A"));
REQUIRE(entry1.compressionMethod == std::string("None"));
REQUIRE(entry1.authentication == BASIC);
REQUIRE(entry1.fee == FREE);
REQUIRE(entry1.bitRate == 5000);
REQUIRE(entry1.misc == std::string("none"));
}
}

}

TEST_CASE( "casterSourceTableEntry are parsed", "[casterSourceTableEntry]")
{
casterSourceTableEntry entry;
SECTION( "Example1 Parse")
{
std::string example1 = "CAS;tiburon.geo.berkeley.edu;2101;RTGPS;UCB;0;USA;37.87;237.74;0.0.0.0;0; Production\r\n";
int numberOfCharactersUsed = entry.parse(example1);

REQUIRE(numberOfCharactersUsed == example1.size());

REQUIRE(entry.type == "CAS");
REQUIRE(entry.host == "tiburon.geo.berkeley.edu");
REQUIRE(entry.port == 2101);
REQUIRE(entry.identifier == "RTGPS");
REQUIRE(entry.operatorID == "UCB");
REQUIRE(entry.nmeaClientsSupported == false);
REQUIRE(entry.country == "USA");
REQUIRE(entry.latitude == Approx(37.87));
REQUIRE(entry.longitude == Approx(237.74));
REQUIRE(entry.misc == "0.0.0.0;0; Production");

//Serialize and then parse
SECTION( "Example1 serialization/reparse")
{
std::string serializedVersion = entry.serialize();
casterSourceTableEntry entry1;

REQUIRE(entry1.parse(serializedVersion) == serializedVersion.size());

REQUIRE(entry1.type == "CAS");
REQUIRE(entry1.host == "tiburon.geo.berkeley.edu");
REQUIRE(entry1.port == 2101);
REQUIRE(entry1.identifier == "RTGPS");
REQUIRE(entry1.operatorID == "UCB");
REQUIRE(entry1.nmeaClientsSupported == false);
REQUIRE(entry1.country == "USA");
REQUIRE(entry1.latitude == Approx(37.87));
REQUIRE(entry1.longitude == Approx(237.74));
REQUIRE(entry1.misc == "0.0.0.0;0; Production");
}
}
}

TEST_CASE( "networkSourceTableEntry are parsed", "[networkSourceTableEntry]")
{
networkSourceTableEntry entry;
SECTION( "Example1 Parse")
{
std::string example1 = "NET;PBO;PBO_SIO;B;N;http://www.unavco.org;none;gps@seismo.berkeley.edu;none\r\n";
int numberOfCharactersUsed = entry.parse(example1);

REQUIRE(numberOfCharactersUsed == example1.size());

REQUIRE(entry.type == "NET");
REQUIRE(entry.identifier == "PBO");
REQUIRE(entry.operatorID == "PBO_SIO");
REQUIRE(entry.authenticationMethod == BASIC);
REQUIRE(entry.fee == FREE);
REQUIRE(entry.networkInformationWebAddress == "http://www.unavco.org");
REQUIRE(entry.streamInformationWebAddress == "none");
REQUIRE(entry.webOrMailAddressForRegistration == "gps@seismo.berkeley.edu");
REQUIRE(entry.misc == "none");

//Serialize and then parse
SECTION( "Example1 serialization/reparse")
{
std::string serializedVersion = entry.serialize();
networkSourceTableEntry entry1;

REQUIRE(entry1.parse(serializedVersion) == serializedVersion.size());

REQUIRE(entry1.type == "NET");
REQUIRE(entry1.identifier == "PBO");
REQUIRE(entry1.operatorID == "PBO_SIO");
REQUIRE(entry1.authenticationMethod == BASIC);
REQUIRE(entry1.fee == FREE);
REQUIRE(entry1.networkInformationWebAddress == "http://www.unavco.org");
REQUIRE(entry1.streamInformationWebAddress == "none");
REQUIRE(entry1.webOrMailAddressForRegistration == "gps@seismo.berkeley.edu");
REQUIRE(entry1.misc == "none");
}
}

}

TEST_CASE( "SOURCE registration request headers are parsed", "[sourceRegistrationRequestHeader]")
{
sourceRegistrationRequestHeader header;

SECTION( "Example1 Parse")
{
std::string example1 = "SOURCE password mountpoint\r\nSource-Agent: NTRIP product comment\r\ntest: field\r\n\r\n";
int numberOfCharactersUsed = header.parse(example1);

REQUIRE(numberOfCharactersUsed == example1.size());
REQUIRE(header.password == "password");
REQUIRE(header.mountpoint == "mountpoint");
REQUIRE(header.productComment == "product comment");
REQUIRE(header.extraFields.size() == 1);
REQUIRE(header.extraFields[0].first == "test");
REQUIRE(header.extraFields[0].second == "field");

//Serialize and then parse
SECTION( "Example1 serialization/reparse")
{
std::string serializedVersion = header.serialize();
sourceRegistrationRequestHeader header1;

REQUIRE(header1.parse(serializedVersion) == serializedVersion.size());
REQUIRE(header1.password == "password");
REQUIRE(header1.mountpoint == "mountpoint");
REQUIRE(header1.productComment == "product comment");
REQUIRE(header1.extraFields.size() == 1);
REQUIRE(header1.extraFields[0].first == "test");
REQUIRE(header1.extraFields[0].second == "field");

}

}
}


TEST_CASE( "sourceManager is created/destroyed and sent requests", "[sourceManager]")
{
zmq::context_t context;

SECTION("Creating/destroying 2 sourceManagers at the same time")
{
{
sourceManager mySourceManager(&context);
sourceManager mySourceManager1(&context);
}//SourceManager should be destroyed after this block
REQUIRE(true == true);
}

SECTION("Test server metadata registration")
{
sourceManager mySourceManager(&context);
std::string sourceMountpoint = "NCStateBasestation";

//Attempt to register source metadata
//Make socket
std::unique_ptr<zmq::socket_t> metaDataRequestSocket;
SOM_TRY
metaDataRequestSocket.reset(new zmq::socket_t(context, ZMQ_REQ));
SOM_CATCH("Error initializing metadata request socket\n")

//Connect to sourceManager tcp port
SOM_TRY
metaDataRequestSocket->connect(("tcp://127.0.0.1:" + std::to_string(mySourceManager.serverMetadataAdditionSocketPortNumber)).c_str());
SOM_CATCH("Error connecting to socket\n")

//Create request message
std::string serializedMetadataRegistrationRequest;
pylongps::ntrip_server_metadata_addition_request metadataRegistrationRequest;

metadataRegistrationRequest.set_mountpoint(sourceMountpoint);
metadataRegistrationRequest.set_server_source_table_entry("STR;NCStateBasestation;CAND;ZERO;0;2;GPS;PBO;USA;35.93935;239.56631;0;0;TRIMBLE NETRS;none;B;N;5000;none\r\n");

//Serialize request
metadataRegistrationRequest.SerializeToString(&serializedMetadataRegistrationRequest);

//Send request to register metadata for source (and get generated password)
SOM_TRY
metaDataRequestSocket->send(serializedMetadataRegistrationRequest.c_str(), serializedMetadataRegistrationRequest.size());
SOM_CATCH("Error sending metadata registration request message\n")

//Sleep to give the server time to process
std::this_thread::sleep_for(std::chrono::milliseconds(100));

std::unique_ptr<zmq::message_t> messageBuffer0;

SOM_TRY
messageBuffer0.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

REQUIRE(metaDataRequestSocket->recv(messageBuffer0.get(), ZMQ_DONTWAIT) == true);  //Server should have sent response

pylongps::ntrip_server_metadata_addition_reply metadataReply;

metadataReply.ParseFromArray(messageBuffer0->data(), messageBuffer0->size());
REQUIRE(metadataReply.IsInitialized() == true);

//Check if metadata request succeeded
REQUIRE(metadataReply.has_request_failure_reason() == false);
REQUIRE(metadataReply.has_password() == true);
REQUIRE(metadataReply.has_mountpoint() == true);
REQUIRE(metadataReply.password().size() != 0);
REQUIRE(metadataReply.mountpoint() == sourceMountpoint);

SECTION("Test server source registration")
{
//Connect to source manager to send source registration message
std::unique_ptr<zmq::socket_t> socket;
SOM_TRY
socket.reset(new zmq::socket_t(context, ZMQ_REQ));
SOM_CATCH("Error initializing test inproc socket\n")

SOM_TRY
socket->connect(mySourceManager.serverRegistrationDeregistrationSocketConnectionString.c_str());
SOM_CATCH("Error connecting to socket\n")

std::string sourceConnectionString = std::string("inproc://InfoSource.")+sourceMountpoint;

ntrip_server_registration_or_deregistraton_request request;
std::string serializedRequest;
request.set_mountpoint(sourceMountpoint);
request.set_registering(true);
request.set_password(metadataReply.password());
request.set_connection_address(sourceConnectionString);
request.SerializeToString(&serializedRequest);


SOM_TRY
socket->send(serializedRequest.c_str(), serializedRequest.size());
SOM_CATCH("Error sending request message\n")

//Sleep to give the server time to process
std::this_thread::sleep_for(std::chrono::milliseconds(100));

std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

REQUIRE(socket->recv(messageBuffer.get(), ZMQ_DONTWAIT) == true);  //Server should have sent response

ntrip_server_registration_or_deregistraton_reply reply;
reply.ParseFromArray(messageBuffer->data(), messageBuffer->size());
REQUIRE(reply.IsInitialized() == true);

REQUIRE(reply.request_succeeded() == true);

SECTION("Test if registered value can be retrieved")
{
//Connect to source manager
std::unique_ptr<zmq::socket_t> socket1;
SOM_TRY
socket1.reset(new zmq::socket_t(context, ZMQ_REQ));
SOM_CATCH("Error initializing test inproc socket\n")

SOM_TRY
socket1->connect(mySourceManager.sourceTableAccessSocketConnectionString.c_str());
SOM_CATCH("Error connecting to socket\n")

ntrip_source_table_request request1;
request1.set_mountpoint(sourceMountpoint);

std::string serializedRequest1;
request1.SerializeToString(&serializedRequest1);

SOM_TRY
socket1->send(serializedRequest1.c_str(), serializedRequest1.size());
SOM_CATCH("Error sending request message\n")

//Sleep to give the server time to process
std::this_thread::sleep_for(std::chrono::milliseconds(100));

std::unique_ptr<zmq::message_t> messageBuffer1;

SOM_TRY
messageBuffer1.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

REQUIRE(socket1->recv(messageBuffer1.get(), ZMQ_DONTWAIT) == true);  //Server should have sent response

ntrip_source_table_reply reply1;
reply1.ParseFromArray(messageBuffer1->data(), messageBuffer1->size());
REQUIRE(reply1.IsInitialized() == true);
REQUIRE(reply1.has_stream_inproc_address() == true);
REQUIRE(reply1.has_source_table() == false);
REQUIRE(reply1.stream_inproc_address() == sourceConnectionString);

}
}
}

}

TEST_CASE( "ntripServer is created and has test connections", "[sourceManager]")
{
//Create ZMQ context
std::unique_ptr<zmq::context_t> context;
SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing ZMQ context\n")

//Create server for later tests
Poco::UInt16 port = 9998;
Poco::Net::TCPServerParams *serverParams = new Poco::Net::TCPServerParams; //TCPServer takes ownership
serverParams->setMaxQueued(10000);
serverParams->setMaxThreads(10000);

//Create source manager
std::unique_ptr<pylongps::sourceManager> manager;
SOM_TRY
manager.reset(new pylongps::sourceManager(context.get(), port+1));
SOM_CATCH("Error initializing source manager\n")

Poco::Net::ServerSocket serverSocket(port); //Create a server socket
auto variable = new casterTCPServerConnectionFactoryImplementation(context.get(), manager->serverRegistrationDeregistrationSocketConnectionString, manager->mountpointDisconnectSocketConnectionString, manager->sourceTableAccessSocketConnectionString, manager->serverMetadataAdditionSocketPortNumber);
Poco::Net::TCPServer tcpServer(variable, serverSocket, serverParams);

//Start the NTRIP HTTPServer
tcpServer.start();


//Attempt to register source metadata
std::string sourceMountpoint = "NCStateBasestation";

//Attempt to register source metadata
//Make socket
std::unique_ptr<zmq::socket_t> metaDataRequestSocket;
SOM_TRY
metaDataRequestSocket.reset(new zmq::socket_t(*context, ZMQ_REQ));
SOM_CATCH("Error initializing metadata request socket\n")

//Connect to sourceManager tcp port
SOM_TRY
metaDataRequestSocket->connect(("tcp://127.0.0.1:" + std::to_string(manager->serverMetadataAdditionSocketPortNumber)).c_str());
SOM_CATCH("Error connecting to socket\n")

//Create request message
std::string serializedMetadataRegistrationRequest;
pylongps::ntrip_server_metadata_addition_request metadataRegistrationRequest;

metadataRegistrationRequest.set_mountpoint(sourceMountpoint);
metadataRegistrationRequest.set_server_source_table_entry("STR;NCStateBasestation;CAND;ZERO;0;2;GPS;PBO;USA;35.93935;239.56631;0;0;TRIMBLE NETRS;none;B;N;5000;none\r\n");

//Serialize request
metadataRegistrationRequest.SerializeToString(&serializedMetadataRegistrationRequest);

//Send request to register metadata for source (and get generated password)
SOM_TRY
metaDataRequestSocket->send(serializedMetadataRegistrationRequest.c_str(), serializedMetadataRegistrationRequest.size());
SOM_CATCH("Error sending metadata registration request message\n")

//Sleep to give the server time to process
std::this_thread::sleep_for(std::chrono::milliseconds(100));

std::unique_ptr<zmq::message_t> messageBuffer0;

SOM_TRY
messageBuffer0.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

REQUIRE(metaDataRequestSocket->recv(messageBuffer0.get(), ZMQ_DONTWAIT) == true);  //Server should have sent response

pylongps::ntrip_server_metadata_addition_reply metadataReply;

metadataReply.ParseFromArray(messageBuffer0->data(), messageBuffer0->size());
REQUIRE(metadataReply.IsInitialized() == true);

//Check if metadata request succeeded
REQUIRE(metadataReply.has_request_failure_reason() == false);
REQUIRE(metadataReply.has_password() == true);
REQUIRE(metadataReply.has_mountpoint() == true);
REQUIRE(metadataReply.password().size() != 0);
REQUIRE(metadataReply.mountpoint() == sourceMountpoint);

//Attempt to register source via NTRIP call
std::string sourceHeaderMessage;
sourceHeaderMessage += "SOURCE " + metadataReply.password() + " " + sourceMountpoint + "\r\n";
sourceHeaderMessage += "Source-Agent: NTRIP product|comment\r\n\r\n";

//Create socket and connect to server
std::unique_ptr<Poco::Net::StreamSocket> sourceSocket;
SOM_TRY
 sourceSocket.reset(new Poco::Net::StreamSocket(Poco::Net::SocketAddress("127.0.0.1:" + std::to_string(port))));
SOM_CATCH("Error initializing POCO socket\n")

REQUIRE(sourceSocket->sendBytes(sourceHeaderMessage.c_str(), sourceHeaderMessage.size()));

//Get response from server
std::string receivedReply;
int bufferSize = 512;
char buffer[512];
int numberOfBytesReceived = 0;
sourceSocket->setReceiveTimeout(Poco::Timespan(5,0)); //Max 5 second wait
for(int i=0; i<5; i++)//Up to 5 message received
{
SOM_TRY
try
{
numberOfBytesReceived = sourceSocket->receiveBytes(buffer, bufferSize);
}
catch(const Poco::TimeoutException &inputException)
{//Don't do anything
}
SOM_CATCH("Error getting reply from server\n")

receivedReply+= std::string(buffer, numberOfBytesReceived);
if(receivedReply.find("\r\n\r\n") != std::string::npos)
{
break;
}
}

REQUIRE(receivedReply.find("\r\n\r\n") != std::string::npos);
REQUIRE(receivedReply.find("ICY 200 OK\r\n\r\n") == 0); 

SECTION("Test retrieving source table\n")
{
int bufferSize0 = 512;
char buffer0[512];

//Create client which gets a source list
std::unique_ptr<Poco::Net::StreamSocket> clientSocket;
SOM_TRY
clientSocket.reset(new Poco::Net::StreamSocket(Poco::Net::SocketAddress("127.0.0.1:" + std::to_string(port))));
SOM_CATCH("Error initializing POCO socket\n")

//Make ntrip request
std::string clientRequest0;
clientRequest0 += "GET btty HTTP/1.0\r\n";
clientRequest0 += "User-Agent: NTRIP product|comment\r\n";
clientRequest0 += "Accept: */*\r\n";
clientRequest0 += "Connection: close\r\n\r\n";

REQUIRE(clientSocket->sendBytes(clientRequest0.c_str(), clientRequest0.size()) == clientRequest0.size());

std::string receivedReply;
numberOfBytesReceived = 0;
clientSocket->setReceiveTimeout(Poco::Timespan(5,0)); //Max 5 second wait
for(int i=0; i<5; i++)//Up to 5 message received
{
SOM_TRY
try
{
numberOfBytesReceived = clientSocket->receiveBytes(buffer0, bufferSize0);
}
catch(const Poco::TimeoutException &inputException)
{//Don't do anything
}
SOM_CATCH("Error getting reply from server\n")


receivedReply+= std::string(buffer0, numberOfBytesReceived);
if(receivedReply.find("\r\n\r\n") != std::string::npos)
{
break;
}
}

REQUIRE(receivedReply.find("SOURCETABLE 200 OK") == 0);
REQUIRE(receivedReply.find("STR;NCStateBasestation;CAND;ZERO;0;2;GPS;PBO;USA;35.939350;239.566310;0;0;TRIMBLE NETRS;none;B;N;5000;none") != std::string::npos);
}

SECTION("Test retrieving source table\n")
{
int bufferSize0 = 512;
char buffer0[512];

//Create client which gets a source list
std::unique_ptr<Poco::Net::StreamSocket> clientSocket;
SOM_TRY
clientSocket.reset(new Poco::Net::StreamSocket(Poco::Net::SocketAddress("127.0.0.1:" + std::to_string(port))));
SOM_CATCH("Error initializing POCO socket\n")

//Make ntrip request
std::string clientRequest0;
clientRequest0 += "GET " + sourceMountpoint + " HTTP/1.0\r\n";
clientRequest0 += "User-Agent: NTRIP product|comment\r\n";
clientRequest0 += "Accept: */*\r\n";
clientRequest0 += "Connection: close\r\n\r\n";

REQUIRE(clientSocket->sendBytes(clientRequest0.c_str(), clientRequest0.size()) == clientRequest0.size());

numberOfBytesReceived = 0;
clientSocket->setReceiveTimeout(Poco::Timespan(5,0)); //Max 5 second wait
for(int i=0; i<5; i++)//Up to 5 message received
{
SOM_TRY
try
{
numberOfBytesReceived = clientSocket->receiveBytes(buffer0, bufferSize0);
}
catch(const Poco::TimeoutException &inputException)
{//Don't do anything
}
SOM_CATCH("Error getting reply from server\n")

std::string receivedReply;
receivedReply+= std::string(buffer0, numberOfBytesReceived);
if(receivedReply.find("\r\n\r\n") != std::string::npos)
{
break;
}
}

REQUIRE(receivedReply.find("ICY 200 OK") == 0);

SECTION("Test source is broadcasted to client")
{
std::this_thread::sleep_for(std::chrono::milliseconds(10));  //Wait for things to settle down so messages aren't missed
std::string testString = "GPS CORRECTIONS\n";
//Send a message via the source connection and see if it is received on the client connection
REQUIRE(sourceSocket->sendBytes(testString.c_str(), testString.size()));

int numberOfBytesReceived1 = 0;
char buffer1[512];
int bufferSize1 = 512;
std::string receivedReply1;
clientSocket->setReceiveTimeout(Poco::Timespan(5,0)); //Max 5 second wait
for(int i=0; i<5; i++)//Up to 5 message received
{
SOM_TRY
try
{
numberOfBytesReceived1 = clientSocket->receiveBytes(buffer1, bufferSize1);
}
catch(const Poco::TimeoutException &inputException)
{//Don't do anything
}
SOM_CATCH("Error getting reply from server\n")

receivedReply1 += std::string(buffer1, numberOfBytesReceived1);
if(receivedReply1.find(testString) != std::string::npos)
{
break;
}
}

REQUIRE(receivedReply1 == testString);

}


}


tcpServer.stop();
}


TEST_CASE( "ntripServer is created and ntripSourceIsCreated", "[casterAndServer]")
{
//Create ZMQ context
std::unique_ptr<zmq::context_t> context;
SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing ZMQ context\n")

//Create server for later tests
Poco::UInt16 port = 9998;
Poco::Net::TCPServerParams *serverParams = new Poco::Net::TCPServerParams; //TCPServer takes ownership
serverParams->setMaxQueued(10000);
serverParams->setMaxThreads(10000);

//Create source manager
std::unique_ptr<pylongps::sourceManager> manager;
SOM_TRY
manager.reset(new pylongps::sourceManager(context.get(), port+1));
SOM_CATCH("Error initializing source manager\n")

Poco::Net::ServerSocket serverSocket(port); //Create a server socket
auto variable = new casterTCPServerConnectionFactoryImplementation(context.get(), manager->serverRegistrationDeregistrationSocketConnectionString, manager->mountpointDisconnectSocketConnectionString, manager->sourceTableAccessSocketConnectionString, manager->serverMetadataAdditionSocketPortNumber);
Poco::Net::TCPServer tcpServer(variable, serverSocket, serverParams);

//Start the NTRIP HTTPServer
tcpServer.start();

//Create source server and have it register
std::unique_ptr<Poco::URI> casterAddress;
SOM_TRY
std::string URIString = "ntrip://127.0.0.1:" + std::to_string(port);
casterAddress.reset(new Poco::URI(URIString));
SOM_CATCH("Error creating URI\n")

std::string sourceMountpoint = "NCStateBasestation0";
std::string metadataString = "STR;"+sourceMountpoint+";CAND;ZERO;0;2;GPS;PBO;USA;35.93935;239.56631;0;0;TRIMBLE NETRS;none;B;N;5000;none";
streamSourceTableEntry metadata;

REQUIRE(metadata.parse(metadataString, false) == metadataString.size());


Poco::Net::TCPServerParams *serverParams0 = new Poco::Net::TCPServerParams; //TCPServer takes ownership
serverParams0->setMaxThreads(1); //One connection at a time
serverParams0->setMaxQueued(1);
Poco::UInt16 port0 = 10010;

Poco::Net::ServerSocket serverSocket0(port0); //Create a server socket
auto variable0 = new serverTCPServerConnectionFactoryImplementation(*casterAddress, metadata);
Poco::Net::TCPServer tcpServer0(variable0, serverSocket0, serverParams0);

//Start the source server
tcpServer0.start();

SECTION("Test source is broadcasted to client")
{//Unit test output easier with bit of sleep
std::this_thread::sleep_for(std::chrono::milliseconds(100));
//Connect source to source server
std::unique_ptr<Poco::Net::StreamSocket> sourceSocket;
SOM_TRY
sourceSocket.reset(new Poco::Net::StreamSocket(Poco::Net::SocketAddress("127.0.0.1:" + std::to_string(port0))));
SOM_CATCH("Error initializing POCO socket\n")

int bufferSize0 = 512;
char buffer0[512];

//Create client which gets a source list
std::unique_ptr<Poco::Net::StreamSocket> clientSocket;
SOM_TRY
clientSocket.reset(new Poco::Net::StreamSocket(Poco::Net::SocketAddress("127.0.0.1:" + std::to_string(port))));
SOM_CATCH("Error initializing POCO socket\n")

std::string testString = "GPS CORRECTIONS\n";
//Send a message via the source connection and see if it is received on the client connection
std::this_thread::sleep_for(std::chrono::milliseconds(100));

//Make ntrip request
std::string clientRequest0;
clientRequest0 += "GET " + sourceMountpoint + " HTTP/1.0\r\n";
//clientRequest0 += "GET btty HTTP/1.0\r\n";
clientRequest0 += "User-Agent: NTRIP product|comment\r\n";
clientRequest0 += "Accept: */*\r\n";
clientRequest0 += "Connection: close\r\n\r\n";


REQUIRE(clientSocket->sendBytes(clientRequest0.c_str(), clientRequest0.size()) == clientRequest0.size());

int numberOfBytesReceived = 0;
std::string receivedReply5;
clientSocket->setReceiveTimeout(Poco::Timespan(5,0)); //Max 5 second wait
for(int i=0; i<5; i++)//Up to 5 message received
{
SOM_TRY
try
{
numberOfBytesReceived = clientSocket->receiveBytes(buffer0, bufferSize0);
}
catch(const Poco::TimeoutException &inputException)
{//Don't do anything
}
SOM_CATCH("Error getting reply from server\n")

receivedReply5+= std::string(buffer0, numberOfBytesReceived);
if(receivedReply5.find("\r\n\r\n") != std::string::npos)
{
break;
}
}

REQUIRE(receivedReply5.find("ICY 200 OK") == 0);

//Send data and see if it is broadcasted
REQUIRE(sourceSocket->sendBytes(testString.c_str(), testString.size()) == testString.size()); 

int numberOfBytesReceived1 = 0;
char buffer1[512];
int bufferSize1 = 512;
std::string receivedReply1;
clientSocket->setReceiveTimeout(Poco::Timespan(5,0)); //Max 5 second wait
for(int i=0; i<5; i++)//Up to 5 message received
{
SOM_TRY
try
{
numberOfBytesReceived1 = clientSocket->receiveBytes(buffer1, bufferSize1);
}
catch(const Poco::TimeoutException &inputException)
{//Don't do anything
}
SOM_CATCH("Error getting reply from server\n")

receivedReply1 += std::string(buffer1, numberOfBytesReceived1);
if(receivedReply1.find(testString) != std::string::npos)
{
break;
}
}

REQUIRE(receivedReply1 == testString);

}

tcpServer.stop();
tcpServer0.stop();
}






