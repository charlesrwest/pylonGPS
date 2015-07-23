#include "PocoTypeHandlers.hpp"

using namespace Poco::Data;

void TypeHandler<class pylongps::base_station_stream_information>::bind(std::size_t inputPosition, const pylongps::base_station_stream_information inputObject, AbstractBinder::Ptr inputBinder, AbstractBinder::Direction inputDirection)
{
if(inputBinder.isNull())
{
throw SOMException("Invalid ZMQ context\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Form in table: (1) base_station_id integer primary key, (2) latitude real, (3) longitude real, (4) expected_update_rate real, (5) message_format integer, (6) informal_name text, (7) source_public_key text, (8) start_time integer


}

std::size_t TypeHandler<class pylongps::base_station_stream_information>::size()
{
}

void TypeHandler<class pylongps::base_station_stream_information>::prepare(std::size_t inputPosition, const pylongps::base_station_stream_information &inputObject, AbstractPreparator::Ptr inputPreparator)
{
}

void TypeHandler<class pylongps::base_station_stream_information>::extract(std::size_t inputPosition, pylongps::base_station_stream_information &inputObject, const pylongps::base_station_stream_information &inputDefaults, AbstractExtractor::Ptr inputExtractor) 
{
}
