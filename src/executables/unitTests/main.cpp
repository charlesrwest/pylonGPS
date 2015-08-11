#define CATCH_CONFIG_MAIN //Make main function automatically
#include "catch.hpp"
#include "caster.hpp"
#include "SOMException.hpp"
#include<memory>
#include<string>
#include "zmq.hpp"
#include "zmq_utils.h"
#include <cstdio>
#include<functional>
#include "protobufSQLConverter.hpp"
#include "protobuf_sql_converter_test_message.pb.h"



using namespace pylongps; //Use pylongps classes without alteration for now
using namespace pylongps_protobuf_sql_converter; //Use protobuf/sql converter test message


TEST_CASE( "Caster Initializes", "[test]")
{

SECTION( "Create and destroy caster")
{
//Make ZMQ context
std::unique_ptr<zmq::context_t> context;

SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing ZMQ context\n")

//Generate keys to use
char z85CasterPublicKey[41];
char z85CasterSecretKey[41];

REQUIRE(zmq_curve_keypair(z85CasterPublicKey, z85CasterSecretKey) == 0);

//Convert to binary format
char buffer[32];
std::string decodedCasterPublicKey;
std::string decodedCasterSecretKey;

zmq_z85_decode((uint8_t *) buffer, z85CasterPublicKey);
decodedCasterPublicKey = std::string(buffer, 32);

zmq_z85_decode((uint8_t *) buffer, z85CasterSecretKey);
decodedCasterSecretKey = std::string(buffer, 32);

caster myCaster(context.get(), 0,9001,9002,9003,9004, 9005, 9006, decodedCasterPublicKey, decodedCasterSecretKey);

REQUIRE( true == true);

}
}

class testClass
{
public:
testClass(int inputNumber)
{
number = inputNumber;
}

int getNumber()
{
return number;
}

void setNumber(int inputNumber)
{
number = inputNumber; 
}

int number;
};

template <class classType> class functionCaller
{
public:
void setFunctionToCall(const std::function<int(classType*)> inputFunctionPointer)
{
memberFunctionPointer = inputFunctionPointer;
}

void callFunction(classType &inputTestClass)
{
//printf("Called with value: %d\n", memberFunctionPointer(&inputTestClass));
}

std::function<int(classType*)> memberFunctionPointer;
};

TEST_CASE( "Test member function pointers", "[test]")
{

SECTION( "Call member function pointer")
{
//Create two objects
testClass test0(0);
testClass test1(9);

functionCaller<testClass> functionCaller;
functionCaller.setFunctionToCall(&testClass::getNumber);

//std::function<void(testClass*)> f_add_display = &testClass::getNumber;

functionCaller.callFunction(test0);
functionCaller.callFunction(test1);

}
}

TEST_CASE( "Test protobufSQLConverter", "[test]")
{

SECTION( "Set/get fields")
{
//Create database connection
sqlite3 *databaseConnection = nullptr;
REQUIRE(sqlite3_open_v2(":memory:", &databaseConnection, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,NULL) == SQLITE_OK);
//REQUIRE(sqlite3_open_v2("./testDatabase", &databaseConnection, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,NULL) == SQLITE_OK);

SOMScopeGuard databaseConnectionGuard([&]() {sqlite3_close_v2(databaseConnection);} );

//Make sure foreign key constractions are active for this connection
REQUIRE(sqlite3_exec(databaseConnection, "PRAGMA foreign_keys = on;", NULL, NULL, NULL) == SQLITE_OK);

//Initialize protobufSQLConverter
protobufSQLConverter<protobuf_sql_converter_test_message> testConverter(databaseConnection, "protobuf_sql_converter_test_message_table");

//Add REQUIRED fields
//int64 primary key
testConverter.addField(PYLON_GPS_GEN_REQUIRED_INT64_FIELD(protobuf_sql_converter_test_message, required_int64, "required_int64"), true); 
testConverter.addField(PYLON_GPS_GEN_REQUIRED_DOUBLE_FIELD(protobuf_sql_converter_test_message, required_double, "required_double" )); 
testConverter.addField(PYLON_GPS_GEN_REQUIRED_STRING_FIELD(protobuf_sql_converter_test_message, required_string, "required_string"));
testConverter.addField(PYLON_GPS_GEN_REQUIRED_ENUM_FIELD(protobuf_sql_converter_test_message, test_enum, required_enum, "required_enum"));



//Add OPTIONAL fields
testConverter.addField(PYLON_GPS_GEN_OPTIONAL_INT64_FIELD(protobuf_sql_converter_test_message, optional_int64, "optional_int64"));
testConverter.addField(PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(protobuf_sql_converter_test_message, optional_double, "optional_double" )); 
testConverter.addField(PYLON_GPS_GEN_OPTIONAL_STRING_FIELD(protobuf_sql_converter_test_message, optional_string, "optional_string"));
testConverter.addField(PYLON_GPS_GEN_OPTIONAL_ENUM_FIELD(protobuf_sql_converter_test_message, test_enum, optional_enum, "optional_enum"));


//These are left null when the message is made (not set)
testConverter.addField(PYLON_GPS_GEN_OPTIONAL_INT64_FIELD(protobuf_sql_converter_test_message, null_optional_int64, "null_optional_int64"));
testConverter.addField(PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(protobuf_sql_converter_test_message, null_optional_double, "null_optional_double" )); 
testConverter.addField(PYLON_GPS_GEN_OPTIONAL_STRING_FIELD(protobuf_sql_converter_test_message, null_optional_string, "null_optional_string"));
testConverter.addField(PYLON_GPS_GEN_OPTIONAL_ENUM_FIELD(protobuf_sql_converter_test_message, test_enum, null_optional_enum, "null_optional_enum"));


//Add REPEATED fields
testConverter.addField(PYLON_GPS_GEN_REPEATED_INT64_FIELD(protobuf_sql_converter_test_message, repeated_int64,  "repeated_int64_table", "repeated_int64", "key"));
testConverter.addField(PYLON_GPS_GEN_REPEATED_DOUBLE_FIELD(protobuf_sql_converter_test_message, repeated_double,  "repeated_double_table", "repeated_double", "key"));
testConverter.addField(PYLON_GPS_GEN_REPEATED_STRING_FIELD(protobuf_sql_converter_test_message, repeated_string,  "repeated_string_table", "repeated_string", "key"));
testConverter.addField(PYLON_GPS_GEN_REPEATED_ENUM_FIELD(protobuf_sql_converter_test_message, test_enum, repeated_enum,  "repeated_enum_table", "repeated_enum", "key"));

//Automatically generate tables
testConverter.createTables();

//Make message to test on
protobuf_sql_converter_test_message testMessage;
testMessage.set_required_int64(1);
testMessage.set_required_double(2.0);
testMessage.set_required_string("3");
testMessage.set_required_enum(TEST_REGISTERED_COMMUNITY);

testMessage.set_optional_int64(4);
testMessage.set_optional_double(5.0);
testMessage.set_optional_string("6");
testMessage.set_optional_enum(TEST_COMMUNITY);


testMessage.add_repeated_int64(7);
testMessage.add_repeated_int64(8);
testMessage.add_repeated_int64(9);
testMessage.add_repeated_double(10.0);
testMessage.add_repeated_double(11.0);
testMessage.add_repeated_double(12.0);
testMessage.add_repeated_string("13");
testMessage.add_repeated_string("14");
testMessage.add_repeated_string("15");
testMessage.add_repeated_enum(TEST_OFFICIAL);
testMessage.add_repeated_enum(TEST_REGISTERED_COMMUNITY);
testMessage.add_repeated_enum(TEST_COMMUNITY);




//Print out all of the fields we added
testConverter.print(testMessage);
//Attempt to write to the database
testConverter.store(testMessage);

std::vector<protobuf_sql_converter_test_message> retrievedValues;
std::vector<::google::protobuf::int64> keys = {1};

SOM_TRY
retrievedValues = testConverter.retrieve(keys);
SOM_CATCH("Error retrieving values")

REQUIRE(retrievedValues.size() == 1);

REQUIRE(retrievedValues[0].required_int64() == 1);
REQUIRE(retrievedValues[0].required_double() == Approx(2.0));
REQUIRE(retrievedValues[0].required_string() == "3");
REQUIRE(retrievedValues[0].required_enum() == TEST_REGISTERED_COMMUNITY);

REQUIRE(retrievedValues[0].has_optional_int64() == true);
REQUIRE(retrievedValues[0].has_optional_double() == true);
REQUIRE(retrievedValues[0].has_optional_string() == true);
REQUIRE(retrievedValues[0].has_optional_enum() == true);

REQUIRE(retrievedValues[0].has_null_optional_int64() == false);
REQUIRE(retrievedValues[0].has_null_optional_double() == false);
REQUIRE(retrievedValues[0].has_null_optional_string() == false);
REQUIRE(retrievedValues[0].has_null_optional_enum() == false);

REQUIRE(retrievedValues[0].optional_int64() == 4);
REQUIRE(retrievedValues[0].optional_double() == Approx(5.0));
REQUIRE(retrievedValues[0].optional_string() == "6");
REQUIRE(retrievedValues[0].optional_enum() == TEST_COMMUNITY);

REQUIRE(retrievedValues[0].repeated_int64_size() == 3);
REQUIRE(retrievedValues[0].repeated_int64(0) == 7);
REQUIRE(retrievedValues[0].repeated_int64(1) == 8);
REQUIRE(retrievedValues[0].repeated_int64(2) == 9);
REQUIRE(retrievedValues[0].repeated_double_size() == 3);
REQUIRE(retrievedValues[0].repeated_double(0) == Approx(10.0));
REQUIRE(retrievedValues[0].repeated_double(1) == Approx(11.0));
REQUIRE(retrievedValues[0].repeated_double(2) == Approx(12.0));
REQUIRE(retrievedValues[0].repeated_string_size() == 3);
REQUIRE(retrievedValues[0].repeated_string(0) == "13");
REQUIRE(retrievedValues[0].repeated_string(1) == "14");
REQUIRE(retrievedValues[0].repeated_string(2) == "15");
REQUIRE(retrievedValues[0].repeated_enum_size() == 3);
REQUIRE(retrievedValues[0].repeated_enum(0) == TEST_OFFICIAL);
REQUIRE(retrievedValues[0].repeated_enum(1) == TEST_REGISTERED_COMMUNITY);
REQUIRE(retrievedValues[0].repeated_enum(2) == TEST_COMMUNITY);

//Test updating a field in the database
testConverter.update(fieldValue((::google::protobuf::int64) 1), "required_double", 16.0);

//Retrieve and check if update went through
SOM_TRY
retrievedValues = testConverter.retrieve(keys);
SOM_CATCH("Error retrieving values")

REQUIRE(retrievedValues.size() == 1);

REQUIRE(retrievedValues[0].required_int64() == 1);
REQUIRE(retrievedValues[0].required_double() == Approx(16.0));
REQUIRE(retrievedValues[0].required_string() == "3");
REQUIRE(retrievedValues[0].required_enum() == TEST_REGISTERED_COMMUNITY);

REQUIRE(retrievedValues[0].has_optional_int64() == true);
REQUIRE(retrievedValues[0].has_optional_double() == true);
REQUIRE(retrievedValues[0].has_optional_string() == true);
REQUIRE(retrievedValues[0].has_optional_enum() == true);

REQUIRE(retrievedValues[0].has_null_optional_int64() == false);
REQUIRE(retrievedValues[0].has_null_optional_double() == false);
REQUIRE(retrievedValues[0].has_null_optional_string() == false);
REQUIRE(retrievedValues[0].has_null_optional_enum() == false);

REQUIRE(retrievedValues[0].optional_int64() == 4);
REQUIRE(retrievedValues[0].optional_double() == Approx(5.0));
REQUIRE(retrievedValues[0].optional_string() == "6");
REQUIRE(retrievedValues[0].optional_enum() == TEST_COMMUNITY);

REQUIRE(retrievedValues[0].repeated_int64_size() == 3);
REQUIRE(retrievedValues[0].repeated_int64(0) == 7);
REQUIRE(retrievedValues[0].repeated_int64(1) == 8);
REQUIRE(retrievedValues[0].repeated_int64(2) == 9);
REQUIRE(retrievedValues[0].repeated_double_size() == 3);
REQUIRE(retrievedValues[0].repeated_double(0) == Approx(10.0));
REQUIRE(retrievedValues[0].repeated_double(1) == Approx(11.0));
REQUIRE(retrievedValues[0].repeated_double(2) == Approx(12.0));
REQUIRE(retrievedValues[0].repeated_string_size() == 3);
REQUIRE(retrievedValues[0].repeated_string(0) == "13");
REQUIRE(retrievedValues[0].repeated_string(1) == "14");
REQUIRE(retrievedValues[0].repeated_string(2) == "15");
REQUIRE(retrievedValues[0].repeated_enum_size() == 3);
REQUIRE(retrievedValues[0].repeated_enum(0) == TEST_OFFICIAL);
REQUIRE(retrievedValues[0].repeated_enum(1) == TEST_REGISTERED_COMMUNITY);
REQUIRE(retrievedValues[0].repeated_enum(2) == TEST_COMMUNITY);


//Delete the object from the database
SOM_TRY
testConverter.deleteObjects(keys);
SOM_CATCH("Error deleting objects")

//Test if delete took
SOM_TRY
retrievedValues = testConverter.retrieve(keys);
SOM_CATCH("Error retrieving values")

REQUIRE(retrievedValues.size() == 0);
}
}

TEST_CASE( "Test unauthenticated stream registration", "[test]")
{

SECTION( "Register a stream")
{
//Make ZMQ context
std::unique_ptr<zmq::context_t> context;

SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing ZMQ context\n")

//Generate keys to use
char z85CasterPublicKey[41];
char z85CasterSecretKey[41];

REQUIRE(zmq_curve_keypair(z85CasterPublicKey, z85CasterSecretKey) == 0);

//Convert to binary format
char buffer[32];
std::string decodedCasterPublicKey;
std::string decodedCasterSecretKey;

zmq_z85_decode((uint8_t *) buffer, z85CasterPublicKey);
decodedCasterPublicKey = std::string(buffer, 32);

zmq_z85_decode((uint8_t *) buffer, z85CasterSecretKey);
decodedCasterSecretKey = std::string(buffer, 32);


Poco::Int64 casterID = 989;
int unauthenticatedRegistrationPort = 9010;
int authenticatedRegistrationPort = 9011;
int clientRequestPort = 9013;
int clientPublishingPort = 9014;
int proxyPublishingPort = 9015;
int streamStatusNotificationPort = 9016;


//caster myCaster(context.get(), casterID,unauthenticatedRegistrationPort,authenticatedRegistrationPort,clientRequestPort, clientPublishingPort, proxyPublishingPort, streamStatusNotificationPort, decodedCasterPublicKey, decodedCasterSecretKey);
caster myCaster(context.get(), casterID,unauthenticatedRegistrationPort,authenticatedRegistrationPort,clientRequestPort, clientPublishingPort, proxyPublishingPort, streamStatusNotificationPort, decodedCasterPublicKey, decodedCasterSecretKey, "testSQLDatabase.db");

//Create a basestation and register it
std::string serializedRegistrationRequest;
transmitter_registration_request registrationRequest;
auto basestationInfo = registrationRequest.mutable_stream_info();
basestationInfo->set_latitude(1.0);
basestationInfo->set_longitude(2.0);
basestationInfo->set_expected_update_rate(3.0);
basestationInfo->set_message_format(RTCM_V3_1);
basestationInfo->set_informal_name("testBasestation");

registrationRequest.SerializeToString(&serializedRegistrationRequest);


//Create socket to talk with caster
std::unique_ptr<zmq::socket_t> registrationSocket;

SOM_TRY //Init socket
registrationSocket.reset(new zmq::socket_t(*context, ZMQ_DEALER));
SOM_CATCH("Error making socket\n")

SOM_TRY
int timeoutWaitTime = 5000; //Max 5 seconds
registrationSocket->setsockopt(ZMQ_RCVTIMEO, (void *) &timeoutWaitTime, sizeof(timeoutWaitTime));
SOM_CATCH("Error setting socket timeout\n")

SOM_TRY //Connect to caster
std::string connectionString = "tcp://127.0.0.1:" +std::to_string(unauthenticatedRegistrationPort);
registrationSocket->connect(connectionString.c_str());
SOM_CATCH("Error connecting socket for registration with caster\n")

SOM_TRY //Send registration request
registrationSocket->send(serializedRegistrationRequest.c_str(), serializedRegistrationRequest.size());
SOM_CATCH("Error sending base station registration request\n")

std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

REQUIRE(registrationSocket->recv(messageBuffer.get()) == true);

transmitter_registration_reply registrationReply;

std::string stringToPrint( (const char *) messageBuffer->data(), messageBuffer->size());

registrationReply.ParseFromArray(messageBuffer->data(), messageBuffer->size());
REQUIRE(registrationReply.IsInitialized() == true);

REQUIRE(registrationReply.request_succeeded() == true);

//Base station registered, so subscribe and see if we get the next message send
//Create socket to subscribe to caster
std::unique_ptr<zmq::socket_t> subscriberSocket;

SOM_TRY //Init socket
subscriberSocket.reset(new zmq::socket_t(*context, ZMQ_SUB));
SOM_CATCH("Error making socket\n")

SOM_TRY
int timeoutWaitTime = 5000; //Max 5 seconds
subscriberSocket->setsockopt(ZMQ_RCVTIMEO, (void *) &timeoutWaitTime, sizeof(timeoutWaitTime));
SOM_CATCH("Error setting socket timeout\n")

SOM_TRY //Connect to caster
std::string connectionString = "tcp://127.0.0.1:" +std::to_string(clientPublishingPort);
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
registrationSocket->send(testString.c_str(), testString.size());
SOM_CATCH("Error sending message to caster\n")

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

REQUIRE(subscriberSocket->recv(messageBuffer.get()) == true);

//Check message format
REQUIRE(messageBuffer->size() == (testString.size() + sizeof(Poco::Int64)*2));

REQUIRE(Poco::ByteOrder::fromNetwork(*((Poco::Int64 *) messageBuffer->data())) == casterID);

REQUIRE(std::string((((const char *) messageBuffer->data()) + sizeof(Poco::Int64)*2), testString.size()) == testString);

//Send a query and see if we get a valid response
std::string serializedClientRequest;
client_query_request clientRequest; //Empty request should return all

clientRequest.SerializeToString(&serializedClientRequest);

std::unique_ptr<zmq::socket_t> clientSocket;

SOM_TRY //Init socket
clientSocket.reset(new zmq::socket_t(*context, ZMQ_REQ));
SOM_CATCH("Error making socket\n")

SOM_TRY
int timeoutWaitTime = 5000; //Max 5 seconds
clientSocket->setsockopt(ZMQ_RCVTIMEO, (void *) &timeoutWaitTime, sizeof(timeoutWaitTime));
SOM_CATCH("Error setting socket timeout\n")

SOM_TRY //Connect to caster
std::string connectionString = "tcp://127.0.0.1:" +std::to_string(clientRequestPort);
clientSocket->connect(connectionString.c_str());
SOM_CATCH("Error connecting socket for registration with caster\n")

SOM_TRY
clientSocket->send(serializedClientRequest.c_str(), serializedClientRequest.size());
SOM_CATCH("Error, unable to send client request\n")

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

REQUIRE(clientSocket->recv(messageBuffer.get()) == true);

client_query_reply queryReply;
queryReply.ParseFromArray(messageBuffer->data(), messageBuffer->size());

REQUIRE(queryReply.IsInitialized() == true);
REQUIRE(queryReply.has_caster_id() == true);
REQUIRE(queryReply.caster_id() == casterID);
REQUIRE(queryReply.has_failure_reason() == false);
REQUIRE(queryReply.base_stations_size() == 1);

auto replyBaseStationInfo = queryReply.base_stations(0);
REQUIRE(replyBaseStationInfo.has_latitude());
REQUIRE(replyBaseStationInfo.latitude() == Approx(1.0));
REQUIRE(replyBaseStationInfo.has_longitude());
REQUIRE(replyBaseStationInfo.longitude() == Approx(2.0));
REQUIRE(replyBaseStationInfo.has_expected_update_rate());
REQUIRE(replyBaseStationInfo.expected_update_rate() == Approx(3.0));
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
(*clientRequest.add_subqueries()) = subQuery0;
(*clientRequest.add_subqueries()) = subQuery1;

clientRequest.SerializeToString(&serializedClientRequest);

SOM_TRY
clientSocket->send(serializedClientRequest.c_str(), serializedClientRequest.size());
SOM_CATCH("Error, unable to send client request\n")

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

REQUIRE(clientSocket->recv(messageBuffer.get()) == true);

queryReply.ParseFromArray(messageBuffer->data(), messageBuffer->size());
REQUIRE(queryReply.IsInitialized() == true);
REQUIRE(queryReply.has_caster_id() == true);
REQUIRE(queryReply.caster_id() == casterID);
REQUIRE(queryReply.has_failure_reason() == false);
REQUIRE(queryReply.base_stations_size() == 1);

replyBaseStationInfo = queryReply.base_stations(0);
REQUIRE(replyBaseStationInfo.has_latitude());
REQUIRE(replyBaseStationInfo.latitude() == Approx(1.0));
REQUIRE(replyBaseStationInfo.has_longitude());
REQUIRE(replyBaseStationInfo.longitude() == Approx(2.0));
REQUIRE(replyBaseStationInfo.has_expected_update_rate());
REQUIRE(replyBaseStationInfo.expected_update_rate() == Approx(3.0));
REQUIRE(replyBaseStationInfo.has_message_format());
REQUIRE(replyBaseStationInfo.message_format() == RTCM_V3_1);
REQUIRE(replyBaseStationInfo.has_informal_name());
REQUIRE(replyBaseStationInfo.informal_name() == "testBasestation");

}
}
