#include "randomStringGenerator.hpp"

/**
This function generates a random, non-deterministic alphanumeric string.
@param inputStringLength: The number of characters in the returned string
@return: The returned string
*/
std::string pylongps::generateRandomString(unsigned int inputStringLength)
{
const char characterArray[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const size_t maxIndex = (sizeof(characterArray) - 1);

std::random_device randomDevice;
std::uniform_int_distribution<> uniformDistribution(0, maxIndex);

auto generateCharacter = [&] () -> char
{
return characterArray[uniformDistribution(randomDevice)];
};

std::string stringToReturn(inputStringLength, 0);
std::generate_n(stringToReturn.begin(), stringToReturn.size(), generateCharacter);
return stringToReturn;
} 
