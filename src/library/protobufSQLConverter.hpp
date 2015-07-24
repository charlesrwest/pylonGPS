#ifndef PROTOBUFSQLCONVERTERHPP
#define PROTOBUFSQLCONVERTERHPP

#include<functional>
#include<tuple>
#include<vector>
#include "google/protobuf/message.h"
#include "SOMException.hpp"

//Use resolve from https://groups.google.com/a/isocpp.org/forum/#!msg/std-discussion/rLVGeGUXsK0/IGj9dKmSyx4J for function overload resolution once C++14 is widely used.  In the mean time, static cast will have to do.

//MACROS to make it easy to pass the information associated with a field by generating tuples with the get/set/has parameters
//USAGE: macroname(className, fieldName, stringOfNameInDatabase) 
//Generates: //stringField<className>(&className::fieldName, static_cast<void (className::*)(const ::std::string& value)>(&className::set_fieldName), &className::has_fieldName, stringOfNameInDatabase)
#define PYLON_GPS_GEN_OPTIONAL_INT64_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) int64Field<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, &INPUT_CLASS::set_##INPUT_FIELD_NAME, &INPUT_CLASS::has_##INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING)

#define PYLON_GPS_GEN_REQUIRED_INT64_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) int64Field<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, &INPUT_CLASS::set_##INPUT_FIELD_NAME, nullptr, INPUT_DATABASE_FIELD_STRING)

#define PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) doubleField<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, &INPUT_CLASS::set_##INPUT_FIELD_NAME, &INPUT_CLASS::has_##INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING)

#define PYLON_GPS_GEN_REQUIRED_DOUBLE_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) doubleField<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, &INPUT_CLASS::set_##INPUT_FIELD_NAME, nullptr, INPUT_DATABASE_FIELD_STRING)

#define PYLON_GPS_GEN_OPTIONAL_STRING_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) stringField<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, static_cast<void (INPUT_CLASS::*)(const ::std::string& value)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), &INPUT_CLASS::has_##INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) 

#define PYLON_GPS_GEN_REQUIRED_STRING_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) stringField<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, static_cast<void (INPUT_CLASS::*)(const ::std::string& value)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), nullptr, INPUT_DATABASE_FIELD_STRING) 



namespace pylongps
{

/**
A tuple of form: int64 get function, int64 set function, has function (nullptr if field is required), name of the field in the SQL database.
*/
template <class classType> using int64Field = std::tuple<std::function<const ::google::protobuf::int64 &(const classType* )>, std::function<void (classType*, ::google::protobuf::int64)>, std::function<bool(const classType*)>, std::string>;

/**
A tuple of form: double get function, double set function, has function (nullptr if field is required), name of the field in the SQL database.
*/
template <class classType> using doubleField = std::tuple<std::function<const double &(const classType*)>, std::function<void (classType*, double)>, std::function<bool(const classType*)>, std::string>;

/**
A tuple of form: string get function, string set function, has function (nullptr if field is required), name of the field in the SQL database.
*/
template <class classType> using stringField = std::tuple<std::function<const std::string &(const classType*)>, std::function<void (classType*, const std::string &)>, std::function<bool(const classType*)>, std::string>;



/**
This template class is suppose to make it simple to insert an arbitrary protobuf object to and from a database by passing the object's has/get/set functions and later passing objects to use those functions on.
*/
template <class classType> class protobufSQLConverter
{
public:

/**
This specific overload lets you add a int64 field for the converter to be able to check/get/set for database reading and writing.
@param inputInt64Field: A tuple of form: get function, set function, has function (nullptr if field is required), name of the field in the SQL database
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
void addField(const int64Field<classType> &inputInt64Field, bool inputIsPrimaryKey = false)
{
if(std::get<0>(inputInt64Field) == nullptr || std::get<1>(inputInt64Field) == nullptr)
{ //Get or set functions are invalid
throw SOMException("Get or set methods are NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

int64Fields.push_back(inputInt64Field);
if(inputIsPrimaryKey)
{
int64FieldsPrimaryKeyIndexes.push_back(int64Fields.size() - 1);
}
}

/**
This specific overload lets you add a double field for the converter to be able to check/get/set for database reading and writing.
@param inputDoubleField: A tuple of form: get function, set function, has function (nullptr if field is required), name of the field in the SQL database
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
void addField(const doubleField<classType> &inputDoubleField, bool inputIsPrimaryKey = false)
{
if(std::get<0>(inputDoubleField) == nullptr || std::get<1>(inputDoubleField) == nullptr)
{ //Get or set functions are invalid
throw SOMException("Get or set methods are NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

doubleFields.push_back(inputDoubleField);
if(inputIsPrimaryKey)
{
doubleFieldsPrimaryKeyIndexes.push_back(doubleFields.size() - 1);
}
}

/**
This specific overload lets you add a string field for the converter to be able to check/get/set for database reading and writing.
@param inputStringField: A tuple of form: get function, set function, has function (nullptr if field is required), name of the field in the SQL database
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
void addField(const stringField<classType> &inputStringField, bool inputIsPrimaryKey = false)
{
if(std::get<0>(inputStringField) == nullptr || std::get<1>(inputStringField) == nullptr)
{ //Get or set functions are invalid
throw SOMException("Get or set methods are NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

stringFields.push_back(inputStringField);
if(inputIsPrimaryKey)
{
stringFieldsPrimaryKeyIndexes.push_back(stringFields.size() - 1);
}
}



void callFunction(classType &inputTestClass)
{
//printf("Called with value: %d\n", memberFunctionPointer(&inputTestClass));
}

/**
This function just prints things for now
*/
void serialize(const classType &inputClass)
{ //Should check if optional field is present
for(int i=0; i<int64Fields.size(); i++)
{
printf("Int64 field (%s): %ld\n", std::get<3>(int64Fields[i]).c_str(), std::get<0>(int64Fields[i])(&inputClass));
}

for(int i=0; i<doubleFields.size(); i++)
{
printf("Double field (%s): %lf\n", std::get<3>(doubleFields[i]).c_str(), std::get<0>(doubleFields[i])(&inputClass));
}

for(int i=0; i<stringFields.size(); i++)
{
printf("String field (%s): %s\n", std::get<3>(stringFields[i]).c_str(), std::get<0>(stringFields[i])(&inputClass).c_str());
}

}


std::vector<int64Field<classType> > int64Fields;
std::vector<int> int64FieldsPrimaryKeyIndexes; //Indexes of primary key fields in the int64Fields vector

std::vector<doubleField<classType> > doubleFields;
std::vector<int> doubleFieldsPrimaryKeyIndexes; //Indexes of primary key fields in the doubleFields vector

std::vector<stringField<classType> > stringFields;
std::vector<int> stringFieldsPrimaryKeyIndexes; //Indexes of primary key fields in the stringFields vector

};

//Given singular field:
//get/set/has functions (has = nullptr if required)
//Name in database
//Type should be automatically identified by function pointer types

//Think I am going to restrict it so that there can only be one primary key in the primary table (rather than multicolumn primary keys).  This simplfies syntax for repeated fields considerably and can always be changed later.

//Given repeated field
//size,get(index)/set(index)
//table in database, reference name field name in database








}
#endif 
