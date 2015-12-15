
//MACROS to make it easy to pass the information associated with a field by generating tuples with the get/set/has parameters
//USAGE: macroname(className, fieldName, stringOfNameInDatabase) 
//Generates: //stringField<className>(&className::fieldName, static_cast<void (className::*)(const ::std::string& value)>(&className::set_fieldName), &className::has_fieldName, stringOfNameInDatabase)
#define PYLON_GPS_GEN_OPTIONAL_INT64_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) field<INPUT_CLASS>(std::function<const ::google::protobuf::int64 &(const INPUT_CLASS* )>(&INPUT_CLASS::INPUT_FIELD_NAME), std::function<void (INPUT_CLASS*, ::google::protobuf::int64)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), std::function<bool(const INPUT_CLASS*)>(&INPUT_CLASS::has_##INPUT_FIELD_NAME), INPUT_DATABASE_FIELD_STRING, false)


#define PYLON_GPS_GEN_REQUIRED_INT64_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) field<INPUT_CLASS>(std::function<const ::google::protobuf::int64 &(const INPUT_CLASS* )>(&INPUT_CLASS::INPUT_FIELD_NAME), std::function<void (INPUT_CLASS*, ::google::protobuf::int64)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), nullptr, INPUT_DATABASE_FIELD_STRING, false)

#define PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) field<INPUT_CLASS>(std::function<const double &(const INPUT_CLASS*)>(&INPUT_CLASS::INPUT_FIELD_NAME), std::function<void (INPUT_CLASS*, double)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), std::function<bool(const INPUT_CLASS*)>(&INPUT_CLASS::has_##INPUT_FIELD_NAME), INPUT_DATABASE_FIELD_STRING)

#define PYLON_GPS_GEN_REQUIRED_DOUBLE_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) field<INPUT_CLASS>(std::function<const double &(const INPUT_CLASS*)>(&INPUT_CLASS::INPUT_FIELD_NAME), std::function<void (INPUT_CLASS*, double)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), nullptr, INPUT_DATABASE_FIELD_STRING)

#define PYLON_GPS_GEN_OPTIONAL_STRING_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) field<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, static_cast<void (INPUT_CLASS::*)(const ::std::string& value)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), &INPUT_CLASS::has_##INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) 

#define PYLON_GPS_GEN_REQUIRED_STRING_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) field<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, static_cast<void (INPUT_CLASS::*)(const ::std::string& value)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), nullptr, INPUT_DATABASE_FIELD_STRING) 

#define PYLON_GPS_GEN_OPTIONAL_ENUM_FIELD(INPUT_CLASS, INPUT_ENUM_TYPE, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) field<INPUT_CLASS>([](const INPUT_CLASS *inputMessage) { return ((::google::protobuf::int64) inputMessage->INPUT_FIELD_NAME()); }, [](INPUT_CLASS *inputMessage, ::google::protobuf::int64 inputEnumValue) { inputMessage->set_##INPUT_FIELD_NAME((INPUT_ENUM_TYPE) inputEnumValue); }, &INPUT_CLASS::has_##INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING, true)

#define PYLON_GPS_GEN_REQUIRED_ENUM_FIELD(INPUT_CLASS, INPUT_ENUM_TYPE, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) field<INPUT_CLASS>([](const INPUT_CLASS *inputMessage) { return ((::google::protobuf::int64) inputMessage->INPUT_FIELD_NAME()); }, [](INPUT_CLASS *inputMessage, ::google::protobuf::int64 inputEnumValue) { inputMessage->set_##INPUT_FIELD_NAME((INPUT_ENUM_TYPE) inputEnumValue); }, nullptr, INPUT_DATABASE_FIELD_STRING, true)

//Class, fieldname in protobuf, name of table to store in database, name of field in table, name of foreign key field in table
#define PYLON_GPS_GEN_REPEATED_INT64_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING) field<INPUT_CLASS>(std::function< ::google::protobuf::int64 (INPUT_CLASS*, int)>(static_cast<::google::protobuf::int64 (INPUT_CLASS::*)(int inputValue) const>(&INPUT_CLASS::INPUT_FIELD_NAME) ), &INPUT_CLASS::add_##INPUT_FIELD_NAME, &INPUT_CLASS::INPUT_FIELD_NAME##_size, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING, false)

//Class, fieldname in protobuf, name of table to store in database, name of field in table, name of foreign key field in table
#define PYLON_GPS_GEN_REPEATED_DOUBLE_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING) field<INPUT_CLASS>(std::function< double (INPUT_CLASS*, int)>(static_cast<double (INPUT_CLASS::*)(int inputValue) const>(&INPUT_CLASS::INPUT_FIELD_NAME) ), &INPUT_CLASS::add_##INPUT_FIELD_NAME, &INPUT_CLASS::INPUT_FIELD_NAME##_size, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING)

//Class, fieldname in protobuf, name of table to store in database, name of field in table, name of foreign key field in table
#define PYLON_GPS_GEN_REPEATED_STRING_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING) field<INPUT_CLASS>(std::function< const std::string &(INPUT_CLASS*, int)>(static_cast<const std::string &(INPUT_CLASS::*)(int) const>(&INPUT_CLASS::INPUT_FIELD_NAME)), static_cast<void (INPUT_CLASS::*)(const std::string &)>(&INPUT_CLASS::add_##INPUT_FIELD_NAME), &INPUT_CLASS::INPUT_FIELD_NAME##_size, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING)

#define PYLON_GPS_GEN_REPEATED_ENUM_FIELD(INPUT_CLASS, INPUT_ENUM_TYPE, INPUT_FIELD_NAME, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING) field<INPUT_CLASS>(std::function< ::google::protobuf::int64 (INPUT_CLASS*, int)>([](INPUT_CLASS *inputMessage, int inputIndex) { return ((::google::protobuf::int64) inputMessage->INPUT_FIELD_NAME(inputIndex)); }), [](INPUT_CLASS *inputMessage, ::google::protobuf::int64 inputEnumValue) { inputMessage->add_##INPUT_FIELD_NAME((INPUT_ENUM_TYPE) inputEnumValue); }, &INPUT_CLASS::INPUT_FIELD_NAME##_size, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING, true)

/**
This function sets the fieldValue with a int64 value.
@param inputInt64Value: The value to set the object to
*/
fieldValue::fieldValue(::google::protobuf::int64 inputInt64Value)
{
type = INT64_TYPE;
int64Value = inputInt64Value;
}

/**
This function sets the fieldValue with a double value.
@param inputDoubleValue: The value to set the object to
*/
fieldValue::fieldValue(double inputDoubleValue)
{
type = DOUBLE_TYPE;
doubleValue = inputDoubleValue;
}

/**
This function sets the fieldValue with a string value.
@param inputStringValue: The value to set the object to
*/
fieldValue::fieldValue(const std::string &inputStringValue)
{
type = STRING_TYPE;
stringValue = inputStringValue;
}



/**
This constructor overload makes the field a required/optional either a int64 field or a enum field (they are stored the same in the database/field interface).
@param inputGetFunctionPointer: The function pointer to the int64 getter function
@param inputSetFunctionPointer: The function pointer to the int64 setter function
@param inputHasFunctionPointer: The function pointer to the int64 has function (nullptr if required)
@param inputFieldNameInDatabase: The field name to use to store/retrieve this field from a database
@param inputIsEnum: True if the field that is represented is an enum

@throws: This function can throw exceptions
*/
template <class classType> field<classType>::field(std::function<const ::google::protobuf::int64 &(const classType* )> inputGetFunctionPointer, std::function<void (classType*, ::google::protobuf::int64)> inputSetFunctionPointer, std::function<bool(const classType*)> inputHasFunctionPointer, const std::string &inputFieldNameInDatabase, bool inputIsEnum)
{ //int64, so tuple index 0
if(inputGetFunctionPointer == nullptr || inputSetFunctionPointer == nullptr || inputFieldNameInDatabase == "")
{
throw SOMException("GET or Set == nullptr or field name is empty\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
type = INT64_TYPE;
fieldNameInDatabase = inputFieldNameInDatabase;
isEnum = inputIsEnum;

//Set function pointers
std::get<0>(getFunctionPointers) = inputGetFunctionPointer;
std::get<0>(setOrAddFunctionPointers) = inputSetFunctionPointer;
hasFunctionPointer = inputHasFunctionPointer;

printf("Int field made: %s\n", inputFieldNameInDatabase.c_str());
}

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
template <class classType> field<classType>::field(std::function< ::google::protobuf::int64 (classType*, int)> inputGetFunctionPointer, std::function<void (classType*, ::google::protobuf::int64)> inputAddFunctionPointer,  std::function<int (const classType*)> inputSizeFunctionPointer, const std::string & inputAssociatedTableName, const std::string &inputFieldName, const std::string &inputForeignKeyName, bool inputIsEnum)
{//repeated int64, so tuple index 1
if(inputGetFunctionPointer == nullptr || inputAddFunctionPointer == nullptr || inputSizeFunctionPointer == nullptr || inputAssociatedTableName == "" || inputFieldName == "" || inputForeignKeyName == "")
{
throw SOMException("GET or ADD or SIZE == nullptr or a table/field name is empty\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
type = REPEATED_INT64_TYPE;
associatedTableName = inputAssociatedTableName;
fieldNameInTable = inputFieldName;
foreignKeyName = inputForeignKeyName;
isEnum = inputIsEnum;


//Set function pointers
std::get<1>(getFunctionPointers) = inputGetFunctionPointer;
std::get<1>(setOrAddFunctionPointers) = inputAddFunctionPointer;
sizeFunctionPointer = inputSizeFunctionPointer;
}

/**
This constructor overload makes the field a required/optional double field.
@param inputGetFunctionPointer: The function pointer to the double getter function
@param inputSetFunctionPointer: The function pointer to the double setter function
@param inputHasFunctionPointer: The function pointer to the double has function (nullptr if required)
@param inputFieldNameInDatabase: The field name to use to store/retrieve this field from a database

@throws: This function can throw exceptions
*/
template <class classType> field<classType>::field(std::function<const double &(const classType*)> inputGetFunctionPointer, std::function<void (classType*, double)> inputSetFunctionPointer, std::function<bool(const classType*)> inputHasFunctionPointer, const std::string &inputFieldNameInDatabase)
{ //double, so tuple index 2
if(inputGetFunctionPointer == nullptr || inputSetFunctionPointer == nullptr || inputFieldNameInDatabase == "")
{
throw SOMException("GET or Set == nullptr or field name is empty\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
type = DOUBLE_TYPE;
fieldNameInDatabase = inputFieldNameInDatabase;

//Set function pointers
std::get<2>(getFunctionPointers) = inputGetFunctionPointer;
std::get<2>(setOrAddFunctionPointers) = inputSetFunctionPointer;
hasFunctionPointer = inputHasFunctionPointer;
}

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
template <class classType> field<classType>::field(std::function< double (classType*, int)> inputGetFunctionPointer, std::function<void (classType*, double)> inputAddFunctionPointer,  std::function<int (const classType*)> inputSizeFunctionPointer, const std::string &inputAssociatedTableName, const std::string &inputFieldNameInTable, const std::string &inputForeignKeyName)
{//repeated double, so tuple index 3
if(inputGetFunctionPointer == nullptr || inputAddFunctionPointer == nullptr || inputSizeFunctionPointer == nullptr || inputAssociatedTableName == "" || inputFieldNameInTable == "" || inputForeignKeyName == "")
{
throw SOMException("GET or ADD or SIZE == nullptr or a table/field name is empty\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
type = REPEATED_DOUBLE_TYPE;
associatedTableName = inputAssociatedTableName;
fieldNameInTable = inputFieldNameInTable;
foreignKeyName = inputForeignKeyName;


//Set function pointers
std::get<3>(getFunctionPointers) = inputGetFunctionPointer;
std::get<3>(setOrAddFunctionPointers) = inputAddFunctionPointer;
sizeFunctionPointer = inputSizeFunctionPointer;
}

/**
This constructor overload makes the field a required/optional string field.
@param inputGetFunctionPointer: The function pointer to the double getter function
@param inputSetFunctionPointer: The function pointer to the string setter function
@param inputHasFunctionPointer: The function pointer to the string has function (nullptr if required)
@param inputFieldNameInDatabase: The field name to use to store/retrieve this field from a database

@throws: This function can throw exceptions
*/
template <class classType> field<classType>::field(std::function<const std::string &(const classType*)> inputGetFunctionPointer, std::function<void (classType*, const std::string &)> inputSetFunctionPointer, std::function<bool(const classType*)> inputHasFunctionPointer, const std::string &inputFieldNameInDatabase)
{
//string, so tuple index 4
if(inputGetFunctionPointer == nullptr || inputSetFunctionPointer == nullptr || inputFieldNameInDatabase == "")
{
throw SOMException("GET or Set == nullptr or field name is empty\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
type = STRING_TYPE;
fieldNameInDatabase = inputFieldNameInDatabase;

//Set function pointers
std::get<4>(getFunctionPointers) = inputGetFunctionPointer;
std::get<4>(setOrAddFunctionPointers) = inputSetFunctionPointer;
hasFunctionPointer = inputHasFunctionPointer;
}

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
template <class classType> field<classType>::field(std::function< const std::string &(classType*, int)> inputGetFunctionPointer, std::function<void (classType*, const ::std::string&)> inputAddFunctionPointer,  std::function<int (const classType*)> inputSizeFunctionPointer, const std::string &inputAssociatedTableName, const std::string &inputFieldNameInTable, const std::string &inputForeignKeyName)
{//repeated string, so tuple index 5
if(inputGetFunctionPointer == nullptr || inputAddFunctionPointer == nullptr || inputSizeFunctionPointer == nullptr || inputAssociatedTableName == "" || inputFieldNameInTable == "" || inputForeignKeyName == "")
{
throw SOMException("GET or ADD or SIZE == nullptr or a table/field name is empty\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
type = REPEATED_STRING_TYPE;
associatedTableName = inputAssociatedTableName;
fieldNameInTable = inputFieldNameInTable;
foreignKeyName = inputForeignKeyName;


//Set function pointers
std::get<5>(getFunctionPointers) = inputGetFunctionPointer;
std::get<5>(setOrAddFunctionPointers) = inputAddFunctionPointer;
sizeFunctionPointer = inputSizeFunctionPointer;
}
 
/**
This function returns a fieldType which contains the type expected from the field type this object contains.
@param inputClass: The object to get the value from

@throws: This function can throw exceptions
*/
template <class classType> fieldValue field<classType>::get(classType &inputClass)
{
//Return a fieldValue with the appropriate type using the associated member function
if(type == INT64_TYPE)
{ //Tuple index 0
printf("Int\n");
return fieldValue(std::get<0>(getFunctionPointers)(&inputClass));
}
else if(type == DOUBLE_TYPE)
{ //Tuple index 2
printf("Double\n");
return fieldValue(std::get<2>(getFunctionPointers)(&inputClass));
}
else if(type == STRING_TYPE)
{ //Tuple index 4
printf("String\n");
return fieldValue(std::get<4>(getFunctionPointers)(&inputClass));
}

throw SOMException("get_field(void) called on field type " + std::to_string((int) type)+"\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}



/**
This function returns a fieldType which contains the type expected from the field type this object contains.
@param inputClass: The object to get the value from
@param inputIndex: The index of the value in the repeated field

@throws: This function can throw exceptions
*/
template <class classType> fieldValue field<classType>::get(classType &inputClass, int inputIndex)
{
//Return a fieldValue with the appropriate type using the associated member function
if(type == REPEATED_INT64_TYPE)
{ //Tuple index 1
return fieldValue(std::get<1>(getFunctionPointers)(&inputClass, inputIndex));
}
else if(type == REPEATED_DOUBLE_TYPE)
{ //Tuple index 3
return fieldValue(std::get<3>(getFunctionPointers)(&inputClass, inputIndex));
}
else if(type == REPEATED_STRING_TYPE)
{ //Tuple index 5
return fieldValue(std::get<5>(getFunctionPointers)(&inputClass, inputIndex));
}

throw SOMException("get_field(int index) called on field type " + std::to_string((int) type)+"\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

/**
This function sets the associated int64 field in the class if this field is optional or required or adds it if it is a repeated field.
@param inputClass: The object to set the field in
@param inputValue: The value to set the field to

@throws: This function can throw exceptions
*/
template <class classType> void field<classType>::setOrAdd(classType &inputClass, ::google::protobuf::int64 inputValue)
{
//Set the associated field's value
if(type == INT64_TYPE)
{ //Tuple index 0
std::get<0>(setOrAddFunctionPointers)(&inputClass, inputValue);
}
else if(type == REPEATED_INT64_TYPE)
{ //Tuple index 1
std::get<1>(setOrAddFunctionPointers)(&inputClass, inputValue);
}
else
{
throw SOMException("setOrAdd_field(int64 value) called on field type " + std::to_string((int) type)+"\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
}

/**
This function sets the associated double field in the class if this field is optional or required or adds it if it is a repeated field.
@param inputClass: The object to set the field in
@param inputValue: The value to set the field to

@throws: This function can throw exceptions
*/
template <class classType> void field<classType>::setOrAdd(classType &inputClass, double inputValue)
{
//Set the associated field's value
if(type == DOUBLE_TYPE)
{ //Tuple index 2
std::get<2>(setOrAddFunctionPointers)(&inputClass, inputValue);
}
else if(type == REPEATED_DOUBLE_TYPE)
{ //Tuple index 3
std::get<3>(setOrAddFunctionPointers)(&inputClass, inputValue);
}
else
{
throw SOMException("setOrAdd_field(double value) called on field type " + std::to_string((int) type)+"\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
}

/**
This function sets the associated string field in the class if this field is optional or required or adds it if it is a repeated field.
@param inputClass: The object to set the field in
@param inputValue: The value to set the field to

@throws: This function can throw exceptions
*/
template <class classType> void field<classType>::setOrAdd(classType &inputClass, const std::string &inputValue)
{
//Set the associated field's value
if(type == STRING_TYPE)
{ //Tuple index 4
std::get<4>(setOrAddFunctionPointers)(&inputClass, inputValue);
}
else if(type == REPEATED_STRING_TYPE)
{ //Tuple index 5
std::get<5>(setOrAddFunctionPointers)(&inputClass, inputValue);
}
else
{
throw SOMException("setOrAdd_field(string value) called on field type " + std::to_string((int) type)+"\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
}

/**
This function sets the associated field in the class if this field is optional or required or adds it if it is a repeated field.
@param inputClass: The object to set the field in
@param inputValue: The value to set the field to

@throws: This function can throw exceptions
*/
template <class classType> void field<classType>::setOrAdd(classType &inputClass, const fieldValue &inputValue)
{
if(inputValue.type == NULL_TYPE && (hasFunctionPointer == nullptr || sizeFunctionPointer != nullptr))
{
throw SOMException("Tried to set required or repeated field to NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(inputValue.type == INT64_TYPE)
{
SOM_TRY
setOrAdd(inputClass, inputValue.int64Value);
SOM_CATCH("Error setting/adding field\n")
}
else if(inputValue.type == DOUBLE_TYPE)
{
SOM_TRY
setOrAdd(inputClass, inputValue.doubleValue);
SOM_CATCH("Error setting/adding field\n")
}
else if(inputValue.type == STRING_TYPE)
{
SOM_TRY
setOrAdd(inputClass, inputValue.stringValue);
SOM_CATCH("Error setting/adding field\n")
}
else
{
throw SOMException("Unrecognized field value type\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

}


/**
This function returns true if the class has the optional field set.  If the field is required or repeated, an exception will be throw.
@param inputClass: The object to check

@throws: This function can throw exceptions
*/
template <class classType> bool field<classType>::has(classType &inputClass)
{
//Return a bool using the associated member function
if(!isSingularField(type))
{
throw SOMException("get_field(void) called on field type " + std::to_string((int) type)+"\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(hasFunctionPointer == nullptr)
{
throw SOMException("has_field(void) called on required field\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

return hasFunctionPointer(&inputClass);
}

/**
This function returns the number of elements in a repeated field.
@param inputClass: The object to check

@throws: This function can throw exceptions
*/
template <class classType> int field<classType>::size(classType &inputClass)
{
//Return a int using the associated member function
if(!isRepeatedField(type))
{
throw SOMException("field_size(void) called on field type " + std::to_string((int) type)+"\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

return sizeFunctionPointer(&inputClass);
}




/**
This function initializes the converter with the database connection and table to place/retrieve the message objects to/from.
@param inputDatabaseConnection: A pointer to the database connection to use
@param inputPrimaryTableName: The name of the SQL table that has the main row to place objects in/retrieve them from

@throws: This function can throw exceptions
*/
template <class classType> protobufSQLConverter<classType>::protobufSQLConverter(sqlite3 *inputDatabaseConnection, std::string inputPrimaryTableName) :  insertPrimaryRowStatement(nullptr, &sqlite3_finalize), retrievePrimaryRowStatement(nullptr, &sqlite3_finalize), startTransactionStatement(nullptr, &sqlite3_finalize), endTransactionStatement(nullptr, &sqlite3_finalize), rollbackStatement(nullptr, &sqlite3_finalize), deletePrimaryRowStatement(nullptr, &sqlite3_finalize)
{
if(inputDatabaseConnection == nullptr)
{
throw SOMException("Database connection pointer is NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

databaseConnection = inputDatabaseConnection;
primaryTableName = inputPrimaryTableName;



std::string startTransactionString = "BEGIN TRANSACTION;";
SOM_TRY
prepareStatement(startTransactionStatement, startTransactionString);
SOM_CATCH("Error, unable to make start transaction statement\n")

std::string endTransactionString = "END TRANSACTION;";
SOM_TRY
prepareStatement(endTransactionStatement, endTransactionString);
SOM_CATCH("Error, unable to make end transaction statement\n")

std::string rollbackTransactionString = "ROLLBACK;";
SOM_TRY
prepareStatement(rollbackStatement, rollbackTransactionString);
SOM_CATCH("Error, unable to make rollback transaction statement\n")
}

/**
This specific overload lets you add a field (or a wrapped enum field) for the converter to be able to check/get/set for database reading and writing.
@param inputField: A field object to associate with this class
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::addField(const field<classType> &inputField, bool inputIsPrimaryKey)
{
if(inputIsPrimaryKey)
{ //Check if primary key already set
if(primaryKeyIndex >= 0)
{
throw SOMException("Primary key has already been set\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
}

if(isSingularField(inputField.type) )
{
if(requiredOrOptionalFieldNameToFieldsIndex.count(inputField.fieldNameInDatabase) > 0)
{
throw SOMException("Field name has already been used\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
}

fields.push_back(inputField);
int currentFieldIndex = fields.size() - 1; 

if(isSingularField(inputField.type) )
{ //Add to map
requiredOrOptionalFieldNameToFieldsIndex[inputField.fieldNameInDatabase] = currentFieldIndex;
}

if(inputIsPrimaryKey && isSingularField(inputField.type) )
{
primaryKeyIndex = currentFieldIndex;
}

if(inputField.type == INT64_TYPE)
{
int64FieldsIndex.insert(currentFieldIndex);
}
else if(inputField.type == REPEATED_INT64_TYPE)
{
repeatedInt64FieldsIndex.insert(currentFieldIndex);
}
else if(inputField.type == DOUBLE_TYPE)
{
doubleFieldsIndex.insert(currentFieldIndex);
}
else if(inputField.type == REPEATED_DOUBLE_TYPE)
{
repeatedDoubleFieldsIndex.insert(currentFieldIndex);
}
else if(inputField.type == STRING_TYPE)
{
stringFieldsIndex.insert(currentFieldIndex);
}
else if(inputField.type == REPEATED_STRING_TYPE)
{
repeatedStringFieldsIndex.insert(currentFieldIndex);
}
else
{
throw SOMException("Unrecognized field type: " + std::to_string(inputField.type)+ "\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

}


/**
This function generates the table to store and retrieve the associated protobuf objects.  It creates the associated tables according to the field/tables names in the database it has a connection to.

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::createTables()
{
if(primaryKeyIndex < 0)
{
throw SOMException("Primary key has not been set\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Generate sql string to create table
std::string createTableString = "CREATE TABLE " + primaryTableName + " (";

auto fieldTypeToString = [](fieldType inputFieldType) -> std::string 
{
if(inputFieldType == INT64_TYPE)
{
return "integer";
}
else if(inputFieldType == DOUBLE_TYPE)
{
return "real";
}
else if(inputFieldType == STRING_TYPE)
{
return "text";
}
else
{
throw SOMException("Unrecognized field type\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
};

//Add required and optional fields to string
int singularFieldCount = 0;
for(int i=0; i<fields.size(); i++)
{
if(!isSingularField(fields[i].type) )
{
continue; //Skip nonsingular fields
}

if(singularFieldCount != 0)
{
createTableString += ", ";
}

std::string fieldString;
SOM_TRY
fieldString = fields[i].fieldNameInDatabase + " "+ fieldTypeToString(fields[i].type);
SOM_CATCH("Error resolving string type\n")

if(i == primaryKeyIndex)
{
fieldString += " primary key";
}
else if(fields[i].hasFunctionPointer == nullptr)
{
fieldString += " not null";
}

createTableString += fieldString; //Add specific field string to statement string

singularFieldCount++;
}

createTableString += "); ";

//Generate statements to create repeated field tables
std::string primaryKeyTypeString;
SOM_TRY
primaryKeyTypeString = fieldTypeToString(fields[primaryKeyIndex].type) + " not null";
SOM_CATCH("Error resolving string type\n")

for(int i=0; i<fields.size(); i++)
{
if(!isRepeatedField(fields[i].type) )
{
continue; //Skip nonrepeated fields
}

std::string subTableCreationString = " CREATE TABLE " + fields[i].associatedTableName + " (" + fields[i].fieldNameInTable;

SOM_TRY
subTableCreationString += " " + fieldTypeToString(repeatedToSingularType(fields[i].type)) + " not null";
SOM_CATCH("Error resolving string type\n")

subTableCreationString += ", " + fields[i].foreignKeyName + " " + primaryKeyTypeString + ", FOREIGN KEY(" + fields[i].foreignKeyName + ") REFERENCES " + primaryTableName + "(" + fields[primaryKeyIndex].fieldNameInDatabase + ") ON DELETE CASCADE); ";

//Add index to speed up repeated field resolution
subTableCreationString += "CREATE INDEX " + fields[i].associatedTableName + "_index" + " ON " + fields[i].associatedTableName + "(" + fields[i].foreignKeyName + ");";

createTableString+=subTableCreationString;
}

auto stepStatement = [](std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> &inputStatement) 
{
int returnValue = sqlite3_step(inputStatement.get());
sqlite3_reset(inputStatement.get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statement\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
};

SOM_TRY //Start transaction, so either whole table set is written or none of it is
stepStatement(startTransactionStatement);
SOM_CATCH("Error starting transaction\n")

//Make sure transactions are rolled back if this has not been disabled
SOMScopeGuard rollbackGuard([&] () 
{
int returnValue = sqlite3_step(rollbackStatement.get());
sqlite3_reset(rollbackStatement.get()); //Reset so that statement can be used again
});

int returnValue = sqlite3_exec(databaseConnection, createTableString.c_str(), nullptr, nullptr, nullptr);
if(returnValue != SQLITE_OK)
{
throw SOMException("Error executing statement: (" +std::to_string(returnValue) +")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

SOM_TRY //End transaction, since table was written successfully
stepStatement(endTransactionStatement);
SOM_CATCH("Error starting transaction\n")

rollbackGuard.dismiss();

}

/**
This function stores the given instance of the class into the associated SQLite tables.  A primary key field must be defined before this function is used.
@param inputClass: An instance of the class to store in the database

@throws: This function can throw exceptions.
*/
template <class classType> void protobufSQLConverter<classType>::store(classType &inputClass)
{
//Generate prepared statments if they have not already been made
SOM_TRY //Should throw if primary key hasn't been defined
generatePrimaryRowStatements();
generateRepeatedFieldStatements();
SOM_CATCH("Error preparing statements\n") 



//Start transaction, so either whole object is written or none of it is
int returnValue = sqlite3_step(startTransactionStatement.get());
sqlite3_reset(startTransactionStatement.get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statement to start transaction\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Make sure transactions are rolled back if this has not been disabled
SOMScopeGuard rollbackGuard([&] () 
{
int returnValue = sqlite3_step(rollbackStatement.get());
sqlite3_reset(rollbackStatement.get()); //Reset so that statement can be used again
});

//Use prepared statement to insert primary row
returnValue = 0;
int primaryRowIndex = 1;
for(int i=0; i<fields.size(); i++)
{
if(!isSingularField(fields[i].type))
{
continue; //Skip nonsingular fields
} 

bool fieldIsSet = true;

if(fields[i].hasFunctionPointer != nullptr)
{ //Check if field is set if it is optional
SOM_TRY
fieldIsSet = fields[i].has(inputClass);
SOM_CATCH("Error checking if field is set\n")
}

if(fieldIsSet)
{
fieldValue fieldValueBuffer;
printf("Field name: %s\n", fields[i].fieldNameInDatabase.c_str());
SOM_TRY
fieldValueBuffer = (fields[i].get(inputClass));
SOM_CATCH("Error geting field value")

printf("Hello %d\n", i);
if(fieldValueBuffer.type != fields[i].type)
{
throw SOMException("field returned wrong type\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

SOM_TRY
bindFieldToStatement(insertPrimaryRowStatement.get(), primaryRowIndex, fieldValueBuffer);
SOM_CATCH("Error binding field\n")

}
else
{ //Field is not set, so bind NULL
returnValue = sqlite3_bind_null(insertPrimaryRowStatement.get(), primaryRowIndex);
}

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error at field: " + std::to_string(primaryRowIndex) + "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

primaryRowIndex++;
}

//Execute statement
returnValue = sqlite3_step(insertPrimaryRowStatement.get());
sqlite3_reset(insertPrimaryRowStatement.get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statement to store primary row associated with object (" +std::to_string(returnValue)+ ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Use prepared statements to insert each repeated field
for(int i=0; i<fields.size(); i++)
{
if(!isRepeatedField(fields[i].type))
{
continue; //Skip non-repeated fields
} 

int repeatedFieldValueCount = 0;
SOM_TRY
repeatedFieldValueCount = fields[i].size(inputClass);
SOM_CATCH("Error, unable to get repeated field size\n")

//Bind foreign key
//INSERT INTO repeated_field_table_name (field_name, foreign_key) VALUES(?, ?);
fieldValue fieldValueBuffer;
SOM_TRY
fieldValueBuffer = (fields[primaryKeyIndex].get(inputClass));
SOM_CATCH("Error geting repeated field value")

if(fieldValueBuffer.type != fields[primaryKeyIndex].type)
{
throw SOMException("field returned wrong type\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

SOM_TRY
bindFieldToStatement(repeatedFieldIndexToInsertionStatement.at(i).get(), 2, fieldValueBuffer);
SOM_CATCH("Error binding field\n")

//Foreign key has been bound, so bind and insert values
for(int a=0; a<repeatedFieldValueCount; a++)
{
SOM_TRY
fieldValueBuffer = (fields[i].get(inputClass, a));
SOM_CATCH("Error geting field value")

if(fieldValueBuffer.type != repeatedToSingularType(fields[i].type))
{
throw SOMException("field returned wrong type\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

SOM_TRY
bindFieldToStatement(repeatedFieldIndexToInsertionStatement.at(i).get(), 1, fieldValueBuffer);
SOM_CATCH("Error binding field\n")

//Step statement to insert value
returnValue = sqlite3_step(repeatedFieldIndexToInsertionStatement.at(i).get());
sqlite3_reset(repeatedFieldIndexToInsertionStatement.at(i).get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statement to store repeated field (" + std::to_string(returnValue) + ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
} //End a loop

}

//End transaction
returnValue = sqlite3_step(endTransactionStatement.get());
sqlite3_reset(endTransactionStatement.get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statement to end transaction (" +std::to_string(returnValue)+")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
rollbackGuard.dismiss(); //Disable automatic rollback 

}

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for. 

@throws: This function can throw exceptions
*/
template <class classType> std::vector<classType> protobufSQLConverter<classType>::retrieve(const std::vector<::google::protobuf::int64> &inputPrimaryKeys)
{
std::vector<fieldValue> primaryKeys;
for(int i=0; i<inputPrimaryKeys.size(); i++)
{
primaryKeys.push_back(inputPrimaryKeys[i]);
}

SOM_TRY
return retrieve(primaryKeys);
SOM_CATCH("Error retrieving objects\n")
}

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.

@throws: This function can throw exceptions
*/
template <class classType> std::vector<classType> protobufSQLConverter<classType>::retrieve(const std::vector<double> &inputPrimaryKeys)
{
std::vector<fieldValue> primaryKeys;
for(int i=0; i<inputPrimaryKeys.size(); i++)
{
primaryKeys.push_back(inputPrimaryKeys[i]);
}

SOM_TRY
return retrieve(primaryKeys);
SOM_CATCH("Error retrieving objects\n")
}

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.

@throws: This function can throw exceptions
*/
template <class classType> std::vector<classType> protobufSQLConverter<classType>::retrieve(const std::vector<std::string> &inputPrimaryKeys)
{
std::vector<fieldValue> primaryKeys;
for(int i=0; i<inputPrimaryKeys.size(); i++)
{
primaryKeys.push_back(inputPrimaryKeys[i]);
}

SOM_TRY
return retrieve(primaryKeys);
SOM_CATCH("Error retrieving objects\n")
}

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.

@throws: This function can throw exceptions
*/
template <class classType> std::vector<classType> protobufSQLConverter<classType>::retrieve(const std::vector<fieldValue> &inputPrimaryKeys)
{
std::vector<classType> results;
for(int i=0; i<inputPrimaryKeys.size(); i++)
{
classType classBuffer;
SOM_TRY
if(retrieve(inputPrimaryKeys[i], classBuffer) == true)
{ //object retrieved successfully
results.push_back(classBuffer);
}
SOM_CATCH("Error retrieving message")
}

return results;
}

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary key.
@param inputPrimaryKey: The primary key to search for.
@param inputClassBuffer: The buffer to store the retrieved class in
@return: True if the associated class was retrieved successfully (primary key was found) and false otherwise

@throws: This function can throw exceptions
*/
template <class classType> bool protobufSQLConverter<classType>::retrieve(const fieldValue &inputPrimaryKey, classType &inputClassBuffer)
{
//Generate prepared statments if they have not already been made
generatePrimaryRowStatements();
generateRepeatedFieldStatements();

//Retrieve primary row
SOM_TRY //Prepare query
bindFieldToStatement(retrievePrimaryRowStatement.get(), 1, inputPrimaryKey);
SOM_CATCH("Error binding field\n")

//Step query
int stepReturnValue = 0;
stepReturnValue = sqlite3_step(retrievePrimaryRowStatement.get());
SOMScopeGuard primaryRowQueryGuard([&](){sqlite3_reset(retrievePrimaryRowStatement.get());}); //Ensure query is reset when function exits

if(stepReturnValue == SQLITE_DONE)
{ //Entry associated with key not found
return false;
}

if(stepReturnValue != SQLITE_ROW)
{
throw SOMException("Error executing primary row retrieval query\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

int primaryRowPosition = 0;
fieldValue valueBuffer;
for(int i=0; i<fields.size(); i++)
{
if(!isSingularField(fields[i].type))
{
continue; //Skip all nonsingular fields
}
SOM_TRY
valueBuffer = getFieldValueFromStatement(retrievePrimaryRowStatement.get(), primaryRowPosition);
SOM_CATCH("Error retrieving field value\n")

if(valueBuffer.type == NULL_TYPE)
{ //Check if it is possible for the field to be null
if(fields[i].hasFunctionPointer == nullptr)
{
throw SOMException("Required field returned NULL value from database\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
//Don't do anything, so NULL field remains unset
}
else
{
if(valueBuffer.type != fields[i].type)
{
throw SOMException("Field type returned from database does not match template field datatype\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

SOM_TRY //Error setting required or optional field
fields[i].setOrAdd(inputClassBuffer, valueBuffer);
SOM_CATCH("Error setting object value\n")
}

primaryRowPosition++;
}


//Retrieve and set repeated values
for(int i=0; i<fields.size(); i++)
{
if(!isRepeatedField(fields[i].type))
{ //Skip nonrepeated fields
continue;
}

SOM_TRY //Prepare query
bindFieldToStatement(repeatedFieldIndexToRetrievalStatement.at(i).get(), 1, inputPrimaryKey);
SOM_CATCH("Error binding field\n")

//Step query
int stepReturnValue = 0;
SOMScopeGuard repeatedRowQueryGuard([&](){sqlite3_reset(repeatedFieldIndexToRetrievalStatement.at(i).get());}); //Ensure query is reset when function exits
while(true)
{
stepReturnValue = sqlite3_step(repeatedFieldIndexToRetrievalStatement.at(i).get());


if(stepReturnValue == SQLITE_DONE)
{ //All results for this field have been retrieved
break; 
}

if(stepReturnValue != SQLITE_ROW)
{
throw SOMException("Error executing repeated row retrieval query\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

fieldValue valueBuffer;
SOM_TRY
valueBuffer = getFieldValueFromStatement(repeatedFieldIndexToRetrievalStatement.at(i).get(), 0);
SOM_CATCH("Error retrieving field value\n")

if(valueBuffer.type != repeatedToSingularType(fields[i].type))
{
throw SOMException("Field type returned from database does not match template field datatype\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

SOM_TRY //Error setting required or optional field
fields[i].setOrAdd(inputClassBuffer, valueBuffer);
SOM_CATCH("Error setting object value\n")
}

} //End fields for loop



return true;
}

/**
This function updates the value of a required or optional field in the given object to that of the given value.  The field name must be registered and the field value type correct or an exception will be thrown.
@param inputPrimaryKeyValue: The primary key of the entry to update
@param inputFieldName: The name of the field to update
@param inputUpdatedValue: The value to set the field to

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::update(const fieldValue &inputPrimaryKeyValue, const std::string &inputFieldName, const fieldValue &inputUpdatedValue)
{
int index = 0;
SOM_TRY
index = requiredOrOptionalFieldNameToFieldsIndex.at(inputFieldName);
SOM_CATCH("Required or optional field does not exist\n")

if(singularFieldIndexToUpdateStatement.count(index) == 0)
{
throw SOMException("The associated update statement was not found\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

auto stepStatement = [](std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> &inputStatement) 
{
int returnValue = sqlite3_step(inputStatement.get());
sqlite3_reset(inputStatement.get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statement\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
};

//Bind associated fields
SOM_TRY
bindFieldToStatement(singularFieldIndexToUpdateStatement.at(index).get(), 1, inputUpdatedValue);
SOM_CATCH("Error binding updated value to statement\n")

SOM_TRY
bindFieldToStatement(singularFieldIndexToUpdateStatement.at(index).get(), 2, inputPrimaryKeyValue);
SOM_CATCH("Error binding updated value to statement\n")

SOM_TRY
stepStatement(singularFieldIndexToUpdateStatement.at(index));
SOM_CATCH("Error updating field\n")
}

/**
This function deletes any objects which have the given primary keys from the database.
@param inputPrimaryKeys: The primary keys to delete.

@throws: This function can throw exceptions 
*/
template <class classType> void protobufSQLConverter<classType>::deleteObjects(const std::vector<::google::protobuf::int64> &inputPrimaryKeys)
{//Convert to field vector
std::vector<fieldValue> primaryKeys;

for(int i=0; i<inputPrimaryKeys.size(); i++)
{
primaryKeys.push_back(fieldValue(inputPrimaryKeys[i]));
}

SOM_TRY
deleteObjects(primaryKeys);
SOM_CATCH("Error deleting objects\n")
}

/**
This function deletes any objects which have the given primary keys from the database.
@param inputPrimaryKeys: The primary keys to delete.

@throws: This function can throw exceptions 
*/
template <class classType> void protobufSQLConverter<classType>::deleteObjects(const std::vector<double> &inputPrimaryKeys)
{//Convert to field vector
std::vector<fieldValue> primaryKeys;

for(int i=0; i<inputPrimaryKeys.size(); i++)
{
primaryKeys.push_back(fieldValue(inputPrimaryKeys[i]));
}

SOM_TRY
deleteObjects(primaryKeys);
SOM_CATCH("Error deleting objects\n")
}

/**
This function deletes any objects which have the given primary keys from the database.
@param inputPrimaryKeys: The primary keys to delete.

@throws: This function can throw exceptions 
*/
template <class classType> void protobufSQLConverter<classType>::deleteObjects(const std::vector<std::string> &inputPrimaryKeys)
{//Convert to field vector
std::vector<fieldValue> primaryKeys;

for(int i=0; i<inputPrimaryKeys.size(); i++)
{
primaryKeys.push_back(fieldValue(inputPrimaryKeys[i]));
}

SOM_TRY
deleteObjects(primaryKeys);
SOM_CATCH("Error deleting objects\n")
}


/**
This function deletes any objects which have the given primary keys from the database.
@param inputPrimaryKeys: The primary keys to delete.

@throws: This function can throw exceptions 
*/
template <class classType> void protobufSQLConverter<classType>::deleteObjects(const std::vector<fieldValue> &inputPrimaryKeys)
{
if(primaryKeyIndex < 0)
{
throw SOMException("Primary key has not been set\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

for(int i=0; i<inputPrimaryKeys.size(); i++)
{
if(inputPrimaryKeys[i].type != fields[primaryKeyIndex].type)
{
throw SOMException("fieldValue does not have same type as primary key\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

SOM_TRY
bindFieldToStatement(deletePrimaryRowStatement.get(), 1, inputPrimaryKeys[i]);
SOM_CATCH("Error binding field to statement\n")

int returnValue = sqlite3_step(deletePrimaryRowStatement.get());
sqlite3_reset(deletePrimaryRowStatement.get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statement\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

}

/**
This function prints information about each of the registered fields and their associated value in the class
@param inputClass: An instance of the class to print field values from
*/
template <class classType> void protobufSQLConverter<classType>::print(classType &inputClass)
{
//TODO: reimplement this function
}

/**
This function returns the name of the field that is defined as the primary key.
@param inputClass: The instance of the class to retrieve the primary key from
@return: The field name of the primary key

@throws: This function can throw exceptions, particularly if the primary key has not been set 
*/
template <class classType> std::string protobufSQLConverter<classType>::getPrimaryKeyFieldName()
{
if(primaryKeyIndex < 0)
{
throw SOMException("Primary key has not been set\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

return fields[primaryKeyIndex].fieldNameInDatabase;
}

/**
This function returns the integer value that SQLite uses to represent the field type of the primary key.
@return: SQLite integer value which indicates the type

@throws: This function can throw exceptions, particularly if the primary key has not been set 
*/
template <class classType> int protobufSQLConverter<classType>::getPrimaryKeySQLiteType()
{
if(primaryKeyIndex < 0)
{
throw SOMException("Primary key has not been set\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

int type = fieldTypeToSQLITEType(fields[primaryKeyIndex].type);

if(type != SQLITE_NULL)
{
return type;
}

throw SOMException("Primary key field type is unrecognized\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

/**
This function generates the prepared statements for operations with the primary row.  It is typically run whenever store, print or retrieve is called.  If the statement has already been generated, it does nothing.

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::generatePrimaryRowStatements()
{
if(insertPrimaryRowStatement.get() != nullptr && retrievePrimaryRowStatement != nullptr)
{
return; //Statements have already been prepared
}

//Construct deletion statement
std::string deletePrimaryRowString;
SOM_TRY
deletePrimaryRowString = "DELETE FROM " + primaryTableName + " WHERE " + getPrimaryKeyFieldName() + " = ?;";
SOM_CATCH("Error creating deleter statement string\n")

SOM_TRY
prepareStatement(deletePrimaryRowStatement, deletePrimaryRowString);
SOM_CATCH("Error, unable to make delete primary row statement\n")

//Construct insert statement string
int countOfSingularFields = numberOfSingularFields();

std::string sqlString = "INSERT INTO " + primaryTableName +" (";
for(int i=0; i<fields.size(); i++)
{
if(!isSingularField(fields[i].type))
{
continue;
}

if(i != 0)
{
sqlString += ", "; //Add seperator before each except the first
}
sqlString += fields[i].fieldNameInDatabase;
}
sqlString += ") values(";

for(int i=0; i<countOfSingularFields; i++)
{
if(i != 0)
{
sqlString += ", "; //Add seperator before each except the first
}
sqlString += "?";
}

sqlString += ");";

SOM_TRY
prepareStatement(insertPrimaryRowStatement, sqlString);
SOM_CATCH("Error, unable to make insert primary row statement\n")

//Generate the retrieve primary row prepared query
//SELECT fieldName1, fieldName2 FROM primaryRowTableName WHERE primaryKeyName IN (?);
std::string retrievePrimaryRowQueryString = "SELECT ";

//Add all of the field names in the field vector order
for(int i=0; i<fields.size(); i++)
{
if(!isSingularField(fields[i].type) )
{//Skip nonsigular field 
continue;
}

if(i!=0)
{
retrievePrimaryRowQueryString += ", ";
}

retrievePrimaryRowQueryString += fields[i].fieldNameInDatabase;
}

std::string primaryKeyFieldName;
SOM_TRY
primaryKeyFieldName = getPrimaryKeyFieldName();
SOM_CATCH("Error getting primary key field name\n")

retrievePrimaryRowQueryString += " FROM " + primaryTableName + " WHERE " + primaryKeyFieldName + " IN (?);";

SOM_TRY
prepareStatement(retrievePrimaryRowStatement, retrievePrimaryRowQueryString);
SOM_CATCH("Error, unable to make insert primary row statement\n")

//Create singular field updates statements
for(int i=0; i<fields.size(); i++)
{
if(!isSingularField(fields[i].type) )
{//Skip nonsigular field 
continue;
}

//UPDATE table_name SET field_name = value WHERE primary_key = id;
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> updateStatementBuffer(nullptr, &sqlite3_finalize); 
std::string updateString = "UPDATE " + primaryTableName + " SET " + fields[i].fieldNameInDatabase + " = ? WHERE ";


SOM_TRY
updateString += getPrimaryKeyFieldName();
SOM_CATCH("Error getting primary key field name\n")


updateString += " = ?;";

SOM_TRY
prepareStatement(updateStatementBuffer, updateString);
SOM_CATCH("Error, unable to make update primary row statement\n")


//Add to map
singularFieldIndexToUpdateStatement.emplace(i,std::move(updateStatementBuffer));
}

}

/**
This function generates the prepared statements for operations with repeated fields.  It is typically run whenever store, print or retrieve is called.  If the statement has already been generated, it does nothing.

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::generateRepeatedFieldStatements()
{
std::string primaryKey;

SOM_TRY
primaryKey = getPrimaryKeyFieldName();
SOM_CATCH("Error getting primary key field name\n")

int repeatedFieldsCount = numberOfRepeatedFields();

if(repeatedFieldsCount == repeatedFieldIndexToInsertionStatement.size() && repeatedFieldsCount == repeatedFieldIndexToRetrievalStatement.size())
{
return; //Statements have already been generated
}

//Create insertion statement for each repeated field
//INSERT INTO repeated_field_table_name (field_name, foreign_key) VALUES(?, ?);
for(int i=0; i<fields.size(); i++)
{
if(!isRepeatedField(fields[i].type))
{ //Skip nonrepeated fields
continue;
}

std::string insertSQLString = "INSERT INTO " + fields[i].associatedTableName + " (" + fields[i].fieldNameInTable + ", " + fields[i].foreignKeyName +") VALUES(?, ?);";

//Compile statement and add to vector
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> insertStatement(nullptr, &sqlite3_finalize);

int returnValue = 0;
SOM_TRY
sqlite3_stmt *buffer;
returnValue = sqlite3_prepare_v2(databaseConnection, insertSQLString.c_str(), insertSQLString.size(), &buffer, NULL);
insertStatement.reset(buffer);
SOM_CATCH("Error, unable to initialize SQLite prepared statement\n")

if(returnValue != SQLITE_OK)
{
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(returnValue) + "\n");
throw SOMException(errorMessage.c_str(), SQLITE3_ERROR, __FILE__, __LINE__);
}

//Store in vector so that it can be associated with the field
repeatedFieldIndexToInsertionStatement.insert(std::make_pair(i, std::move(insertStatement)));
}

//Generate select statement for each repeated field
// SELECT value_field_name, key_field_name FROM field_table_name WHERE key_field_name IN(?);
for(int i=0; i<fields.size(); i++)
{
if(!isRepeatedField(fields[i].type))
{ //Skip nonrepeated fields
continue;
}

std::string queryString = "SELECT " + fields[i].fieldNameInTable + ", " + fields[i].foreignKeyName + " FROM " + fields[i].associatedTableName + " WHERE " + fields[i].foreignKeyName +" IN(?);";

//Compile statement
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> statementPointer(nullptr, &sqlite3_finalize);

int returnValue = 0;
SOM_TRY
sqlite3_stmt *buffer;
returnValue = sqlite3_prepare_v2(databaseConnection, queryString.c_str(), queryString.size(), &buffer, NULL);
statementPointer.reset(buffer);
SOM_CATCH("Error, unable to initialize SQLite prepared statement\n")

if(returnValue != SQLITE_OK)
{
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(sqlite3_extended_errcode(databaseConnection)) + "\n");
throw SOMException(errorMessage.c_str(), INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Add to map
repeatedFieldIndexToRetrievalStatement.insert(std::make_pair(i, std::move(statementPointer)));
}


}

/*
This function returns the count of the total number of singular fields.
@return: the number of singular fields
*/
template <class classType> int protobufSQLConverter<classType>::numberOfSingularFields()
{
return int64FieldsIndex.size() + doubleFieldsIndex.size() + stringFieldsIndex.size();
}

/*
This function returns the count of the total number of repeated fields.
@return: the number of repeated fields
*/
template <class classType> int protobufSQLConverter<classType>::numberOfRepeatedFields()
{
return repeatedInt64FieldsIndex.size() + repeatedDoubleFieldsIndex.size() + repeatedStringFieldsIndex.size();
}

/**
This function creates a sqlite statement, binds it with the given statement string and assigns it to the given unique_ptr.
@param inputStatement: The unique_ptr to assign statement ownership to
@param inputStatementString: The SQL string to construct the statement from

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::prepareStatement(std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> &inputStatement, const std::string &inputStatementString)
{
int returnValue = 0;
SOM_TRY
sqlite3_stmt *buffer;
returnValue = sqlite3_prepare_v2(databaseConnection, inputStatementString.c_str(), inputStatementString.size(), &buffer, NULL);
inputStatement.reset(buffer);
SOM_CATCH("Error, unable to initialize SQLite prepared statement\n")

if(returnValue != SQLITE_OK)
{
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(sqlite3_extended_errcode(databaseConnection)) + "\n");
throw SOMException(errorMessage.c_str(), INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
}

/*
This member function makes it easier to bind a field to a given statement compactly.
@param inputStatement: The prepared statement to bind the field to
@param inputQueryIndex: The index in the prepared query to bind (starts at 1)
@param inputFieldValue: The field value to bind to the position

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::bindFieldToStatement(sqlite3_stmt *inputStatement, int inputQueryIndex, const fieldValue &inputFieldValue)
{
if(inputStatement == nullptr || inputQueryIndex < 1)
{
throw SOMException("Statement is NULL or query index is less than 1\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

int returnValue = 0;
if(inputFieldValue.type == INT64_TYPE)
{ //Field is set, so bind value
returnValue = sqlite3_bind_int64(inputStatement, inputQueryIndex, inputFieldValue.int64Value);
}
else if(inputFieldValue.type == DOUBLE_TYPE)
{
returnValue = sqlite3_bind_double(inputStatement, inputQueryIndex, inputFieldValue.doubleValue);
}
else if(inputFieldValue.type == STRING_TYPE)
{
returnValue = sqlite3_bind_text(inputStatement, inputQueryIndex, inputFieldValue.stringValue.c_str(), inputFieldValue.stringValue.size(), SQLITE_TRANSIENT);
}
else
{
throw SOMException("Unrecognized field type: "+ std::to_string(inputFieldValue.type) +"\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error occurred (" + std::to_string(returnValue)+ ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

/**
This function retrieves the associated column value as the appropriate type of fieldValue from a given statement.  If the value of the column entry is NULL, then a fieldValue with the type set to NULL is returned.  Index starts at 0.  The results are undefined if it a called with an invalid index.
@param inputStatement: A pointer to the statement to retrieve the value from
@param inputQueryIndex: The index of the value in the prepared statement
@return: The value of the row field or NULL if the field is NULL

@throws: This function can throw exceptions
*/
template <class classType> fieldValue protobufSQLConverter<classType>::getFieldValueFromStatement(sqlite3_stmt *inputStatement, unsigned int inputQueryIndex)
{
if(inputStatement == nullptr)
{
throw SOMException("Null pointer received for statement input\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

int SQLFieldType = sqlite3_column_type(inputStatement, inputQueryIndex);


switch(SQLFieldType)
{
case SQLITE_INTEGER:
SOM_TRY
return fieldValue((::google::protobuf::int64) sqlite3_column_int(inputStatement, inputQueryIndex));
SOM_CATCH("Error setting integer fieldValue")
break;

case SQLITE_FLOAT:
SOM_TRY
return fieldValue(sqlite3_column_double(inputStatement, inputQueryIndex));
SOM_CATCH("Error setting double fieldValue")
break;

case SQLITE_TEXT:
SOM_TRY
return fieldValue(std::string((const char *) sqlite3_column_text(inputStatement, inputQueryIndex), sqlite3_column_bytes(inputStatement, inputQueryIndex)));
SOM_CATCH("Error setting string fieldValue")
break;

case SQLITE_NULL: //Do nothing, is already set to NULL
SOM_TRY
return fieldValue();
SOM_CATCH("Error setting null fieldValue")
break;

default:
throw SOMException("Unrecognized SQLITE field type: "+ std::to_string(SQLFieldType) +"\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
break;
}

return fieldValue();
}

