#include "Poco/ByteOrder.h"
#include<chrono>

#include "transceiver.hpp"
#include "transceiver_configuration.pb.h"
#include "commandLineArgumentParser.hpp"
#include<array>
#include "zmq.hpp"
#include "commandLineTransceiver.hpp"

using namespace pylongps;



//Defined options
// -C configurationFile
// -help list of possible options
// -near latitude longitude
// -max_range distance -> the maximum distance to search for basestations (in meters) 
// -official -> return official basestations and not community
// -registered -> return registered basestations and not community
// -autoconnect -> use the first basestation form data source
// -read_file fileName ->use given file as data source
// -read_tcp ipAddress PortNumber -> use given tcp server as data source
// -read_zmq ipAddress PortNumber -> use given zmq publisher as data source
// -read_basestation casterID streamID
// -input_caster IP PortNumbers -> use non pylongps.com caster
// -output_caster IP PortNumbers -> use non pylongps.com caster
// -output_file filename
// -output_tcp PortNumber
// -output_zmq PortNumber
// -output_basestation latitude longitude format informalName expectedUpdateRate
// -private_key keyFile -> private key to use for new basestation
// -credentials_file credentialsFile -> the credentials file to use for the new basestation



int main(int argc, char** argv)
{
std::unique_ptr<commandLineTransceiver> dataTransceiver;

try
{
dataTransceiver.reset(new commandLineTransceiver(argv, argc));
}
catch(const std::exception &inputException)
{
return 1;
}

while(dataTransceiver->shouldWait)
{
std::this_thread::sleep_for(std::chrono::seconds(2));
}

return 0;
}




