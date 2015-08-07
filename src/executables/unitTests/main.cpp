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
