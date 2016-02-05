#pragma once

#include "Poco/ByteOrder.h"
#include<chrono>

#include "transceiver.hpp"
#include "transceiver_configuration.pb.h"
#include "commandLineArgumentParser.hpp"
#include<array>
#include "zmq.hpp"
#include<iostream>

namespace pylongps
{
typedef std::tuple<std::string, std::array<int64_t, 6>> casterConnectionInfo;
const int64_t MILLISECONDS_TO_WAIT_FOR_QUERY_RESPONSE = 10000;

class commandLineTransceiver
{
public:
/**
This function initializes the transceiver.
@param inputArgumentStrings: The strings forming the program arguments
@param inputNumberOfArguments: How many argument strings there are
*/
commandLineTransceiver(char **inputArgumentStrings, int inputNumberOfArguments);

/**
This function sets a parser up to restrict the number of arguments for each option and enable the parser to generate intelligent error messages when an option is invalid.
*/
void addOptionSettingsToParser();

/**
This function initializes a structure containing the information required to connect to a caster using information associated with a option to the command line parser or the URL/default information associated with pylongps.com if the option is not found.
@param inputCasterOptionString: The option to get it from
@param inputCasterInfoBuffer: The structure to store it in
@return: True if the data was able to be received successfully, false otherwise
*/
bool parseCasterInfoOrGetFromDefaultURL(const std::string &inputCasterOptionString, casterConnectionInfo &inputCasterInfoBuffer);

/**
This function intitializes a range subquery based on the arguments in the parser.
@param inputSubqueryBuffer: The subquery to initialize
@return: true if successful and false otherwise
*/
bool parseAndInitializeSubquery(client_subquery &inputSubqueryBuffer);

/**
This function prints the results of a query to stdout.
@param inputReply: The reply to print
@param inputLatitude: The latitude the center results are sorted from, if any
@param inputLongitude: The longitude the center results are sorted from, if any
@param inputCenterPointIsValid: true if the lat/lon is valid
*/
void printQueryResults(const client_query_reply &inputReply, double inputLatitude = 0.0, double inputLongitude = 0.0, bool inputCenterPointIsValid = true);

/**
This function sends a query to the caster and prints the results of the query to stdout.
@return: true if successful and false if an error occurred
*/
bool sendQueryAndPrintResults();

/**
This function constructs a proper query based on the command arguments.
@param inputQueryBuffer: The buffer to store the constructed
@return: true if successful
*/
bool constructQuery(client_query_request &inputQueryBuffer);

/**
This function constructs a proper query based on the command arguments.
@param inputReplyBuffer: The buffer to store the received reply in
@return: true if successful
*/
bool constructAndSendQuery(client_query_reply &inputReplyBuffer);

/*
This function queries the caster, finds first basestation that matches the query (often sorted by distance) and connects to receive its updates.
@return: true if successful
*/
bool constructBasestationDataReceiver();

/**
This function constructs a data receiver that listens for the caster/basestation combination given using the read_basestation option.
@return: true if successful
*/
bool constructParticularBasestationDataReceiver();

/**
This function constructs a data receiver that reads from a file.
@return: true if successful
*/
bool constructFileDataReceiver();

/**
This function constructs a data receiver that reads from a tcp server.
@return: true if successful
*/
bool constructTCPDataReceiver();

/**
This function constructs a data receiver that reads from a ZMQ publisher.
@return: true if successful
*/
bool constructZMQDataReceiver();

/**
This function constructs a data sender that writes to a file.
@return: true if successful
*/
bool constructFileDataSender();

/**
This function constructs a data sender that creates a tcp server.
@return: true if successful
*/
bool constructTCPDataSender();

/**
This function constructs a data sender that creates a ZMQ publisher.
@return: true if successful
*/
bool constructZMQDataSender();

/*
This function constructs a data sender that connects and shares the data as a basestation.
@return: true if successful
*/
bool constructBasestationDataSender();


zmq::context_t context;
commandLineParser parser;
transceiver dataTransceiver;
casterConnectionInfo inputCasterConnectionInfo;
casterConnectionInfo outputCasterConnectionInfo;
std::string dataReceiverConnectionString;
std::string dataSenderID;
bool shouldWait;
};
}
