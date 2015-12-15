#include "Poco/ByteOrder.h"
#include<chrono>
#include<Poco/Net/DNS.h>

#include "transceiver.hpp"
#include "SOMException.hpp"

using namespace pylongps;

//Construct a pair of dummy basestations that send regular updates to the pylongps.com caster (port 10001)
const int REGISTRATION_PORT = 10001;
const int CLIENT_REQUEST_PORT = 10002;
const int CLIENT_PUBLISHING_PORT = 10003;
const int CASTER_ID = 0;

//host.addresses()[0].toString()


void REQUIRE(bool inputCondition)
{
throw SOMException("Required condition not met\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
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
std::unique_ptr<zmq::socket_t> testMessagePublisher;

SOM_TRY //Init socket
testMessagePublisher.reset(new zmq::socket_t(*context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

std::string ZMQPubSocketAddressString = "tcp://*:9031";

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
pubDataReceiverAddress = com->createZMQPubDataReceiver("127.0.0.1:9031");
SOM_CATCH("Error, unable to create data receiver\n")


//Caster data sender
std::string senderURI;
SOM_TRY 
senderURI = com->createPylonGPSV2DataSender(pubDataReceiverAddress, "127.0.0.1:" +std::to_string(REGISTRATION_PORT), 1.0, 2.0, RTCM_V3_1, "testBasestation", 3.0);
SOM_CATCH("Error making caster sender\n")

std::this_thread::sleep_for(std::chrono::milliseconds(10));


//Send a query and see if we get a valid response
client_query_request queryRequest; //Empty request should return all
client_query_reply queryReply;

SOM_TRY
queryReply = transceiver::queryPylonGPSV2Caster(queryRequest, "127.0.0.1:" + std::to_string(CLIENT_REQUEST_PORT), 5000, *context);
SOM_CATCH("Error querying caster\n")


REQUIRE(queryReply.IsInitialized() == true);
REQUIRE(queryReply.has_caster_id() == true);
REQUIRE(queryReply.has_failure_reason() == false);
REQUIRE(queryReply.base_stations_size() == 1);

auto replyBaseStationInfo = queryReply.base_stations(0);
REQUIRE(replyBaseStationInfo.has_latitude());
REQUIRE(fabs(replyBaseStationInfo.latitude() - 1.0) < .001);
REQUIRE(replyBaseStationInfo.has_longitude());
REQUIRE(fabs(replyBaseStationInfo.longitude() - 2.0) < .001);
REQUIRE(replyBaseStationInfo.has_expected_update_rate());
REQUIRE(fabs(replyBaseStationInfo.expected_update_rate() - 3.0) < .001);
REQUIRE(replyBaseStationInfo.has_message_format());
REQUIRE(replyBaseStationInfo.message_format() == RTCM_V3_1);
REQUIRE(replyBaseStationInfo.has_informal_name());
REQUIRE(replyBaseStationInfo.informal_name() == "testBasestation");
REQUIRE(replyBaseStationInfo.has_base_station_id());

//Store station ID so it can be used for later checks
auto baseStationID = replyBaseStationInfo.base_station_id();

//Do it again with a more complex query

//Let the base station entry age a bit, so we can check uptime
std::this_thread::sleep_for(std::chrono::milliseconds(100));

client_query_request clientRequest0; //Empty request should return all
client_query_reply clientReply0;

client_subquery subQuery0; //Shouldn't return any results
subQuery0.add_acceptable_classes(OFFICIAL);

sql_double_condition doubleCondition;
sql_integer_condition integerCondition;
sql_string_condition stringCondition;

client_subquery subQuery1; //Should return 1
subQuery1.add_acceptable_classes(COMMUNITY);
subQuery1.add_acceptable_formats(RTCM_V3_1);

doubleCondition.set_value(0.9); 
doubleCondition.set_relation(GREATER_THAN); 
auto doubleConditionBuffer = subQuery1.add_latitude_condition();
(*doubleConditionBuffer) = doubleCondition;
doubleCondition.set_value(1.1); 
doubleCondition.set_relation(LESS_THAN);
doubleConditionBuffer = subQuery1.add_latitude_condition(); 
(*doubleConditionBuffer) = doubleCondition;



doubleCondition.set_value(1.9); 
doubleCondition.set_relation(GREATER_THAN_EQUAL_TO); 
doubleConditionBuffer = subQuery1.add_longitude_condition();
(*doubleConditionBuffer) = doubleCondition;
doubleCondition.set_value(2.1); 
doubleCondition.set_relation(LESS_THAN_EQUAL_TO); 
doubleConditionBuffer = subQuery1.add_longitude_condition();
(*doubleConditionBuffer) = doubleCondition;


doubleCondition.set_value(.01); //Up longer than .01 seconds
doubleCondition.set_relation(GREATER_THAN_EQUAL_TO); 
doubleConditionBuffer = subQuery1.add_uptime_condition();
(*doubleConditionBuffer) = doubleCondition;

doubleCondition.set_value(2.9); 
doubleCondition.set_relation(GREATER_THAN_EQUAL_TO); 
doubleConditionBuffer = subQuery1.add_expected_update_rate_condition();
(*doubleConditionBuffer) = doubleCondition;


stringCondition.set_value("testBasestat%");
stringCondition.set_relation(LIKE);
(*subQuery1.mutable_informal_name_condition()) = stringCondition;

integerCondition.set_value(baseStationID);
integerCondition.set_relation(EQUAL_TO);
auto integerConditionBuffer = subQuery1.add_base_station_id_condition();
(*integerConditionBuffer) = integerCondition;


base_station_radius_subquery circleSubQueryPart;
circleSubQueryPart.set_latitude(1.0);
circleSubQueryPart.set_longitude(2.0);
circleSubQueryPart.set_radius(10);
(*subQuery1.mutable_circular_search_region()) = circleSubQueryPart;

//Add subqueries to request
(*clientRequest0.add_subqueries()) = subQuery0;
(*clientRequest0.add_subqueries()) = subQuery1;

SOM_TRY
clientReply0 = transceiver::queryPylonGPSV2Caster(clientRequest0, "127.0.0.1:" + std::to_string(CLIENT_REQUEST_PORT), 5000, *context);
SOM_CATCH("Error querying caster\n")


REQUIRE(queryReply.IsInitialized() == true);
REQUIRE(queryReply.has_caster_id() == true);
REQUIRE(queryReply.has_failure_reason() == false);
REQUIRE(queryReply.base_stations_size() == 1);

replyBaseStationInfo = queryReply.base_stations(0);
REQUIRE(replyBaseStationInfo.has_latitude());
REQUIRE(fabs(replyBaseStationInfo.latitude() - 1.0) < .001);
REQUIRE(replyBaseStationInfo.has_longitude());
REQUIRE(fabs(replyBaseStationInfo.longitude() - 2.0) < .001);
REQUIRE(replyBaseStationInfo.has_expected_update_rate());
REQUIRE(fabs(replyBaseStationInfo.expected_update_rate() - 3.0) < .001);
REQUIRE(replyBaseStationInfo.has_message_format());
REQUIRE(replyBaseStationInfo.message_format() == RTCM_V3_1);
REQUIRE(replyBaseStationInfo.has_informal_name());
REQUIRE(replyBaseStationInfo.informal_name() == "testBasestation");

//Base station registered, so subscribe and see if we get the next message send

//Make transciever subscribe to our basestation in the caster and forward the data to a ZMQ PUB socket
std::string receiverConnectionString;

SOM_TRY
receiverConnectionString = com->createPylonGPSV2DataReceiver("127.0.0.1:"+std::to_string(CLIENT_PUBLISHING_PORT), CASTER_ID, baseStationID); 
SOM_CATCH("Error, unable to make receiver\n")

std::string zmqSenderID;
int zmqSenderPortNumber = 9020;

SOM_TRY
zmqSenderID = com->createZMQDataSender(receiverConnectionString, zmqSenderPortNumber);
SOM_CATCH("Error, unable to create zmq data sender\n")

//Create socket to subscribe to zmq data sender
std::unique_ptr<zmq::socket_t> subscriberSocket;

SOM_TRY //Init socket
subscriberSocket.reset(new zmq::socket_t(*context, ZMQ_SUB));
SOM_CATCH("Error making socket\n")

SOM_TRY
int timeoutWaitTime = 5000; //Max 5 seconds
subscriberSocket->setsockopt(ZMQ_RCVTIMEO, (void *) &timeoutWaitTime, sizeof(timeoutWaitTime));
SOM_CATCH("Error setting socket timeout\n")

SOM_TRY //Connect to data sender
std::string connectionString = "tcp://127.0.0.1:" +std::to_string(zmqSenderPortNumber);
subscriberSocket->connect(connectionString.c_str());
SOM_CATCH("Error connecting socket for registration with caster\n")

SOM_TRY //Set filter to allow any published messages to be received
subscriberSocket->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for socket\n")

//Sleep for a few milliseconds to allow connection to stabilize so no messages are missed
std::this_thread::sleep_for(std::chrono::milliseconds(10));

//Send message to caster and see if we get it back with the subscriber
std::string testString = "This is a test string\n";
SOM_TRY
testMessagePublisher->send(testString.c_str(), testString.size());
SOM_CATCH("Error sending message to caster\n")

std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

REQUIRE(subscriberSocket->recv(messageBuffer.get()) == true);

//Check message format
REQUIRE(messageBuffer->size() == testString.size());

REQUIRE(std::string((const char *) messageBuffer->data(), messageBuffer->size()) == testString);

printf("Tests completed successfully\n");

//Sleep forever while caster operates
while(true)
{
std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

return 0;
}


