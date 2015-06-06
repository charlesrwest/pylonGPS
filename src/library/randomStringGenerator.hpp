#ifndef  RANDOMSTRINGGENERATORHPP
#define RANDOMSTRINGGENERATORHPP

#include<string>
#include<random>
#include<algorithm>

namespace pylongps
{

/**
This function generates a random, non-deterministic alphanumeric string.
@param inputStringLength: The number of characters in the returned string
@return: The returned string
*/
std::string generateRandomString(unsigned int inputStringLength);





}
#endif
