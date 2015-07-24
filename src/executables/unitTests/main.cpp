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
#include "credentials.pb.h"


using namespace pylongps; //Use pylongps classes without alteration for now

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
printf("Called with value: %d\n", memberFunctionPointer(&inputTestClass));
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

SECTION( "Get a string")
{
//Initialize protobufSQLConverter
protobufSQLConverter<credentials> testConverter;

//Attempt to add fields for credentials
std::function<const std::string &(const credentials*)> get(&credentials::permissions);
std::function<void (credentials*, const std::string &)> set(static_cast<void (credentials::*)(const ::std::string& value)>(&credentials::set_permissions));
std::function<bool(const credentials*)> has(&credentials::has_permissions);
std::string fieldName = "test";
std::tuple<std::function<const std::string &(const credentials*)>, std::function<void (credentials*, const std::string &)>, std::function<bool(const credentials*)>, std::string> testTuple(get, set, has, fieldName);
testConverter.addField(testTuple);


}
}
