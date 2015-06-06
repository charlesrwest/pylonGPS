/**
\defgroup Server
\defgroup Serialization 
\defgroup Events
*/

/**
\mainpage
Hello and welcome.

We are just starting to see the deployment of robots in a commercial setting and most of the pieces are in place for Robotics as a Service. This project is dedicated to making it even easier to do interesting and useful robotics applications using inexpensive differential GPS.

Differential GPS gives enhanced accuracy (<10 cm radius vs the normal 3000 cm radius) which can enable applications such as landing a quadcopter on a small platform or keeping a mobile robot on the sidewalk. It works by having a nearby GPS basestation with a known location that can tell the mobile unit how to compensate for the errors that the basestation has observed in the GPS signal.

Pylon GPS makes it easy to share GPS updates from your basestation and allows mobile units to get updates from any nearby basestations. This means that when a basestation with Pylon GPS is deployed, everyone within 10 km of the basestation can get updates and create applications using differential GPS. It works by maintaining a central server with a list of basestations and relaying laying updates using the NTRIP protocol. Future versions of the software will make it easier to search and host your own update sharing servers using MessagePack and ZeroMQ for serialization and network transport. 

This project has just been started and does not yet have functional code.  This unfortunate situation should be rectified by May 23rd, 2015.
*/ 
