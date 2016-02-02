#pragma once

#include<map>
#include<string>
#include<algorithm>
#include<cstdio>

namespace pylongps
{

const std::string PROGRAM_STRING = "PROGRAM";

/**
This class can be used to simplify processing command line arguments in a program by seperating them out depending on the option.  It only recognizes non-numeric option names (no -1 as an option, just -optionName -1AsArgument).
*/
class commandLineParser
{
public:
/**
This constructor doesn't do anything so that member functions can be called to set conditions before parsing.
*/
commandLineParser();

/**
This function calls parse on the given arguments with the default options.
@param inputStringArrays: The array of strings to read from (must be null terminated)
@param inputNumberOfStringArrays: How many string arrays to process
*/
commandLineParser(const char **inputStringArrays, int inputNumberOfStringArrays);

/**
This function returns true if optionToAssociatedArguments map has an entry for the given option.
@param inputOption: The option to check for
@return: true if the option has an entry and false otherwise
*/
bool has(const std::string &inputOption) const;

/**
This function returns true if one or more of the given options are in the optionToAssociatedArguments map.
@param inputOptionsToCheck: The set of options to check if the parser has
@return: true if one or more of the options are in the parser results
*/
bool hasOneOrMore(const std::vector<std::string> &inputOptionsToCheck) const;

/**
This function populate's this objects <string, std::vector<string>> map with the options it was able to detect and their associated values.  The first string array is assumed to be the program name and is stored as the pair <"PROGRAM", std::vector with actual name and all arguments that were not affilated with options>.  Unless it has been previously set otherwise, all non option arguments which follow an option argument (denoted with the "-" symbol) are stored in the list associated with the option (which is stripped of its "-" symbol and then acts as a key).  In the case of an option showing up more than one time, the associated arguments are concatenated to the list.
@param inputStringArrays: The array of strings to read from (must be null terminated)
@param inputNumberOfStringArrays: How many string arrays to process
*/
void parse(const char **inputStringArrays, int inputNumberOfStringArrays);

/**
If set to true, the parser will shift all options to lower case before considering them (making "-option" and "-OpTion" equivalent and stored as "option").
@param inputIgnoreCase: True if option case should be ignored
*/
void ignoreOptionCase(bool inputIgnoreCase);

/**
This function sets the maximum number of arguments that a the parser will consider to belong to the given option in one instance.  This allows syntax such as "-f fileToConsider" without causing the option to potentially claim arguments that don't belong to it.
@param inputOptionString: The option string to clamp, such as "option" (will convert to lower case if that option is enabled).
@param inputMaximumNumberOfArguments: The maximum number of arguments that are permitted to follow a single instance of the option.
*/
void clampOptionNumberOfOptionArguments(const std::string &inputOptionString, int inputMaximumNumberOfArguments);

/**
This function is used to generate suggestions in option invalid messages and clamp the number of arguments to pass to an option based on its suggested usage.
@param inputOption: The option to generate suggestion messages for and potentially determine the clamp number for
@param inputSuggestedArguments: The list of arguments the option is expecting
@param inputAutomaticallyClampNumberOfArguments: True if the number of arguments should be deduced and clamping applied
*/
void addSuggestedUsage(const std::string &inputOption, const std::vector<std::string> &inputSuggestedArguments = std::vector<std::string>(), bool inputAutomaticallyClampNumberOfArguments = true);

/**
This function prints all of the option/argument pairs stored in the parser.
*/
void printRetrievedOptionArgumentPairs() const;

/**
This function prints an error message to stderr regarding a particular option (missing if the option is not found, error otherwise).
@param inputOption: The option to print the message about
*/
void printErrorMessageRegardingOption(const std::string &inputOption) const;

/**
This function prints a message to stdout which lists the options which have suggested arguments and the associated suggestions.
*/
void printHelpMessage() const;

std::map<std::string, std::vector<std::string>> optionToAssociatedArguments;

private:
bool ignoreOptionCaseFlag = false;
std::map<std::string, int> optionStringToMaximumNumberOfArguments;
std::map<std::string, std::vector<std::string>> optionToSuggestedArguments;
};

}
