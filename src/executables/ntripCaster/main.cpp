#include "casterApplication.hpp"

using namespace pylongps; //Use pylongps classes without alteration for now

int main(int argc, char **argv)
{
casterApplication app;
return app.run(argc, argv); //Creates daemon process which runs the "main" member function of the casterApplication
}
