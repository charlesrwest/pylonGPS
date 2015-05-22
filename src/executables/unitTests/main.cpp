#define CATCH_CONFIG_MAIN //Make main function automatically
#include "catch.hpp"
#include "NMEAGGASentence.hpp"


TEST_CASE( "NMEA GGA Sentences are parsed", "[NMEAGGASentence]")
{
NMEAGGASentence testSentence;

SECTION( "Example1 Parse")
{
REQUIRE( testSentence.parse("$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F")  == true);
SECTION( "Testing values" )
{
REQUIRE( testSentence.talkerID == "GP");
REQUIRE( testSentence.hours == 17);
REQUIRE( testSentence.minutes == 28);
REQUIRE( testSentence.seconds == Approx(14.0));
REQUIRE( testSentence.latitude == Approx(37.39109795));
REQUIRE( testSentence.longitude == Approx(-122.0378263));
REQUIRE( testSentence.qualityType == DIFFERENTIAL_GPS_FIX);
REQUIRE( testSentence.numberOfSatellitesInUse == 6);
REQUIRE( testSentence.horizontalDilutionOfPrecision == Approx(1.2));
REQUIRE( testSentence.antennaAltitude == Approx(18.893));
REQUIRE( testSentence.geoidalSeparation == Approx(-25.669));
REQUIRE( testSentence.differentialUpdateAge == Approx(2.0));
REQUIRE( testSentence.differentialBasestationID == std::string("0031"));
}

SECTION("Serialization to Parse Test")
{
std::string serializedString = testSentence.serialize();
NMEAGGASentence testSentence2;

REQUIRE(testSentence2.parse(serializedString) == true);

SECTION( "Testing values" )
{
REQUIRE( testSentence2.talkerID == "GP");
REQUIRE( testSentence2.hours == 17);
REQUIRE( testSentence2.minutes == 28);
REQUIRE( testSentence2.seconds == Approx(14.0));
REQUIRE( testSentence2.latitude == Approx(37.39109795));
REQUIRE( testSentence2.longitude == Approx(-122.0378263));
REQUIRE( testSentence2.qualityType == DIFFERENTIAL_GPS_FIX);
REQUIRE( testSentence2.numberOfSatellitesInUse == 6);
REQUIRE( testSentence2.horizontalDilutionOfPrecision == Approx(1.2));
REQUIRE( testSentence2.antennaAltitude == Approx(18.893));
REQUIRE( testSentence2.geoidalSeparation == Approx(-25.669));
REQUIRE( testSentence2.differentialUpdateAge == Approx(2.0));
REQUIRE( testSentence2.differentialBasestationID == std::string("0031"));
}
}
}
}
