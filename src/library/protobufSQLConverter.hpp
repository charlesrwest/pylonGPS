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
REPEATED_INT64_TYPE = 20, 
DOUBLE_TYPE = 30,
REPEATED_DOUBLE_TYPE = 40,
STRING_TYPE = 50,
REPEATED_STRING_TYPE = 60,
NULL_TYPE = 70
};

/**
This function indicates if the field type is singular.
@param inputFieldType: The field type to resolve
@return: True if it is singular
*/
inline bool isSingularField(fieldType inputFieldType)
{
return inputFieldType == INT64_TYPE || inputFieldType == DOUBLE_TYPE || inputFieldType == STRING_TYPE;
}

/**
This function indicates if the field type is repeated.
@param inputFieldType: The field type to resolve
@return: True if it is repeated
*/
inline bool isRepeatedField(fieldType inputFieldType)
{
return inputFieldType == REPEATED_INT64_TYPE || inputFieldType == REPEATED_DOUBLE_TYPE || inputFieldType == REPEATED_STRING_TYPE;
}

/**
This function converts a repeated type into a singular type (REPEATED_INT64_TYPE -> INT64_TYPE)
@param inputFieldType: The field type to resolve
@return: The singular of the type (NULL_TYPE if not recognized)
*/
inline fieldType repeatedToSingularType(fieldType inputFieldType)
{
if(inputFieldType == REPEATED_INT64_TYPE)
{
return INT64_TYPE;
}
else if(inputFieldType == REPEATED_DOUBLE_TYPE)
{
return DOUBLE_TYPE;
}
else if(inputFieldType == REPEATED_STRING_TYPE)
{
return STRING_TYPE;
}

return NULL_TYPE;
}

/**
This function resolves the field type to its corresponding SQLITE type as much as possible.
@param inputFieldType: The field type to resolve
@return: the sqlite field type (SQLITE_NULL if unrecognized)  
*/
inline int fieldTypeToSQLITEType(fieldType inputFieldType)
{
if(inputFieldType == INT64_TYPE || inputFieldType == REPEATED_INT64_TYPE)
{
return SQLITE_INTEGER;
}
else if(inputFieldType == DOUBLE_TYPE || inputFieldType == REPEATED_DOUBLE_TYPE)
{
return SQLITE_FLOAT;
}
else if(inputFieldType == STRING_TYPE || inputFieldType == REPEATED_STRING_TYPE)
{
return SQLITE_TEXT;
}

return SQLITE_NULL;
}

/*
This class represents the value of a singular field (int64, double or string).  The type member indicates which type of values it holds (and therefore which member to get).
*/
class fieldValue
{
public:
/*
This function constructs a default (invalid fieldValue).
*/
fieldValue()
{
type = NULL_TYPE;
}

/**
This function sets the fieldValue with a int64 value.
@param inputInt64Value: The value to set the object to
*/
fieldValue(::google::protobuf::int64 inputInt64Value);

/**
This function sets the fieldValue with a double value.
@param inputDoubleValue: The value to set the object to
*/
fieldValue(double inputDoubleValue);

/**
This function sets the fieldValue with a string value.
@param inputStringValue: The value to set the object to
*/
fieldValue(const std::string &inputStringValue);

fieldType type;
::google::protobuf::int64 int64Value;
double doubleValue;
std::string stringValue;
};

/**
This class represents one of the following: a optional/required/repeated 64 bit integer field, a optional/required/repeated enum, a optional/required/repeated double or a optional/required/repeated string.  It contains member named "type" which indicates which type it holds.  Member functions can then be called using the get/set/has/add interfaces in a portable way by passing the type member for overload resolution.  
*/
template <class classType> class field
{
public:
/**
This constructor overload makes the field a required/optional either a int64 field or a enum field (they are stored the same in the database/field interface).
@param inputGetFunctionPointer: The function pointer to the int64 getter function
@param inputSetFunctionPointer: The function pointer to the int64 setter function
@param inputHasFunctionPointer: The function pointer to the int64 has function (nullptr if required)
@param inputFieldNameInDatabase: The field name to use to store/retrieve this field from a database
@param inputIsEnum: True if the field that is represented is an enum

@throws: This function can throw exceptions
*/
field(std::function<const ::google::protobuf::int64 &(const classType* )> inputGetFunctionPointer, std::function<void (classType*, ::google::protobuf::int64)> inputSetFunctionPointer, std::function<bool(const classType*)> inputHasFunctionPointer, const std::string &inputFieldNameInDatabase, bool inputIsEnum = false);

/**
This constructor overload makes the field a repeated either int64 field or enum field (they are stored the same in the database/field interface).
@param inputGetFunctionPointer: The function pointer to the repeated int64 getter function
@param inputAddFunctionPointer: The function pointer to the adder function
@param inputSizeFunctionPointer: The function pointer to the int64 size
@param inputAssociatedTableName: The name of the table to store the repeated field in
@param inputFieldNameInTable: The field name to use to store/retrieve this field from its associated table
@param inputForeignKeyName: The field name used to store/retrieve the foreign key which points to the primary table
@param inputIsEnum: True if the repeated field that is represented contains enums

@throws: This function can throw exceptions
*/
field(std::function< ::google::protobuf::int64 (classType*, int)> inputGetFunctionPointer, std::function<void (classType*, ::google::protobuf::int64)> inputAddFunctionPointer,  std::function<int (const classType*)> inputSizeFunctionPointer, const std::string & inputAssociatedTableName, const std::string &inputFieldName, const std::string &inputForeignKeyName, bool inputIsEnum = false);

/**
This constructor overload makes the field a required/optional double field.
@param inputGetFunctionPointer: The function pointer to the double getter function
@param inputSetFunctionPointer: The function pointer to the double setter function
@param inputHasFunctionPointer: The function pointer to the double has function (nullptr if required)
@param inputFieldNameInDatabase: The field name to use to store/retrieve this field from a database

@throws: This function can throw exceptions
*/
field(std::function<const double &(const classType*)> inputGetFunctionPointer, std::function<void (classType*, double)> inputSetFunctionPointer, std::function<bool(const classType*)> inputHasFunctionPointer, const std::string &inputFieldNameInDatabase);

/**
This constructor overload makes the field a repeated double field.
@param inputGetFunctionPointer: The function pointer to the repeated double getter function
@param inputAddFunctionPointer: The function pointer to the adder function
@param inputSizeFunctionPointer: The function pointer to the double field size
@param inputAssociatedTableName: The name of the table to store the repeated field in
@param inputFieldNameInTable: The field name to use to store/retrieve this field from its associated table
@param inputForeignKeyName: The field name used to store/retrieve the foreign key which points to the primary table

@throws: This function can throw exceptions
*/
field(std::function< double (classType*, int)> inputGetFunctionPointer, std::function<void (classType*, double)> inputAddFunctionPointer,  std::function<int (const classType*)> inputSizeFunctionPointer, const std::string &inputAssociatedTableName, const std::string &inputFieldNameInTable, const std::string &inputForeignKeyName);

/**
This constructor overload makes the field a required/optional string field.
@param inputGetFunctionPointer: The function pointer to the double getter function
@param inputSetFunctionPointer: The function pointer to the string setter function
@param inputHasFunctionPointer: The function pointer to the string has function (nullptr if required)
@param inputFieldNameInDatabase: The field name to use to store/retrieve this field from a database

@throws: This function can throw exceptions
*/
field(std::function<const std::string &(const classType*)> inputGetFunctionPointer, std::function<void (classType*, const std::string &)> inputSetFunctionPointer, std::function<bool(const classType*)> inputHasFunctionPointer, const std::string &inputFieldNameInDatabase);

/**
This constructor overload makes the field a repeated string field.
@param inputGetFunctionPointer: The function pointer to the repeated string getter function
@param inputAddFunctionPointer: The function pointer to the adder function
@param inputSizeFunctionPointer: The function pointer to the string field size
@param inputAssociatedTableName: The name of the table to store the repeated field in
@param inputFieldNameInTable: The field name to use to store/retrieve this field from its associated table
@param inputForeignKeyName: The field name used to store/retrieve the foreign key which points to the primary table

@throws: This function can throw exceptions
*/
field(std::function< const std::string &(classType*, int)> inputGetFunctionPointer, std::function<void (classType*, const ::std::string&)> inputAddFunctionPointer,  std::function<int (const classType*)> inputSizeFunctionPointer, const std::string &inputAssociatedTableName, const std::string &inputFieldNameInTable, const std::string &inputForeignKeyName); 
 
/**
This function returns a fieldType which contains the type expected from the field type this object contains.
@param inputClass: The object to get the value from

@throws: This function can throw exceptions
*/
fieldValue get(classType &inputClass);

/**
This function returns a fieldType which contains the type expected from the field type this object contains.
@param inputClass: The object to get the value from
@param inputIndex: The index of the value in the repeated field

@throws: This function can throw exceptions
*/
fieldValue get(classType &inputClass, int inputIndex);

/**
This function sets the associated int64 field in the class if this field is optional or required or adds it if it is a repeated field.
@param inputClass: The object to set the field in
@param inputValue: The value to set the field to

@throws: This function can throw exceptions
*/
void setOrAdd(classType &inputClass, ::google::protobuf::int64 inputValue);

/**
This function sets the associated double field in the class if this field is optional or required or adds it if it is a repeated field.
@param inputClass: The object to set the field in
@param inputValue: The value to set the field to

@throws: This function can throw exceptions
*/
void setOrAdd(classType &inputClass, double inputValue);

/**
This function sets the associated string field in the class if this field is optional or required or adds it if it is a repeated field.
@param inputClass: The object to set the field in
@param inputValue: The value to set the field to

@throws: This function can throw exceptions
*/
void setOrAdd(classType &inputClass, const std::string &inputValue);

/**
This function sets the associated field in the class if this field is optional or required or adds it if it is a repeated field.
@param inputClass: The object to set the field in
@param inputValue: The value to set the field to

@throws: This function can throw exceptions
*/
void setOrAdd(classType &inputClass, const fieldValue &inputValue);

/**
This function returns true if the class has the optional field set.  If the field is required or repeated, an exception will be throw.
@param inputClass: The object to check

@throws: This function can throw exceptions
*/
bool has(classType &inputClass);

/**
This function returns the number of elements in a repeated field.
@param inputClass: The object to check

@throws: This function can throw exceptions
*/
int size(classType &inputClass);



fieldType type; //Type represented by this object
std::string fieldNameInDatabase;
std::string associatedTableName;
std::string fieldNameInTable;
std::string foreignKeyName;
bool isEnum; //Only set on int64 types 

//Tuples to store get/setOrAdd/has/size pointers (order: int64, repeated int64, double, repeated double, string, repeated string)
std::tuple<std::function<const ::google::protobuf::int64 &(const classType* )>, std::function< ::google::protobuf::int64 (classType*, int)>, std::function<const double &(const classType*)>, std::function< double (classType*, int)>, std::function<const std::string &(const classType*)>, std::function< const std::string &(classType*, int)> > getFunctionPointers;

std::tuple<std::function<void (classType*, ::google::protobuf::int64)>, std::function<void (classType*, ::google::protobuf::int64)>, std::function<void (classType*, double)>, std::function<void (classType*, double)>, std::function<void (classType*, const std::string &)>, std::function<void (classType*, const ::std::string&)> > setOrAddFunctionPointers;

std::function<bool(const classType*)> hasFunctionPointer;
std::function<int (const classType*)> sizeFunctionPointer;
};




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
This specific overload lets you add a field (or a wrapped enum field) for the converter to be able to check/get/set for database reading and writing.
@param inputField: A field object to associate with this class
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
void addField(const field<classType> &inputField, bool inputIsPrimaryKey = false);

/**
This function generates the table to store and retrieve the associated protobuf objects.  It creates the associated tables according to the field/tables names in the database it has a connection to.

@throws: This function can throw exceptions
*/
void createTables();

/**
This function stores the given instance of the class into the associated SQLite tables.  A primary key field must be defined before this function is used.
@param inputClass: An instance of the class to store in the database

@throws: This function can throw exceptions.
*/
void store(classType &inputClass);

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for. 

@throws: This function can throw exceptions
*/
std::vector<classType> retrieve(const std::vector<::google::protobuf::int64> &inputPrimaryKeys);

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.

@throws: This function can throw exceptions
*/
std::vector<classType> retrieve(const std::vector<double> &inputPrimaryKeys);

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.

@throws: This function can throw exceptions
*/
std::vector<classType> retrieve(const std::vector<std::string> &inputPrimaryKeys);

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.

@throws: This function can throw exceptions
*/
std::vector<classType> retrieve(const std::vector<fieldValue> &inputPrimaryKeys);

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary key.
@param inputPrimaryKey: The primary key to search for.
@param inputClassBuffer: The buffer to store the retrieved class in
@return: True if the associated class was retrieved successfully (primary key was found) and false otherwise

@throws: This function can throw exceptions
*/
bool retrieve(const fieldValue &inputPrimaryKey, classType &inputClassBuffer);

/**
This function updates the value of a required or optional field in the given object to that of the given value.  The field name must be registered and the field value type correct or an exception will be thrown.
@param inputPrimaryKeyValue: The primary key of the entry to update
@param inputFieldName: The name of the field to update
@param inputUpdatedValue: The value to set the field to

@throws: This function can throw exceptions
*/
void update(const fieldValue &inputPrimaryKeyValue, const std::string &inputFieldName, const fieldValue &inputUpdatedValue); 

/**
This function deletes any objects which have the given primary keys from the database.
@param inputPrimaryKeys: The primary keys to delete.

@throws: This function can throw exceptions 
*/
void deleteObjects(const std::vector<::google::protobuf::int64> &inputPrimaryKeys);

/**
This function deletes any objects which have the given primary keys from the database.
@param inputPrimaryKeys: The primary keys to delete.

@throws: This function can throw exceptions 
*/
void deleteObjects(const std::vector<double> &inputPrimaryKeys);

/**
This function deletes any objects which have the given primary keys from the database.
@param inputPrimaryKeys: The primary keys to delete.

@throws: This function can throw exceptions 
*/
void deleteObjects(const std::vector<std::string> &inputPrimaryKeys);

/**
This function deletes any objects which have the given primary keys from the database.
@param inputPrimaryKeys: The primary keys to delete.

@throws: This function can throw exceptions 
*/
void deleteObjects(const std::vector<fieldValue> &inputPrimaryKeys);

/**
This function prints information about each of the registered fields and their associated value in the class
@param inputClass: An instance of the class to print field values from
*/
void print(classType &inputClass);

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
This function generates the prepared statements for operations with the primary row.  It is typically run whenever store, print or retrieve is called.  If the statement has already been generated, it does nothing.

@throws: This function can throw exceptions
*/
void generatePrimaryRowStatements();

/**
This function generates the prepared statements for operations with repeated fields.  It is typically run whenever store, print or retrieve is called.  If the statement has already been generated, it does nothing.

@throws: This function can throw exceptions
*/
void generateRepeatedFieldStatements();

/**
This function returns the count of the total number of singular fields.
@return: the number of singular fields
*/
int numberOfSingularFields();

/**
This function returns the count of the total number of repeated fields.
@return: the number of repeated fields
*/
int numberOfRepeatedFields();

/**
This function creates a sqlite statement, binds it with the given statement string and assigns it to the given unique_ptr.
@param inputStatement: The unique_ptr to assign statement ownership to
@param inputStatementString: The SQL string to construct the statement from

@throws: This function can throw exceptions
*/
void prepareStatement(std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> &inputStatement, const std::string &inputStatementString);

/**
This member function makes it easier to bind a field to a given statement compactly.
@param inputStatement: The prepared statement to bind the field to
@param inputQueryIndex: The index in the prepared query to bind (starts at 1)
@param inputFieldValue: The field value to bind to the position

@throws: This function can throw exceptions
*/
void bindFieldToStatement(sqlite3_stmt *inputStatement, int inputQueryIndex, const fieldValue &inputFieldValue);

/**
This function retrieves the associated column value as the appropriate type of fieldValue from a given statement.  If the value of the column entry is NULL, then a fieldValue with the type set to NULL is returned.  Index starts at 0.  The results are undefined if it a called with an invalid index.
@param inputStatement: A pointer to the statement to retrieve the value from
@param inputQueryIndex: The index of the value in the prepared statement
@return: The value of the row field or NULL if the field is NULL

@throws: This function can throw exceptions
*/
fieldValue getFieldValueFromStatement(sqlite3_stmt *inputStatement, unsigned int inputQueryIndex);

sqlite3 *databaseConnection;
std::string primaryTableName;

//Have to initialize these pointers with deleter (rather than default) or they throw an error
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> insertPrimaryRowStatement;
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> retrievePrimaryRowStatement;
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> deletePrimaryRowStatement;
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> startTransactionStatement; //Used to start/stop transactions
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> endTransactionStatement;
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> rollbackStatement;


std::map<int, std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedFieldIndexToInsertionStatement; //A map from the associated "fields" index value for a repeated field to the associated prepared SQL statement

std::map<int, std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedFieldIndexToRetrievalStatement; //A map from the associated "fields" index value for a repeated field to the associated prepared SQL statement

std::map<int, std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > singularFieldIndexToUpdateStatement;

int primaryKeyIndex = -1; //Index of primary key in the fields vector (negative if not yet set)

std::vector<field<classType> > fields; //All fields types are stored in one vector, with the type member allowing resolution of needed functions to call

std::map<std::string, int> requiredOrOptionalFieldNameToFieldsIndex;

std::set<int> int64FieldsIndex; //Indexes of all required/optional int64 fields in the fields vector
std::set<int> repeatedInt64FieldsIndex; //Indexes of all repeated int64 fields in the fields vector
std::set<int> doubleFieldsIndex; //Indexes of all required/optional double fields in the fields vector
std::set<int> repeatedDoubleFieldsIndex; //Indexes of all repeated double fields in the fields vector
std::set<int> stringFieldsIndex; //Indexes of all required/optional string fields in the fields vector
std::set<int> repeatedStringFieldsIndex; //Indexes of all repeated string fields in the fields vector

};

//Include header with function definitions so that seperation similar to the normal hpp/cpp file breakdown is possible without breaking the templates
#include "protobufSQLConverterDefinitions.hpp"

//Think I am going to restrict it so that there can only be one primary key in the primary table (rather than multicolumn primary keys).  This simplfies syntax for repeated fields considerably and can always be changed later.







}
#endif 
