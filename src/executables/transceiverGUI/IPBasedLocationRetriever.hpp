#pragma once

#include<QObject>
#include<string>
#include<utility>

/**
This class retrieves the latitude/longitude associated with the current host's IP (if any) and emits it as a Qt signal.  It is meant to be run in a seperate Qt thread, as it makes network calls which can take some time to complete.
*/
class IPBasedLocationRetriever : public QObject
{
Q_OBJECT
public:

public slots:
/**
This function performs the retrieval operation the object was created for.
*/
void operate();

signals:
void retrievedLatitudeLongitude(std::pair<double, double> inputLatitudeLongitude); //The lat/long pair (in that order)
void finished(); //Called when object task is completed (whether or not the geocode could be retrieved
void error(std::string inputError); //A signal emitted if a error was encountered


}; 
