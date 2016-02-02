#include "commandLineArgumentParser.hpp"

using namespace pylongps;

/**
This constructor doesn't do anything so that member functions can be called to set conditions before parsing.
*/
commandLineParser::commandLineParser()
{
}

/**
This function calls parse on the given arguments with the default options.
@param inputStringArrays: The array of strings to read from (must be null terminated)
@param inputNumberOfStringArrays: How many string arrays to process
*/
commandLineParser::commandLineParser(const char **inputStringArrays, int inputNumberOfStringArrays)
{
parse(inputStringArrays, inputNumberOfStringArrays);
}

/**
This function returns true if optionToAssociatedArguments map has an entry for the given option.
@param inputOption: The option to check for
@return: true if the option has an entry and false otherwise
*/
bool commandLineParser::has(const std::string &inputOption) const
{
return optionToAssociatedArguments.count(inputOption) > 0;
}

/**
This function returns true if one or more of the given options are in the optionToAssociatedArguments map.
@param inputOptionsToCheck: The set of options to check if the parser has
@return: true if one or more of the options are in the parser results
*/
bool commandLineParser::hasOneOrMore(const std::vector<std::string> &inputOptionsToCheck) const
{
for(const std::string &option : inputOptionsToCheck)
{
if(has(option))
{
return true;
}
}

return false;//None of the options have been found
}

/**
This function populate's this objects <string, std::vector<string>> map with the options it was able to detect and their associated values.  The first string array is assumed to be the program name and is stored as the pair <"PROGRAM", std::vector with actual name and all arguments that were not affilated with options>.  Unless it has been previously set otherwise, all non option arguments which follow an option argument (denoted with the "-" symbol) are stored in the list associated with the option (which is stripped of its "-" symbol and then acts as a key).  In the case of an option showing up more than one time, the associated arguments are concatenated to the list.
@param inputStringArrays: The array of strings to read from (must be null terminated)
@param inputNumberOfStringArrays: How many string arrays to process
*/
void commandLineParser::parse(const char **inputStringArrays, int inputNumberOfStringArrays)
{
optionToAssociatedArguments.clear();
optionStringToMaximumNumberOfArguments.erase(PROGRAM_STRING);

if(inputNumberOfStringArrays > 0)
{
optionToAssociatedArguments[PROGRAM_STRING] = std::vector<std::string>{inputStringArrays[0]};
}
else
{
optionToAssociatedArguments[PROGRAM_STRING] = std::vector<std::string>{""};
}

std::string currentOptionString = PROGRAM_STRING;
int numberOfArgumentsForTheCurrentOption = 0;
for(int i=1; i<inputNumberOfStringArrays; i++)
{
std::string currentArgument = inputStringArrays[i];
if(currentArgument.find("-") == 0 && (currentArgument.size() >= 2 && !std::isdigit(currentArgument[1]))) //Depends on && order to prevent segfault
{//It is an option

if(ignoreOptionCaseFlag)
{//Make lower case
std::transform(currentArgument.begin(), currentArgument.end(), currentArgument.begin(), ::tolower);
}

currentOptionString = currentArgument.substr(1); //Remove '-'
numberOfArgumentsForTheCurrentOption = 0;
if(optionToAssociatedArguments.count(currentOptionString) == 0)
{//Add an empty vector to indicate this option was present and allow associated arguments to be added
optionToAssociatedArguments[currentOptionString] = std::vector<std::string>();
}
continue;
}

//It's an non option argument
if(optionStringToMaximumNumberOfArguments.count(currentOptionString) > 0)
{
if(optionStringToMaximumNumberOfArguments.at(currentOptionString) <= numberOfArgumentsForTheCurrentOption)
{//The current option has exceeded it's allowed number of arguments per instance, so set it as a program argument instead
optionToAssociatedArguments[PROGRAM_STRING].push_back(currentArgument);
continue;
}
}

optionToAssociatedArguments[currentOptionString].push_back(currentArgument);
numberOfArgumentsForTheCurrentOption++;
}

}

/**
If set to true, the parser will shift all options to lower case before considering them (making "-option" and "-OpTion" equivalent and stored as "option").
@param inputIgnoreCase: True if option case should be ignored
*/
void commandLineParser::ignoreOptionCase(bool inputIgnoreCase)
{
ignoreOptionCaseFlag = inputIgnoreCase;
}

/**
This function sets the maximum number of arguments that a the parser will consider to belong to the given option in one instance.  This allows syntax such as "-f fileToConsider" without causing the option to potentially claim arguments that don't belong to it.
@param inputOptionString: The option string to clamp, such as "option" (will convert to lower case if that option is enabled).
@param inputMaximumNumberOfArguments: The maximum number of arguments that are permitted to follow a single instance of the option.
*/
void commandLineParser::clampOptionNumberOfOptionArguments(const std::string &inputOptionString, int inputMaximumNumberOfArguments)
{
optionStringToMaximumNumberOfArguments[inputOptionString] = inputMaximumNumberOfArguments;
} 

/**
This function is used to generate suggestions in option invalid messages and clamp the number of arguments to pass to an option based on its suggested usage.
@param inputOption: The option to generate suggestion messages for and potentially determine the clamp number for
@param inputSuggestedArguments: The list of arguments the option is expecting
@param inputAutomaticallyClampNumberOfArguments: True if the number of arguments should be deduced and clamping applied
*/
void commandLineParser::addSuggestedUsage(const std::string &inputOption, const std::vector<std::string> &inputSuggestedArguments, bool inputAutomaticallyClampNumberOfArguments)
{
optionToSuggestedArguments[inputOption] = inputSuggestedArguments;
if(inputAutomaticallyClampNumberOfArguments)
{
clampOptionNumberOfOptionArguments(inputOption, inputSuggestedArguments.size());
}
}

/**
This function prints all of the option/argument pairs stored in the parser.
*/
void commandLineParser::printRetrievedOptionArgumentPairs() const
{
for(const auto &instance : optionToAssociatedArguments)
{
printf("%s : ", instance.first.c_str());
for(const std::string &argumentString : instance.second)
{
printf("%s  ", argumentString.c_str());
}
printf("\n");
}
}

/**
This function prints an error message to stderr regarding a particular option (missing if the option is not found, error otherwise).
@param inputOption: The option to print the message about
*/
void commandLineParser::printErrorMessageRegardingOption(const std::string &inputOption) const
{
if(optionToAssociatedArguments.count(inputOption) == 0)
{
fprintf(stderr, "Error, required option %s missing\n", inputOption.c_str());
return;
}

const std::vector<std::string> &arguments =  optionToAssociatedArguments.at(inputOption);

fprintf(stderr, "Error, option %s with argument(s) ", inputOption.c_str());
for(const std::string &argument : arguments)
{
fprintf(stderr, "%s ", argument.c_str());
}
fprintf(stderr, "is invalid\n");

if(optionToSuggestedArguments.count(inputOption) > 0)
{
fprintf(stderr, "Suggested option usage: -%s ", inputOption.c_str());
const std::vector<std::string> &suggestedArguments = optionToSuggestedArguments.at(inputOption);
for(const std::string &suggestedArgument : suggestedArguments)
{
fprintf(stderr, "%s ", suggestedArgument.c_str());
}
printf("\n");
}

}

/**
This function prints a message to stdout which lists the options which have suggested arguments and the associated suggestions.
*/
void commandLineParser::printHelpMessage() const
{
printf("Available options:\n");
for(const auto &instance : optionToSuggestedArguments)
{
printf("-%s", instance.first.c_str());
const std::vector<std::string> &suggestedArguments = instance.second;
if(suggestedArguments.size() > 0)
{
printf(" : ");
}

for(const std::string &suggestedArgument : suggestedArguments)
{
printf("%s ", suggestedArgument.c_str());
}
printf("\n");
}
}
