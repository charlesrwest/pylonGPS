#ifndef POCOTYPEHANDLERSHPP
#define POCOTYPEHANDLERSHPP

#include<cstddef>

#include "Poco/Data/TypeHandler.h"
#include "Poco/Data/AbstractBinder.h"
#include "Poco/Data/AbstractPreparation.h"
#include "Poco/Data/AbstractExtractor.h"
#include "base_station_stream_information.pb.h"
#include "SOM_Exception.hpp"

/**
This file contains handlers to make it easier to insert and retrieve certain types of complex objects.
*/

namespace Poco 
{
namespace Data 
{


template <> class TypeHandler<class pylongps::base_station_stream_information>
{ //This handles inserting and retrieving all of the fields in the message except for the signing keys list

//Form in table: (1) base_station_id integer primary key, (2) latitude real, (3) longitude real, (4) expected_update_rate real, (5) message_format integer, (6) informal_name text, (7) source_public_key text, (8) start_time integer

/**
@throws: This function can throw exceptions
*/
static void bind(std::size_t inputPosition, const pylongps::base_station_stream_information inputObject, AbstractBinder::Ptr inputBinder, AbstractBinder::Direction inputDirection);

/**
*/
static std::size_t size();

/**
*/
static void prepare(std::size_t inputPosition, const pylongps::base_station_stream_information &inputObject, AbstractPreparator::Ptr inputPreparator);

/**
*/
static void extract(std::size_t inputPosition, pylongps::base_station_stream_information &inputObject, const pylongps::base_station_stream_information &inputDefaults, AbstractExtractor::Ptr inputExtractor);

private:
TypeHandler();
~TypeHandler();
TypeHandler(const TypeHandler &inputTypeHandler);
TypeHandler& operator=(const TypeHandler &inputTypeHandler);
};

















}
}
#endif 
