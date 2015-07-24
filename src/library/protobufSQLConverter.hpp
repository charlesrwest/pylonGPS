#ifndef PROTOBUFSQLCONVERTERHPP
#define PROTOBUFSQLCONVERTERHPP

#include<functional>
#include<tuple>
#include<vector>

//Use resolve from https://groups.google.com/a/isocpp.org/forum/#!msg/std-discussion/rLVGeGUXsK0/IGj9dKmSyx4J for function overload resolution once C++14 is widely used.  In the mean time, static cast will have to do.


namespace pylongps
{


/**
This template class is suppose to make it simple to insert an arbitrary protobuf object to and from a database by passing the object's has/get/set functions and later passing objects to use those functions on.
*/
template <class classType> class protobufSQLConverter
{
public:
void addField(std::tuple<std::function<const std::string &(const classType*)>, std::function<void (classType*, const std::string &)>, std::function<bool(const classType*)>, std::string> inputStringField)
{
stringFields.push_back(inputStringField);
}

void callFunction(classType &inputTestClass)
{
//printf("Called with value: %d\n", memberFunctionPointer(&inputTestClass));
}

std::vector< std::tuple<std::function<const std::string &(const classType*)>, std::function<void (classType*, const std::string &)>, std::function<bool(const classType*)>, std::string> > stringFields;
};

//Given singular field:
//get/set/has functions (has = nullptr if required)
//Name in database
//Type should be automatically identified by function pointer types










}
#endif 
