#define CATCH_CONFIG_MAIN //Make main function automatically
#include "catch.hpp"
#include "caster.hpp"
#include "SOMException.hpp"
#include<memory>
#include<string>
#include "zmq.hpp"
#include "zmq_utils.h"

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
