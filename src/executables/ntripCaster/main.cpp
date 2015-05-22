#include "casterApplication.hpp"


int main(int argc, char **argv)
{
casterApplication app;
return app.run(argc, argv); //Creates daemon process which runs the "main" member function of the NTRIPCasterApplication
}
