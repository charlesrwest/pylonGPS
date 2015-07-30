
//MACROS to make it easy to pass the information associated with a field by generating tuples with the get/set/has parameters
//USAGE: macroname(className, fieldName, stringOfNameInDatabase) 
//Generates: //stringField<className>(&className::fieldName, static_cast<void (className::*)(const ::std::string& value)>(&className::set_fieldName), &className::has_fieldName, stringOfNameInDatabase)
#define PYLON_GPS_GEN_OPTIONAL_INT64_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) int64Field<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, &INPUT_CLASS::set_##INPUT_FIELD_NAME, &INPUT_CLASS::has_##INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING, false)

#define PYLON_GPS_GEN_REQUIRED_INT64_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) int64Field<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, &INPUT_CLASS::set_##INPUT_FIELD_NAME, nullptr, INPUT_DATABASE_FIELD_STRING, false)

#define PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) doubleField<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, &INPUT_CLASS::set_##INPUT_FIELD_NAME, &INPUT_CLASS::has_##INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING)

#define PYLON_GPS_GEN_REQUIRED_DOUBLE_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) doubleField<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, &INPUT_CLASS::set_##INPUT_FIELD_NAME, nullptr, INPUT_DATABASE_FIELD_STRING)

#define PYLON_GPS_GEN_OPTIONAL_STRING_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) stringField<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, static_cast<void (INPUT_CLASS::*)(const ::std::string& value)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), &INPUT_CLASS::has_##INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) 

#define PYLON_GPS_GEN_REQUIRED_STRING_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) stringField<INPUT_CLASS>(&INPUT_CLASS::INPUT_FIELD_NAME, static_cast<void (INPUT_CLASS::*)(const ::std::string& value)>(&INPUT_CLASS::set_##INPUT_FIELD_NAME), nullptr, INPUT_DATABASE_FIELD_STRING) 

#define PYLON_GPS_GEN_OPTIONAL_ENUM_FIELD(INPUT_CLASS, INPUT_ENUM_TYPE, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) int64Field<INPUT_CLASS>([](const INPUT_CLASS *inputMessage) { return ((::google::protobuf::int64) inputMessage->INPUT_FIELD_NAME()); }, [](INPUT_CLASS *inputMessage, ::google::protobuf::int64 inputEnumValue) { inputMessage->set_##INPUT_FIELD_NAME((INPUT_ENUM_TYPE) inputEnumValue); }, &INPUT_CLASS::has_##INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING, true)

#define PYLON_GPS_GEN_REQUIRED_ENUM_FIELD(INPUT_CLASS, INPUT_ENUM_TYPE, INPUT_FIELD_NAME, INPUT_DATABASE_FIELD_STRING) int64Field<INPUT_CLASS>([](const INPUT_CLASS *inputMessage) { return ((::google::protobuf::int64) inputMessage->INPUT_FIELD_NAME()); }, [](INPUT_CLASS *inputMessage, ::google::protobuf::int64 inputEnumValue) { inputMessage->set_##INPUT_FIELD_NAME((INPUT_ENUM_TYPE) inputEnumValue); }, nullptr, INPUT_DATABASE_FIELD_STRING, true)

//Class, fieldname in protobuf, name of table to store in database, name of field in table, name of foreign key field in table
#define PYLON_GPS_GEN_REPEATED_INT64_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING) repeatedInt64Field<INPUT_CLASS>(static_cast<::google::protobuf::int64 (INPUT_CLASS::*)(int inputValue) const>(&INPUT_CLASS::INPUT_FIELD_NAME), &INPUT_CLASS::add_##INPUT_FIELD_NAME, &INPUT_CLASS::INPUT_FIELD_NAME##_size, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING, false)

//Class, fieldname in protobuf, name of table to store in database, name of field in table, name of foreign key field in table
#define PYLON_GPS_GEN_REPEATED_DOUBLE_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING) repeatedDoubleField<INPUT_CLASS>(static_cast<double (INPUT_CLASS::*)(int inputValue) const>(&INPUT_CLASS::INPUT_FIELD_NAME), &INPUT_CLASS::add_##INPUT_FIELD_NAME, &INPUT_CLASS::INPUT_FIELD_NAME##_size, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING)

//Class, fieldname in protobuf, name of table to store in database, name of field in table, name of foreign key field in table
#define PYLON_GPS_GEN_REPEATED_STRING_FIELD(INPUT_CLASS, INPUT_FIELD_NAME, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING) repeatedStringField<INPUT_CLASS>(static_cast<const std::string &(INPUT_CLASS::*)(int) const>(&INPUT_CLASS::INPUT_FIELD_NAME), static_cast<void (INPUT_CLASS::*)(const std::string &)>(&INPUT_CLASS::add_##INPUT_FIELD_NAME), &INPUT_CLASS::INPUT_FIELD_NAME##_size, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING)

#define PYLON_GPS_GEN_REPEATED_ENUM_FIELD(INPUT_CLASS, INPUT_ENUM_TYPE, INPUT_FIELD_NAME, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING) repeatedInt64Field<INPUT_CLASS>([](INPUT_CLASS *inputMessage, int inputIndex) { return ((::google::protobuf::int64) inputMessage->INPUT_FIELD_NAME(inputIndex)); }, [](INPUT_CLASS *inputMessage, ::google::protobuf::int64 inputEnumValue) { inputMessage->add_##INPUT_FIELD_NAME((INPUT_ENUM_TYPE) inputEnumValue); }, &INPUT_CLASS::INPUT_FIELD_NAME##_size, INPUT_DATABASE_TABLE_STRING, INPUT_DATABASE_FIELD_STRING, INPUT_DATABASE_FOREIGN_KEY_STRING, true)

/**
This function initializes the converter with the database connection and table to place/retrieve the message objects to/from.
@param inputDatabaseConnection: A pointer to the database connection to use
@param inputPrimaryTableName: The name of the SQL table that has the main row to place objects in/retrieve them from

@throws: This function can throw exceptions
*/
template <class classType> protobufSQLConverter<classType>::protobufSQLConverter(sqlite3 *inputDatabaseConnection, std::string inputPrimaryTableName) :  insertPrimaryRowStatement(nullptr, &sqlite3_finalize)
{
if(inputDatabaseConnection == nullptr)
{
throw SOMException("Database connection pointer is NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

databaseConnection = inputDatabaseConnection;
primaryTableName = inputPrimaryTableName;
}

/**
This specific overload lets you add a int64 field (or a wrapped enum field) for the converter to be able to check/get/set for database reading and writing.
@param inputInt64Field: A tuple of form: get function, set function, has function (nullptr if field is required), name of the field in the SQL database
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::addField(const int64Field<classType> &inputInt64Field, bool inputIsPrimaryKey)
{
if(std::get<0>(inputInt64Field) == nullptr || std::get<1>(inputInt64Field) == nullptr)
{ //Get or set functions are invalid
throw SOMException("Get or set methods are NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(inputIsPrimaryKey && ((int64FieldsPrimaryKeyIndexes.size() > 0) && (doubleFieldsPrimaryKeyIndexes.size() > 0) && (stringFieldsPrimaryKeyIndexes.size() > 0)) )
{
throw SOMException("More than one field set to primary key\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

int64Fields.push_back(inputInt64Field);
if(inputIsPrimaryKey)
{
int64FieldsPrimaryKeyIndexes.insert(int64Fields.size() - 1);
}

SOM_TRY
regeneratePrimaryRowStatements();
SOM_CATCH("Error updating primary row prepared SQL statement\n")
}

/**
This specific overload lets you add a double field for the converter to be able to check/get/set for database reading and writing.
@param inputDoubleField: A tuple of form: get function, set function, has function (nullptr if field is required), name of the field in the SQL database
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::addField(const doubleField<classType> &inputDoubleField, bool inputIsPrimaryKey)
{
if(std::get<0>(inputDoubleField) == nullptr || std::get<1>(inputDoubleField) == nullptr)
{ //Get or set functions are invalid
throw SOMException("Get or set methods are NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(inputIsPrimaryKey && ((int64FieldsPrimaryKeyIndexes.size() > 0) && (doubleFieldsPrimaryKeyIndexes.size() > 0) && (stringFieldsPrimaryKeyIndexes.size() > 0)) )
{
throw SOMException("More than one field set to primary key\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

doubleFields.push_back(inputDoubleField);
if(inputIsPrimaryKey)
{
doubleFieldsPrimaryKeyIndexes.insert(doubleFields.size() - 1);
}

SOM_TRY
regeneratePrimaryRowStatements();
SOM_CATCH("Error updating primary row prepared SQL statement\n")
}

/**
This specific overload lets you add a string field for the converter to be able to check/get/set for database reading and writing.
@param inputStringField: A tuple of form: get function, set function, has function (nullptr if field is required), name of the field in the SQL database
@param inputIsPrimaryKey: True if this a primary key for the object 

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::addField(const stringField<classType> &inputStringField, bool inputIsPrimaryKey)
{
if(std::get<0>(inputStringField) == nullptr || std::get<1>(inputStringField) == nullptr)
{ //Get or set functions are invalid
throw SOMException("Get or set methods are NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(inputIsPrimaryKey && ((int64FieldsPrimaryKeyIndexes.size() > 0) && (doubleFieldsPrimaryKeyIndexes.size() > 0) && (stringFieldsPrimaryKeyIndexes.size() > 0)) )
{
throw SOMException("More than one field set to primary key\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

stringFields.push_back(inputStringField);
if(inputIsPrimaryKey)
{
stringFieldsPrimaryKeyIndexes.insert(stringFields.size() - 1);
}

SOM_TRY
regeneratePrimaryRowStatements();
SOM_CATCH("Error updating primary row prepared SQL statement\n")
}

/**
This specific overload lets you add a repeated int64 field (or a wrapped repeated enum field) for the converter to be able to check/get/set for database reading and writing.
@param inputRepeatedInt64Field: A tuple of form: get (index) function, add function, size function, name of the table for this repeated field in the SQL database, name of the value field in the table, name of foreign key that points to primary table

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::addField(const repeatedInt64Field<classType> &inputRepeatedInt64Field)
{
if(std::get<0>(inputRepeatedInt64Field) == nullptr || std::get<1>(inputRepeatedInt64Field) == nullptr || std::get<2>(inputRepeatedInt64Field) == nullptr)
{ //Get, add or size functions are invalid
throw SOMException("Get, add or size methods are NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(std::get<3>(inputRepeatedInt64Field) == "" || std::get<4>(inputRepeatedInt64Field) == "" || std::get<5>(inputRepeatedInt64Field) == "")
{
throw SOMException("Empty database field\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

repeatedInt64Fields.push_back(inputRepeatedInt64Field);

//Initialize and add associated prepared SQL statement for insertion
std::string insertSQLString = "INSERT INTO " + std::get<3>(inputRepeatedInt64Field) + " (" + std::get<4>(inputRepeatedInt64Field) + ", " + std::get<5>(inputRepeatedInt64Field) +") VALUES(?, ?);";

printf("%s\n", insertSQLString.c_str());

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
repeatedInt64InsertionStatements.push_back(std::move(insertStatement));
}

/**
This specific overload lets you add a repeated double field for the converter to be able to check/get/set for database reading and writing.
@param inputRepeatedDoubleField: A tuple of form: get (index) function, add function, size function, name of the table for this repeated field in the SQL database, name of the value field in the table, name of foreign key that points to primary table

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::addField(const repeatedDoubleField<classType> &inputRepeatedDoubleField)
{
if(std::get<0>(inputRepeatedDoubleField) == nullptr || std::get<1>(inputRepeatedDoubleField) == nullptr || std::get<2>(inputRepeatedDoubleField) == nullptr)
{ //Get, add or size functions are invalid
throw SOMException("Get, add or size methods are NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(std::get<3>(inputRepeatedDoubleField) == "" || std::get<4>(inputRepeatedDoubleField) == "" || std::get<5>(inputRepeatedDoubleField) == "")
{
throw SOMException("Empty database field\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

repeatedDoubleFields.push_back(inputRepeatedDoubleField);

//Initialize and add associated prepared SQL statement for insertion
std::string insertSQLString = "INSERT INTO " + std::get<3>(inputRepeatedDoubleField) + " (" + std::get<4>(inputRepeatedDoubleField) + ", " + std::get<5>(inputRepeatedDoubleField) +") values(?, ?);";

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
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(sqlite3_extended_errcode(databaseConnection)) + "\n");
throw SOMException(errorMessage.c_str(), SQLITE3_ERROR, __FILE__, __LINE__);
}

//Store in vector so that it can be associated with the field
repeatedDoubleInsertionStatements.push_back(std::move(insertStatement));
}

/**
This specific overload lets you add a repeated string field for the converter to be able to check/get/set for database reading and writing.
@param inputRepeatedStringField: A tuple of form: get (index) function, add function, size function, name of the table for this repeated field in the SQL database, name of the value field in the table, name of foreign key that points to primary table

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::addField(const repeatedStringField<classType> &inputRepeatedStringField)
{
if(std::get<0>(inputRepeatedStringField) == nullptr || std::get<1>(inputRepeatedStringField) == nullptr || std::get<2>(inputRepeatedStringField) == nullptr)
{ //Get, add or size functions are invalid
throw SOMException("Get, add or size methods are NULL\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(std::get<3>(inputRepeatedStringField) == "" || std::get<4>(inputRepeatedStringField) == "" || std::get<5>(inputRepeatedStringField) == "")
{
throw SOMException("Empty database field\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

repeatedStringFields.push_back(inputRepeatedStringField);

//Initialize and add associated prepared SQL statement for insertion
std::string insertSQLString = "INSERT INTO " + std::get<3>(inputRepeatedStringField) + " (" + std::get<4>(inputRepeatedStringField) + ", " + std::get<5>(inputRepeatedStringField) +") values(?, ?);";

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
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(sqlite3_extended_errcode(databaseConnection)) + "\n");
throw SOMException(errorMessage.c_str(), SQLITE3_ERROR, __FILE__, __LINE__);
}

//Store in vector so that it can be associated with the field
repeatedStringInsertionStatements.push_back(std::move(insertStatement));
}

/**
This function stores the given instance of the class into the associated SQLite tables. A primary key field must be defined before this function is used.
@param inputClass: An instance of the class to store in the database
@return: 1 if succcessful and 0 otherwise

@throws: This function can throw exceptions.
*/
template <class classType> int protobufSQLConverter<classType>::store(classType &inputClass)
{
if(int64FieldsPrimaryKeyIndexes.size() == 0 && doubleFieldsPrimaryKeyIndexes.size() == 0 && stringFieldsPrimaryKeyIndexes.size() == 0)
{
throw SOMException("A primary key has not been specified\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Bind entries for the primary row statement (statement index starts at 1)
int primaryRowIndex = 1;
int returnValue = 0;

//All optional and required integer fields
for(int i=0; i<int64Fields.size(); i++)
{

if(std::get<2>(int64Fields[i]) == nullptr)
{ //Its a required field
returnValue = sqlite3_bind_int64(insertPrimaryRowStatement.get(), primaryRowIndex, std::get<0>(int64Fields[i])(&inputClass));
}
else
{
if(!std::get<2>(int64Fields[i])(&inputClass))
{ //Not set, so NULL value
returnValue = sqlite3_bind_null(insertPrimaryRowStatement.get(), primaryRowIndex);
}
else
{ //A set optional field
returnValue = sqlite3_bind_int64(insertPrimaryRowStatement.get(), primaryRowIndex, std::get<0>(int64Fields[i])(&inputClass));
}
}

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error at field: " + std::to_string(primaryRowIndex) + "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

primaryRowIndex++;
} //End int64Fields for loop

for(int i=0; i<doubleFields.size(); i++)
{

if(std::get<2>(doubleFields[i]) == nullptr)
{ //Its a required field
returnValue = sqlite3_bind_double(insertPrimaryRowStatement.get(), primaryRowIndex, std::get<0>(doubleFields[i])(&inputClass));
}
else
{
if(!std::get<2>(doubleFields[i])(&inputClass))
{ //Not set, so NULL value
returnValue = sqlite3_bind_null(insertPrimaryRowStatement.get(), primaryRowIndex);
}
else
{ //A set optional field
returnValue = sqlite3_bind_double(insertPrimaryRowStatement.get(), primaryRowIndex, std::get<0>(doubleFields[i])(&inputClass));
}
}

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error at field: " + std::to_string(primaryRowIndex) + "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

primaryRowIndex++;
} //End doubleFields for loop

for(int i=0; i<stringFields.size(); i++)
{

if(std::get<2>(stringFields[i]) == nullptr)
{ //Its a required field
returnValue = sqlite3_bind_text(insertPrimaryRowStatement.get(), primaryRowIndex, std::get<0>(stringFields[i])(&inputClass).c_str(), std::get<0>(stringFields[i])(&inputClass).size(), SQLITE_TRANSIENT);
}
else
{
if(!std::get<2>(stringFields[i])(&inputClass))
{ //Not set, so NULL value
returnValue = sqlite3_bind_null(insertPrimaryRowStatement.get(), primaryRowIndex);
}
else
{ //A set optional field
returnValue = sqlite3_bind_text(insertPrimaryRowStatement.get(), primaryRowIndex, std::get<0>(stringFields[i])(&inputClass).c_str(), std::get<0>(stringFields[i])(&inputClass).size(), SQLITE_TRANSIENT);
}
}

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error at field: " + std::to_string(primaryRowIndex) + "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

primaryRowIndex++;
}//End stringFields for loop

//Main row statement prepared, so execute it
returnValue = sqlite3_step(insertPrimaryRowStatement.get());
sqlite3_reset(insertPrimaryRowStatement.get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statement to store primary row associated with object\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Figure out what the primary key value is
std::tuple<fieldType, ::google::protobuf::int64, double, std::string> primaryKey;
SOM_TRY
primaryKey = getPrimaryKey(inputClass);
SOM_CATCH("Error retrieving primary key value\n")

//For each repeated field, insert the associated entry

//Lambda to set key (can throw exceptions)
//Set key value, which stays the same for each value in the repeated field
auto setKeyLambda = [&](sqlite3_stmt *inputPreparedStatement) {
int returnValue = 0;
if(std::get<0>(primaryKey) == INT64_TYPE)
{
returnValue = sqlite3_bind_int64(inputPreparedStatement, 2, std::get<1>(primaryKey));
}
else if(std::get<0>(primaryKey) == DOUBLE_TYPE)
{
returnValue = sqlite3_bind_double(inputPreparedStatement, 2, std::get<2>(primaryKey));
}
else if(std::get<0>(primaryKey) == STRING_TYPE)
{
returnValue = sqlite3_bind_text(inputPreparedStatement, 2, std::get<3>(primaryKey).c_str(), std::get<3>(primaryKey).size(), SQLITE_TRANSIENT);
}

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error at field: " + std::to_string(primaryRowIndex) + "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
};

//Insert repeated int64 fields
for(int i=0; i<repeatedInt64Fields.size() && i<repeatedInt64InsertionStatements.size(); i++)
{
long int numberOfValues = std::get<2>(repeatedInt64Fields[i])(&inputClass);

SOM_TRY
setKeyLambda(repeatedInt64InsertionStatements[i].get());
SOM_CATCH("Error setting key value for repeated int64 statement (" + std::to_string(i) + ")\n")

for(int a=0; a<numberOfValues; a++)
{
returnValue = sqlite3_bind_int64(repeatedInt64InsertionStatements[i].get(), 1, std::get<0>(repeatedInt64Fields[i])(&inputClass, a));

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error at field: " + std::to_string(primaryRowIndex) + "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//value insertion statement prepared, so execute it
returnValue = sqlite3_step(repeatedInt64InsertionStatements[i].get());
sqlite3_reset(repeatedInt64InsertionStatements[i].get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statements to store repeated int64 values\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

}
}

//Insert repeated double fields
for(int i=0; i<repeatedDoubleFields.size() && i<repeatedDoubleInsertionStatements.size(); i++)
{
long int numberOfValues = std::get<2>(repeatedDoubleFields[i])(&inputClass);

SOM_TRY
setKeyLambda(repeatedDoubleInsertionStatements[i].get());
SOM_CATCH("Error setting key value for repeated double statement (" + std::to_string(i) + ")\n")

for(int a=0; a<numberOfValues; a++)
{
returnValue = sqlite3_bind_double(repeatedDoubleInsertionStatements[i].get(), 1, std::get<0>(repeatedDoubleFields[i])(&inputClass, a));

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error at field: " + std::to_string(primaryRowIndex) + "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//value insertion statement prepared, so execute it
returnValue = sqlite3_step(repeatedDoubleInsertionStatements[i].get());
sqlite3_reset(repeatedDoubleInsertionStatements[i].get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statements to store repeated double values\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

}
}

//Insert repeated double fields
for(int i=0; i<repeatedStringFields.size() && i<repeatedStringInsertionStatements.size(); i++)
{
long int numberOfValues = std::get<2>(repeatedStringFields[i])(&inputClass);

SOM_TRY
setKeyLambda(repeatedStringInsertionStatements[i].get());
SOM_CATCH("Error setting key value for repeated double statement (" + std::to_string(i) + ")\n")

for(int a=0; a<numberOfValues; a++)
{
returnValue = sqlite3_bind_text(repeatedStringInsertionStatements[i].get(), 1, std::get<0>(repeatedStringFields[i])(&inputClass, a).c_str(), std::get<0>(repeatedStringFields[i])(&inputClass, a).size(), SQLITE_TRANSIENT);

if(returnValue != SQLITE_OK)
{
throw SOMException("Database error at field: " + std::to_string(primaryRowIndex) + "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//value insertion statement prepared, so execute it
returnValue = sqlite3_step(repeatedStringInsertionStatements[i].get());
sqlite3_reset(repeatedStringInsertionStatements[i].get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE)
{
throw SOMException("Error executing statements to store repeated double values\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

}
}



//TODO: add transactions to ensure either the whole object is added or none is
return 1;
}

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for. 
*/
template <class classType> std::vector<classType> protobufSQLConverter<classType>::retrieve(const std::vector<::google::protobuf::int64> &inputPrimaryKeys)
{
if(int64FieldsPrimaryKeyIndexes.size() != 1)
{
throw SOMException("A integer primary key has not been specified\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(inputPrimaryKeys.size() == 0)
{
return std::vector<classType>(0); //Return empty vector since there are no keys (and therefore no matches)
}

std::map<::google::protobuf::int64, int> primaryKeyToObjectVectorIndex;

//Compose the associated query statement
std::string primaryRowsString = getPrimaryTableQuerySubstring + " WHERE " + std::get<3>(int64Fields[(*int64FieldsPrimaryKeyIndexes.begin())]) + " IN (";

for(int i=0; i<inputPrimaryKeys.size(); i++)
{
if(i!=0)
{
primaryRowsString += ", ";
}

primaryRowsString += "?";
}
primaryRowsString += ");";


std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> getPrimaryRowsStatement(nullptr, &sqlite3_finalize);

int returnValue = 0;
SOM_TRY
sqlite3_stmt *buffer;
returnValue = sqlite3_prepare_v2(databaseConnection, primaryRowsString.c_str(), primaryRowsString.size(), &buffer, NULL);
getPrimaryRowsStatement.reset(buffer);
SOM_CATCH("Error, unable to initialize SQLite prepared statement\n")

if(returnValue != SQLITE_OK)
{
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(sqlite3_extended_errcode(databaseConnection)) + "\n");
throw SOMException(errorMessage.c_str(), SQLITE3_ERROR, __FILE__, __LINE__);
}

//Bind the IDs to search for in the primary expression
for(int i=0; i<inputPrimaryKeys.size(); i++)
{
returnValue = sqlite3_bind_int64(getPrimaryRowsStatement.get(), i+1, inputPrimaryKeys[i]);
}

if(returnValue != SQLITE_OK)
{
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(sqlite3_extended_errcode(databaseConnection)) + "\n");
throw SOMException(errorMessage.c_str(), SQLITE3_ERROR, __FILE__, __LINE__);
}

//Execute query and make resulting messages
//Run the query, processing each row returned until we hit SQLITE_DONE
int stepReturnValue=0;
std::vector<classType> results;
while(true)
{

//Process the statement to get the next row
stepReturnValue = sqlite3_step(getPrimaryRowsStatement.get());

if(stepReturnValue == SQLITE_ROW)
{

//Ensure the row types are acceptable
int rowType = 0;
std::vector<bool> isNull(primaryRowExpectedTypes.size());
for(int i=0; i<primaryRowExpectedTypes.size() && i<isNullable.size() ;i++)
{
rowType = sqlite3_column_type(getPrimaryRowsStatement.get(), i);
isNull[i] = (rowType == SQLITE_NULL); //Indicate if field in database is null
if(!(rowType == primaryRowExpectedTypes[i]) && !((rowType == SQLITE_NULL) && isNullable[i]))
{
//Row is not a value it is suppose to be
throw SOMException("SQLite returned primary row value which is invalid (" + std::to_string(i) + ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

classType messageBuffer;
::google::protobuf::int64 primaryKeyBuffer = 0;

//Retrieve values and set associated fields
int rowPosition = 0; //Where we are in the row parameters
for(int i=0; i<int64Fields.size(); i++)
{
auto integerValue = sqlite3_column_int(getPrimaryRowsStatement.get(), rowPosition);
if(i == (*int64FieldsPrimaryKeyIndexes.begin()))
{ //This is the primary key field
primaryKeyBuffer = integerValue;
}

if(!isNullable[rowPosition] || !isNull[rowPosition])
{//Valid value, so set field accordingly
std::get<1>(int64Fields[i])(&messageBuffer, integerValue);
}
rowPosition++;
}

for(int i=0; i<doubleFields.size(); i++)
{
if(!isNullable[rowPosition] || !isNull[rowPosition])
{//Valid value, so set field accordingly
std::get<1>(doubleFields[i])(&messageBuffer, sqlite3_column_double(getPrimaryRowsStatement.get(), rowPosition));
}
rowPosition++;
}

for(int i=0; i<stringFields.size(); i++)
{
//Get size of string and convert
std::string stringBuffer((const char *) sqlite3_column_text(getPrimaryRowsStatement.get(), rowPosition), sqlite3_column_bytes(getPrimaryRowsStatement.get(), rowPosition));

if(!isNullable[rowPosition] || !isNull[rowPosition])
{//Valid value, so set field accordingly
std::get<1>(stringFields[i])(&messageBuffer, stringBuffer);
}
rowPosition++;
}

results.push_back(messageBuffer);
primaryKeyToObjectVectorIndex[primaryKeyBuffer] = results.size()-1; //Store where the object associated with the key is stored 
}
else if(stepReturnValue == SQLITE_DONE)
{
break;  //Finished query successfully
}
else
{
//Row is not a value it is suppose to be
throw SOMException("Error, unable to step SQL query\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

} //End while loop

SOM_TRY
generateRepeatedFieldRetrievalStatements(inputPrimaryKeys.size());
SOM_CATCH("Error, unable to generate statements to get repeated fields")

int primaryKeySQLITEType = 0;
SOM_TRY
primaryKeySQLITEType = getPrimaryKeySQLiteType();
SOM_CATCH("Unable to retrieve type of primary key\n")

for(int i=0; i<repeatedInt64RetrievalStatements.size() && i<repeatedInt64Fields.size(); i++)
{
//Bind the IDs to search for in the primary expression
for(int a=0; a<inputPrimaryKeys.size(); a++)
{
returnValue = sqlite3_bind_int64(repeatedInt64RetrievalStatements[i].get(), a+1, inputPrimaryKeys[a]);
}

if(returnValue != SQLITE_OK)
{
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(sqlite3_extended_errcode(databaseConnection)) + "\n");
throw SOMException(errorMessage.c_str(), SQLITE3_ERROR, __FILE__, __LINE__);
}

//Run the query, processing each row returned until we hit SQLITE_DONE
int stepReturnValue = 0;
while(true)
{
//Process the statement to get the next row
stepReturnValue = sqlite3_step(repeatedInt64RetrievalStatements[i].get());

if(stepReturnValue == SQLITE_DONE)
{
break;  //Finished query successfully
}
else if(stepReturnValue != SQLITE_ROW)
{//Row is not a value it is suppose to be
throw SOMException("Error, unable to step SQL query\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Ensure the row types are acceptable
int rowType = 0;

std::vector<int> expectedTypes = {SQLITE_INTEGER, primaryKeySQLITEType};
for(int a=0; a<expectedTypes.size(); a++)
{
rowType = sqlite3_column_type(repeatedInt64RetrievalStatements[i].get(), a);
if(rowType != expectedTypes[a])
{
//Row is not a type it is suppose to be
throw SOMException("SQLite returned row value which is invalid (" + std::to_string(a) + ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

//Get key, value
auto keyValue = sqlite3_column_int(repeatedInt64RetrievalStatements[i].get(), 1);
if(primaryKeyToObjectVectorIndex.count(keyValue) != 1)
{
throw SOMException("SQLite returned key value which is invalid (" + std::to_string(i) + ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Add it to the associated object
std::get<1>(repeatedInt64Fields[i])(&results[primaryKeyToObjectVectorIndex[keyValue]], sqlite3_column_int(repeatedInt64RetrievalStatements[i].get(), 0));
}


}

//TODO: Finish here (handle double, string repeated fields)

return results;
}

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.
*/
template <class classType> std::vector<classType> protobufSQLConverter<classType>::retrieve(const std::vector<double> &inputPrimaryKeys)
{
return std::vector<classType>(0);
}

/**
This overload of the function retrieves from the database all protobuf messages that contain one of the given primary keys.
@param inputPrimaryKeys: The primary keys to search for.
*/
template <class classType> std::vector<classType> protobufSQLConverter<classType>::retrieve(const std::vector<std::string> &inputPrimaryKeys)
{
return std::vector<classType>(0);
}

/**
This function prints information about each of the registered fields and their associated value in the class
@param inputClass: An instance of the class to print field values from
*/
template <class classType> void protobufSQLConverter<classType>::print(classType &inputClass)
{
//Handle optional and required fields
for(int i=0; i<int64Fields.size(); i++)
{
if(std::get<4>(int64Fields[i]))
{
continue; //It's an enum
}


if(std::get<2>(int64Fields[i]) == nullptr)
{ //required field
printf("Required Int64 field (%s): %ld\n", std::get<3>(int64Fields[i]).c_str(), std::get<0>(int64Fields[i])(&inputClass));
continue;
}

if(!std::get<2>(int64Fields[i])(&inputClass))
{
printf("Optional Int64 field (%s): NULL VALUE\n", std::get<3>(int64Fields[i]).c_str());
}
else
{
printf("Optional Int64 field (%s): %ld\n", std::get<3>(int64Fields[i]).c_str(), std::get<0>(int64Fields[i])(&inputClass));
}

}

for(int i=0; i<doubleFields.size(); i++)
{
if(std::get<2>(doubleFields[i]) == nullptr)
{ //required field
printf("Required double field (%s): %lf\n", std::get<3>(doubleFields[i]).c_str(), std::get<0>(doubleFields[i])(&inputClass));
continue;
}

if(!std::get<2>(doubleFields[i])(&inputClass))
{
printf("Optional double field (%s): NULL VALUE\n", std::get<3>(doubleFields[i]).c_str());
}
else
{
printf("Optional double field (%s): %lf\n", std::get<3>(doubleFields[i]).c_str(), std::get<0>(doubleFields[i])(&inputClass));
}

}


for(int i=0; i<stringFields.size(); i++)
{
if(std::get<2>(stringFields[i]) == nullptr)
{ //required field
printf("Required string field (%s): %s\n", std::get<3>(stringFields[i]).c_str(), std::get<0>(stringFields[i])(&inputClass).c_str());
continue;
}

if(!std::get<2>(stringFields[i])(&inputClass))
{
printf("Optional string field (%s): NULL VALUE\n", std::get<3>(stringFields[i]).c_str());
}
else
{
printf("Optional string field (%s): %s\n", std::get<3>(stringFields[i]).c_str(), std::get<0>(stringFields[i])(&inputClass).c_str());
}

}

for(int i=0; i<int64Fields.size(); i++)
{
if(!std::get<4>(int64Fields[i]))
{
continue; //It's an int64
}


if(std::get<2>(int64Fields[i]) == nullptr)
{ //required field
printf("Required enum field (%s): %ld\n", std::get<3>(int64Fields[i]).c_str(), std::get<0>(int64Fields[i])(&inputClass));
continue;
}

if(!std::get<2>(int64Fields[i])(&inputClass))
{
printf("Optional enum field (%s): NULL VALUE\n", std::get<3>(int64Fields[i]).c_str());
}
else
{
printf("Optional enum field (%s): %ld\n", std::get<3>(int64Fields[i]).c_str(), std::get<0>(int64Fields[i])(&inputClass));
}

}


//Handle repeated fields
for(int i=0; i<repeatedInt64Fields.size(); i++)
{//For each repeated field
if(std::get<6>(repeatedInt64Fields[i]))
{ //It's an enum
continue;
}

for(int a=0; a < std::get<2>(repeatedInt64Fields[i])(&inputClass); a++)
{//For each value in the repeated field
printf("Repeated int64 field (%s,%s): %ld\n", std::get<3>(repeatedInt64Fields[i]).c_str(), std::get<4>(repeatedInt64Fields[i]).c_str(), std::get<0>(repeatedInt64Fields[i])(&inputClass, a));
}
}

for(int i=0; i<repeatedDoubleFields.size(); i++)
{//For each repeated field
for(int a=0; a < std::get<2>(repeatedDoubleFields[i])(&inputClass); a++)
{//For each value in the repeated field
printf("Repeated double field (%s,%s): %lf\n", std::get<3>(repeatedDoubleFields[i]).c_str(), std::get<4>(repeatedDoubleFields[i]).c_str(), std::get<0>(repeatedDoubleFields[i])(&inputClass, a));
}
}

for(int i=0; i<repeatedStringFields.size(); i++)
{//For each repeated field
for(int a=0; a < std::get<2>(repeatedStringFields[i])(&inputClass); a++)
{//For each value in the repeated field
printf("Repeated string field (%s,%s): %s\n", std::get<3>(repeatedStringFields[i]).c_str(), std::get<4>(repeatedStringFields[i]).c_str(), std::get<0>(repeatedStringFields[i])(&inputClass, a).c_str());
}
}

for(int i=0; i<repeatedInt64Fields.size(); i++)
{//For each repeated field
if(!std::get<6>(repeatedInt64Fields[i]))
{ //It's an int64
continue;
}

for(int a=0; a < std::get<2>(repeatedInt64Fields[i])(&inputClass); a++)
{//For each value in the repeated field
printf("Repeated enum field (%s,%s): %ld\n", std::get<3>(repeatedInt64Fields[i]).c_str(), std::get<4>(repeatedInt64Fields[i]).c_str(), std::get<0>(repeatedInt64Fields[i])(&inputClass, a));
}
}

}

/**
This function returns a tuple that indicates the type and value of the primary key used with a given class.
@param inputClass: The instance of the class to retrieve the primary key from
@return: A tuple containing the type of the primary key, the integer value, the double value or the string value

@throws: This function can throw exceptions, particularly if the primary key has not been set 
*/
template <class classType> std::tuple<fieldType, ::google::protobuf::int64, double, std::string> protobufSQLConverter<classType>::getPrimaryKey(classType &inputClass)
{
std::tuple<fieldType, ::google::protobuf::int64, double, std::string> primaryKey;

if(int64FieldsPrimaryKeyIndexes.size() > 0)
{
std::get<1>(primaryKey) = std::get<0>(int64Fields[*int64FieldsPrimaryKeyIndexes.begin()])(&inputClass);
std::get<0>(primaryKey) = INT64_TYPE;
}
else if(doubleFieldsPrimaryKeyIndexes.size() > 0)
{
std::get<2>(primaryKey) = std::get<0>(doubleFields[*doubleFieldsPrimaryKeyIndexes.begin()])(&inputClass);
std::get<0>(primaryKey) = DOUBLE_TYPE;
}
else if(stringFieldsPrimaryKeyIndexes.size() > 0)
{
std::get<3>(primaryKey) = std::get<0>(stringFields[(*stringFieldsPrimaryKeyIndexes.begin())])(&inputClass);
std::get<0>(primaryKey) = STRING_TYPE;
}
else
{
throw SOMException("A primary key has not been specified\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

return primaryKey;
}

/**
This function returns the name of the field that is defined as the primary key.
@param inputClass: The instance of the class to retrieve the primary key from
@return: The field name of the primary key

@throws: This function can throw exceptions, particularly if the primary key has not been set 
*/
template <class classType> std::string protobufSQLConverter<classType>::getPrimaryKeyFieldName()
{
if(int64FieldsPrimaryKeyIndexes.size() > 0)
{
return std::get<3>(int64Fields[*int64FieldsPrimaryKeyIndexes.begin()]);
}
else if(doubleFieldsPrimaryKeyIndexes.size() > 0)
{
return std::get<3>(doubleFields[*doubleFieldsPrimaryKeyIndexes.begin()]);
}
else if(stringFieldsPrimaryKeyIndexes.size() > 0)
{
return std::get<3>(stringFields[(*stringFieldsPrimaryKeyIndexes.begin())]);
}

throw SOMException("A primary key has not been specified\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

/**
This function returns the integer value that SQLite uses to represent the field type of the primary key.
@return: SQLite integer value which indicates the type

@throws: This function can throw exceptions, particularly if the primary key has not been set 
*/
template <class classType> int protobufSQLConverter<classType>::getPrimaryKeySQLiteType()
{
if(int64FieldsPrimaryKeyIndexes.size() > 0)
{
return SQLITE_INTEGER;
}
else if(doubleFieldsPrimaryKeyIndexes.size() > 0)
{
return SQLITE_FLOAT;
}
else if(stringFieldsPrimaryKeyIndexes.size() > 0)
{
return SQLITE_TEXT;
}

throw SOMException("A primary key has not been specified\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

/**
This function regenerates insertPrimaryRowStatement so that it can be used to insert the values of all the optional and required fields into the associated primary row.  It also regenerates the getPrimaryTableQuerySubstring.  This function is typically run each time a new optional or required field is added.

@throws: This function can throw exceptions
*/
template <class classType> void protobufSQLConverter<classType>::regeneratePrimaryRowStatements()
{
//Construct insert statement string
std::string sqlString = "INSERT INTO " + primaryTableName +" (";
for(int i=0; i<int64Fields.size(); i++)
{
if(i != 0)
{
sqlString += ", "; //Add seperator before each except the first
}
sqlString += std::get<3>(int64Fields[i]);
}

for(int i=0; i<doubleFields.size(); i++)
{
sqlString += ", "; //Add seperator before each except the first
sqlString += std::get<3>(doubleFields[i]);
}

for(int i=0; i<stringFields.size(); i++)
{
sqlString += ", "; //Add seperator before each except the first
sqlString += std::get<3>(stringFields[i]);
}

sqlString += ") values(";
long int numberOfOptionalOrRequiredFields = int64Fields.size()+doubleFields.size()+stringFields.size();

for(int i=0; i<numberOfOptionalOrRequiredFields; i++)
{
if(i != 0)
{
sqlString += ", "; //Add seperator before each except the first
}
sqlString += "?";
}

sqlString += ");";

int returnValue = 0;
SOM_TRY
sqlite3_stmt *buffer;
returnValue = sqlite3_prepare_v2(databaseConnection, sqlString.c_str(), sqlString.size(), &buffer, NULL);
insertPrimaryRowStatement.reset(buffer);
SOM_CATCH("Error, unable to initialize SQLite prepared statement\n")

if(returnValue != SQLITE_OK)
{
std::string errorMessage(std::string("Error preparing parametric sql statement: ") + sqlite3_errstr(sqlite3_extended_errcode(databaseConnection)) + "\n");
throw SOMException(errorMessage.c_str(), INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Regenerate the getPrimaryTableQuerySubstring and primaryRowExpectedTypes
getPrimaryTableQuerySubstring = "SELECT ";
primaryRowExpectedTypes.clear();
isNullable.clear();

//Add all of the field names in the proper order (int64, double, string in same order as the field vectors)
for(int i=0; i<int64Fields.size(); i++)
{
if(i!=0)
{
getPrimaryTableQuerySubstring += ", ";
}

getPrimaryTableQuerySubstring += std::get<3>(int64Fields[i]);
primaryRowExpectedTypes.push_back(SQLITE_INTEGER);
isNullable.push_back(std::get<2>(int64Fields[i]) != nullptr); //Non-null pointer indicates an optional field

}

for(int i=0; i<doubleFields.size(); i++)
{
getPrimaryTableQuerySubstring += ", ";

getPrimaryTableQuerySubstring += std::get<3>(doubleFields[i]);
primaryRowExpectedTypes.push_back(SQLITE_FLOAT);
isNullable.push_back(std::get<2>(doubleFields[i]) != nullptr); //Non-null pointer indicates an optional field
}

for(int i=0; i<stringFields.size(); i++)
{
getPrimaryTableQuerySubstring += ", ";

getPrimaryTableQuerySubstring += std::get<3>(stringFields[i]);
primaryRowExpectedTypes.push_back(SQLITE_TEXT);
isNullable.push_back(std::get<2>(stringFields[i]) != nullptr); //Non-null pointer indicates an optional field
}

getPrimaryTableQuerySubstring += " FROM " + primaryTableName;
}


/**
This function generates all of the prepared statements used to retrieve the values of repeated fields from the SQLite database.
@param inputNumberOfPrimaryKeys: This is how many different primary keys it should return results for (typically in the form value, key form)

@throws: This function can throw exceptions, particular if the primary key has not been set 
*/
template <class classType> void protobufSQLConverter<classType>::generateRepeatedFieldRetrievalStatements(unsigned int inputNumberOfPrimaryKeys)
{
std::string primaryKey;

SOM_TRY
primaryKey = getPrimaryKeyFieldName();
SOM_CATCH("Error getting primary key field name\n")

//Clear any existing queries
repeatedInt64RetrievalStatements.clear();
repeatedDoubleRetrievalStatements.clear();
repeatedStringRetrievalStatements.clear();

//Generate new ones
// SELECT value_field_name, key_field_name FROM field_table_name WHERE IN(primary_key1, primary_key2);
for(int i=0; i<repeatedInt64Fields.size(); i++)
{//4,5, 3
std::string queryString = "SELECT " + std::get<4>(repeatedInt64Fields[i]) + ", " + std::get<5>(repeatedInt64Fields[i]) + " FROM " + std::get<3>(repeatedInt64Fields[i]) + " WHERE " + std::get<5>(repeatedInt64Fields[i]) +" IN(";

for(int a=0; a<inputNumberOfPrimaryKeys; a++)
{
if(a != 0)
{
queryString += ", ";
}
queryString += "?";
}
queryString += ");";

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

//Add to vector
repeatedInt64RetrievalStatements.push_back(std::move(statementPointer));
}

for(int i=0; i<repeatedDoubleFields.size(); i++)
{//4,5, 3
std::string queryString = "SELECT " + std::get<4>(repeatedInt64Fields[i]) + ", " + std::get<5>(repeatedInt64Fields[i]) + " FROM " + std::get<3>(repeatedInt64Fields[i]) + " WHERE " + std::get<5>(repeatedInt64Fields[i]) +" IN(";

for(int a=0; a<inputNumberOfPrimaryKeys; a++)
{
if(a != 0)
{
queryString += ", ";
}
queryString += "?";
}
queryString += ");";

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

//Add to vector
repeatedDoubleRetrievalStatements.push_back(std::move(statementPointer));
}

for(int i=0; i<repeatedStringFields.size(); i++)
{//4,5, 3
std::string queryString = "SELECT " + std::get<4>(repeatedInt64Fields[i]) + ", " + std::get<5>(repeatedInt64Fields[i]) + " FROM " + std::get<3>(repeatedInt64Fields[i]) + " WHERE " + std::get<5>(repeatedInt64Fields[i]) +" IN(";

for(int a=0; a<inputNumberOfPrimaryKeys; a++)
{
if(a != 0)
{
queryString += ", ";
}
queryString += "?";
}
queryString += ");";

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

//Add to vector
repeatedStringRetrievalStatements.push_back(std::move(statementPointer));
}

}







