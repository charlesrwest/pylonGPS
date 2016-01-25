#pragma once

#include<QObject>
#include<string>
#include<utility>
#include "utilityFunctions.hpp"
#include "SOMScopeGuard.hpp"
#include<algorithm>
#include "transceiverGUI.hpp"
#include "client_query_request.pb.h"
#include "client_query_reply.pb.h"
#include<mutex>
#include<QThread>


namespace pylongps
{
const int BASESTATION_LIST_RECEIVER_TIMEOUT_DURATION_IN_MILLISECONDS = 1000;

class transceiverGui;

/**
This class retrieves the latitude/longitude associated with the address it was constructed with (if any) and emits it as a Qt signal.  It is meant to be run in a seperate Qt thread, as it makes network calls which can take some time to complete.
*/
class basestationListRetriever : public QThread
{
Q_OBJECT
public:
/**
This function initializes the object and gives it the GUI for it to update.
@param inputGUI: The GUI to update
@param inputQueryRangeBoundingBox: The west, north, east, south boundaries to consider for the restrictions on the basestations to consider (in radians)
@param inputConnectionString: The ZMQ connection string to use for connecting to the caster

throw: This function can throw exceptions
*/
basestationListRetriever(pylongps::transceiverGUI &inputGUI, std::array<double, 4> &inputBoundArray, const std::string &inputConnectionString);

transceiverGUI &guiReference;
std::array<double, 4> boundArray;
std::unique_ptr<zmq::socket_t> casterRequestSocket;
std::string connectionString;

/**
This function performs the retrieval operation the object was created for.
*/
void run();

signals:
void updatedBasestationLists();
void error(std::string inputError); //A signal emitted if a error was encountered


};  

}
