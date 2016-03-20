#include "Poco/ByteOrder.h"
#include<chrono>
#include<Poco/Net/DNS.h>

#include "transceiver.hpp"
#include "SOMException.hpp"

using namespace pylongps;

//Construct 100 dummy basestations with random lat/long
const int REGISTRATION_PORT = 10001;
const int CLIENT_REQUEST_PORT = 10002;
const int CLIENT_PUBLISHING_PORT = 10003;
const int CASTER_ID = 0;

const int NUMBER_OF_FAKE_BASESTATIONS_TO_MAKE = 1;

std::string updateString = "LARS PORSENA of Clusium,	  By the Nine Gods he swore	That the great house of Tarquin	  Should suffer wrong no more.	By the Nine Gods he swore it,	         And named a trysting-day,	And bade his messengers ride forth,	East and west and south and north,	  To summon his array.	East and west and south and north	        10  The messengers ride fast,	And tower and town and cottage	  Have heard the trumpet’s blast.	Shame on the false Etruscan	  Who lingers in his home,	        When Porsena of Clusium	  Is on the march for Rome!	 The horsemen and the footmen	  Are pouring in amain	From many a stately market-place,	          From many a fruitful plain,	From many a lonely hamlet,	  Which, hid by beech and pine,	Like an eagle’s nest hangs on the crest	  Of purple Apennine:	         From lordly Volaterræ,	  Where scowls the far-famed hold	Piled by the hands of giants	  For godlike kings of old;	From sea-girt Populonia,	          Whose sentinels descry	Sardinia’s snowy mountain-tops	  Fringing the southern sky;	From the pisæ";

void REQUIRE(bool inputCondition, const char *inputSourceFileName, int inputSourceLineNumber)
{
if(!inputCondition)
{
throw SOMException("Required condition not met\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, inputSourceFileName, inputSourceLineNumber);
}
}

int main(int argc, char** argv)
{
Poco::Net::HostEntry host;

SOM_TRY
host = Poco::Net::DNS::hostByName("www.pylongps.com");
SOM_CATCH("Error, unable to resolve URL\n")

printf("Accessing www.pylongps.com\n");
for(int i=0; i<host.addresses().size(); i++)
{
printf("IP Addresses: %s\n", host.addresses()[i].toString().c_str());
}

if(host.addresses().size() == 0)
{
fprintf(stderr,"Error, unable to resolve address\n");
}

std::unique_ptr<zmq::context_t> context;

SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error, unable to initialize caster\n")



//Register a unauthenticated stream
std::random_device generator;
std::uniform_int_distribution<int> portNumberGenerator(11000, 19000);
int portNumber = portNumberGenerator(generator);

std::unique_ptr<zmq::socket_t> testMessagePublisher;

SOM_TRY //Init socket
testMessagePublisher.reset(new zmq::socket_t(*context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

std::string ZMQPubSocketAddressString = "tcp://*:" + std::to_string(portNumber);

SOM_TRY
testMessagePublisher->bind(ZMQPubSocketAddressString.c_str());
SOM_CATCH("Error binding socket\n")

std::unique_ptr<transceiver> com;

SOM_TRY
com.reset(new transceiver(*context));
SOM_CATCH("Error initializing transceiver\n")


//ZMQ Pub receiver
std::string pubDataReceiverAddress;

SOM_TRY
pubDataReceiverAddress = com->createZMQPubDataReceiver("127.0.0.1:" + std::to_string(portNumber));
SOM_CATCH("Error, unable to create data receiver\n")


//Create caster data senders
std::uniform_real_distribution<double> latitudeGenerator(-90.0, 90.0);
std::uniform_real_distribution<double> longitudeGenerator(-180.0, 180.0);

int countOfRegisteredBasestations = 0;
for(int i=0; i<NUMBER_OF_FAKE_BASESTATIONS_TO_MAKE; i++)
{
try
{
SOM_TRY 
com->createPylonGPSV2DataSender(pubDataReceiverAddress,  host.addresses()[0].toString()+":" +std::to_string(REGISTRATION_PORT), latitudeGenerator(generator), longitudeGenerator(generator), RTCM_V3_1, "testBasestation" + std::to_string(portNumber) + std::to_string(i), 3.0);
SOM_CATCH("Error making caster sender\n")
std::this_thread::sleep_for(std::chrono::milliseconds(100));
SOM_TRY
testMessagePublisher->send(updateString.c_str(), updateString.size());
SOM_CATCH("Error sending message to caster\n")
countOfRegisteredBasestations++;
}
catch(const std::exception &inputException)
{
fprintf(stderr, "Registration failed at the %dth basestation\nError: %s\n", i, inputException.what());
}
}

printf("%d basestations have been registered\n", countOfRegisteredBasestations);



//Send a query and see if we get a valid response
client_query_request queryRequest; //Empty request should return all
client_query_reply queryReply;

SOM_TRY
queryReply = transceiver::queryPylonGPSV2Caster(queryRequest, host.addresses()[0].toString()+":" + std::to_string(CLIENT_REQUEST_PORT), 5000, *context);
SOM_CATCH("Error querying caster\n")



REQUIRE(queryReply.IsInitialized() == true, __FILE__, __LINE__);
REQUIRE(queryReply.has_caster_id() == true, __FILE__, __LINE__);
REQUIRE(queryReply.has_failure_reason() == false, __FILE__, __LINE__);


//Sleep forever while caster operates


while(true)
{
SOM_TRY
testMessagePublisher->send(updateString.c_str(), updateString.size());
SOM_CATCH("Error sending message to caster\n")
std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

return 0;
}


