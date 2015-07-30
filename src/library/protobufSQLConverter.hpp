#ifndef PROTOBUFSQLCONVERTERHPP
#define PROTOBUFSQLCONVERTERHPP

#include<functional>
#include<tuple>
#include<vector>
#include "google/protobuf/message.h"
#include "SOMException.hpp"
#include "sqlite3.h"
#include<memory>

//Use resolve from https://groups.google.com/a/isocpp.org/forum/#!msg/std-discussion/rLVGeGUXsK0/IGj9dKmSyx4J for function overload resolution once C++14 is widely used.  In the mean time, static cast will have to do.

//MACROS are defined in protobufSQLConverterDefinitions.hpp to make it easy to pass the information associated with a field by generating tuples with the get/set/has parameters
//USAGE: macroname(className, fieldName, stringOfNameInDatabase) 
//Generates: //stringField<className>(&className::fieldName, static_cast<void (className::*)(const ::std::string& value)>(&className::set_fieldName), &className::has_fieldName, stringOfNameInDatabase)

namespace pylongps
{

enum fieldType //The type of a field
{
INT64_TYPE = 10,
DOUBLE_TYPE = 20,
STRING_TYPE = 30,
};



/**
A tuple of form: int64 get function, int64 set function, has function (nullptr if field is required), name of the field in the SQL database, (optional) represents an enum.
*/
template <class classType> using int64Field = std::tuple<std::function<const ::google::protobuf::int64 &(const classType* )>, std::function<void (classType*, ::google::protobuf::int64)>, std::function<bool(const classType*)>, std::string, bool>;

/**
A tuple of form: double get function, double set function, has function (nullptr if field is required), name of the field in the SQL database.
*/
template <class classType> using doubleField = std::tuple<std::function<const double &(const classType*)>, std::function<void (classType*, double)>, std::function<bool(const classType*)>, std::string>;

/**
A tuple of form: string get function, string set function, has function (nullptr if field is required), name of the field in the SQL database.
*/
template <class classType> using stringField = std::tuple<std::function<const std::string &(const classType*)>, std::function<void (classType*, const std::string &)>, std::function<bool(const classType*)>, std::string>;

/**
A tuple of form: int64 get(index) function, int64 add function, number of entries function, name of the field's associated table in the SQL database, name of the field's value in the table, name of the foreign key which points to the primary key in the main table, true if this object represents an enum
*/
template <class classType> using repeatedInt64Field = std::tuple<std::function< ::google::protobuf::int64 (classType*, int)>, std::function<void (classType*, ::google::protobuf::int64)>,  std::function<int (const classType*)>, std::string, std::string, std::string, bool>;

/**
A tuple of form: double get(index) function, double add function, number of entries function, name of the field's associated table in the SQL database, name of the field's value in the table, name of the foreign key which points to the primary key in the main table
*/
template <class classType> using repeatedDoubleField = std::tuple<std::function< double (classType*, int)>, std::function<void (classType*, double)>,  std::function<int (const classType*)>, std::string, std::string, std::string>;

/**
A tuple of form: string get(index) function, string add function, number of entries function, name of the field's associated table in the SQL database, name of the field's value in the table, name of the foreign key which points to the primary key in the main table
*/
template <class classType> using repeatedStringField = std::tuple<std::function< const std::string &(classType*, int)>, std::function<void (classType*, const ::std::string&)>,  std::function<int (const classType*)>, std::string, std::string, std::string>;



/**
This template class is suppose to make it simple to insert an arbitrary protobuf object to and from a database by passing the object's has/get/set functions and later passing objects to use those functions on.
*/
template <class classType> class protobufSQLConverter
{
public:
/**
This function initializes the converter with the database connection and table to place/retrieve the message objects to/from.
@param inputDatabaseConnection: A pointer to the database connection to use
@param inputPrimaryTableName: The name of the SQL table that has the main row to place objects in/retrieve them from

@throws: This function can throw exceptions
*/
protobufSQLConverter(sqlite3 *inputDatabaseConnection, std::string inputPrimaryTableName);

/**
This specific overload lets you add a int64 field (or a wrapped enum field) for the converter to be able to check/get/set for database reading and writing.
@param inputInt64Field: A tuple of form: get function, set function, has function (nullptr if field is required), name of the field in the SQL database
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
void addField(const int64Field<classType> &inputInt64Field, bool inputIsPrimaryKey = false);

/**
This specific overload lets you add a double field for the converter to be able to check/get/set for database reading and writing.
@param inputDoubleField: A tuple of form: get function, set function, has function (nullptr if field is required), name of the field in the SQL database
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
void addField(const doubleField<classType> &inputDoubleField, bool inputIsPrimaryKey = false);

/**
This specific overload lets you add a string field for the converter to be able to check/get/set for database reading and writing.
@param inputStringField: A tuple of form: get function, set function, has function (nullptr if field is required), name of the field in the SQL database
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
void addField(const stringField<classType> &inputStringField, bool inputIsPrimaryKey = false);

/**
This specific overload lets you add a repeated int64 field (or a wrapped repeated enum field) for the converter to be able to check/get/set for database reading and writing.
@param inputRepeatedInt64Field: A tuple of form: get (index) function, add function, size function, name of the table for this repeated field in the SQL database, name of the value field in the table, name of foreign key that points to primary table

@throws: This function can throw exceptions
*/
void addField(const repeatedInt64Field<classType> &inputRepeatedInt64Field);

/**
This specific overload lets you add a repeated double field for the converter to be able to check/get/set for database reading and writing.
@param inputRepeatedDoubleField: A tuple of form: get (index) function, add function, size function, name of the table for this repeated field in the SQL database, name of the value field in the table, name of foreign key that points to primary table

@throws: This function can throw exceptions
*/
void addField(const repeatedDoubleField<classType> &inputRepeatedDoubleField);

/**
This specific overload lets you add a repeated string field for the converter to be able to check/get/set for database reading and writing.
@param inputRepeatedStringField: A tuple of form: get (index) function, add function, size function, name of the table for this repeated field in the SQL database, name of the value field in the table, name of foreign key that points to primary table

@throws: This function can throw exceptions
*/
void addField(const repeatedStringField<classType> &inputRepeatedStringField);

/**
This function stores the given instance of the class into the associated SQLite tables.  A primary key field must be defined before this function is used.
@param inputClass: An instance of the class to store in the database
@return: 1 if succcessful and 0 otherwise

@throws: This function can throw exceptions.
*/
int store(classType &inputClass);

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for. 
*/
std::vector<classType> retrieve(const std::vector<::google::protobuf::int64> &inputPrimaryKeys);

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.
*/
std::vector<classType> retrieve(const std::vector<double> &inputPrimaryKeys);

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.
*/
std::vector<classType> retrieve(const std::vector<std::string> &inputPrimaryKeys);


/**
This function prints information about each of the registered fields and their associated value in the class
@param inputClass: An instance of the class to print field values from
*/
void print(classType &inputClass);

/**
This function returns a tuple that indicates the type and value of the primary key used with a given class.
@param inputClass: The instance of the class to retrieve the primary key from
@return: A tuple containing the type of the primary key, the integer value, the double value or the string value

@throws: This function can throw exceptions, particularly if the primary key has not been set 
*/
std::tuple<fieldType, ::google::protobuf::int64, double, std::string> getPrimaryKey(classType &inputClass);

/**
This function returns the name of the field that is defined as the primary key.
@param inputClass: The instance of the class to retrieve the primary key from
@return: The field name of the primary key

@throws: This function can throw exceptions, particularly if the primary key has not been set 
*/
std::string getPrimaryKeyFieldName();

/**
This function returns the integer value that SQLite uses to represent the field type of the primary key.
@return: SQLite integer value which indicates the type

@throws: This function can throw exceptions, particularly if the primary key has not been set 
*/
int getPrimaryKeySQLiteType();

/**
This function regenerates insertPrimaryRowStatement so that it can be used to insert the values of all the optional and required fields into the associated primary row.  It also regenerates the getPrimaryTableQuerySubstring.  This function is typically run each time a new optional or required field is added.

@throws: This function can throw exceptions
*/
void regeneratePrimaryRowStatements();

/**
This function generates all of the prepared statements used to retrieve the values of repeated fields from the SQLite database.
@param inputNumberOfPrimaryKeys: This is how many different primary keys it should return results for (typically in the form value, key form)

@throws: This function can throw exceptions, particular if the primary key has not been set 
*/
void generateRepeatedFieldRetrievalStatements(unsigned int inputNumberOfPrimaryKeys);

sqlite3 *databaseConnection;
std::string primaryTableName;

//Have to initialize these pointers with deleter (rather than default) or they throw an error
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> insertPrimaryRowStatement;

std::vector<std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedInt64InsertionStatements;
std::vector<std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedDoubleInsertionStatements;
std::vector<std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedStringInsertionStatements;

std::vector<std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedInt64RetrievalStatements;
std::vector<std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedDoubleRetrievalStatements;
std::vector<std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedStringRetrievalStatements;

std::string getPrimaryTableQuerySubstring; //This is a substring that is used at the beginning of the primary row query.  It selects with all of the fields in the proper order (same as the vectors)
std::vector<int> primaryRowExpectedTypes; //This is a list of all of the field types expected when a primary row is returned (in the proper order)
std::vector<bool> isNullable; //Indicates if a NULL value is OK for the primary row field or not

std::vector<int64Field<classType> > int64Fields;
std::set<int> int64FieldsPrimaryKeyIndexes; //Indexes of primary key fields in the int64Fields vector

std::vector<doubleField<classType> > doubleFields;
std::set<int> doubleFieldsPrimaryKeyIndexes; //Indexes of primary key fields in the doubleFields vector

std::vector<stringField<classType> > stringFields;
std::set<int> stringFieldsPrimaryKeyIndexes; //Indexes of primary key fields in the stringFields vector

std::vector<repeatedInt64Field<classType> >  repeatedInt64Fields;
std::vector<repeatedDoubleField<classType> > repeatedDoubleFields;
std::vector<repeatedStringField<classType> > repeatedStringFields;
};

//Include header with function definitions so that seperation similar to the normal hpp/cpp file breakdown is possible without breaking the templates
#include "protobufSQLConverterDefinitions.hpp"


//Given singular field:
//get/set/has functions (has = nullptr if required)
//Name in database
//Type should be automatically identified by function pointer types

//Think I am going to restrict it so that there can only be one primary key in the primary table (rather than multicolumn primary keys).  This simplfies syntax for repeated fields considerably and can always be changed later.

//Given repeated field
//size,get(index)/set(index)
//table in database, reference name field name in database
//get, set, size
//std::tuple<std::function<const ::google::protobuf::int64 &(const classType*, int)>, std::function<void (classType*, int, ::google::protobuf::int64)>,  std::function<const int (const classType*)>, std::string, std::string>







}
#endif 
