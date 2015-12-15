#include "messageDatabaseDefinition.hpp"

using namespace pylongps;
using namespace google::protobuf;

/**
This function initializes the maps associated with the given object types fields and creates the SQLite tables to store the associated data.
@param inputDatabaseConnection: The connection to the database to use
@param inputMessageDescriptor: The protobuf Descriptor associated with the message
@param inputStringToPreappendToTableNames: What to stick in front of the object template name

@throws: This function can throw exceptions if one of the database operations fail or there is an unrecognized field type
*/
messageDatabaseDefinition::messageDatabaseDefinition(sqlite3 &inputDatabaseConnection, const google::protobuf::Descriptor &inputMessageDescriptor, const std::string &inputStringToPreappendToTableNames) : databaseConnection(inputDatabaseConnection), insertPrimaryRowStatement(nullptr, &sqlite3_finalize), retrievePrimaryRowStatement(nullptr, &sqlite3_finalize), deleteMessageStatement(nullptr, &sqlite3_finalize)
{
//Make sure foreign key checks are one
if(sqlite3_exec(&databaseConnection, "PRAGMA foreign_keys = on;", NULL, NULL, NULL) != SQLITE_OK)
{
throw SOMException("Database error occurred\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

messageDescriptor = &inputMessageDescriptor;

//Resolve full object name 
messageTableName = inputStringToPreappendToTableNames + messageDescriptor->name();

SOM_TRY
setPrimaryKeyFieldIndex();
SOM_CATCH("Error, unable to find primary key in message\n")

SOM_TRY
initializeFieldNumberVectors();
SOM_CATCH("Error, unable to initialize field numbers\n")

SOM_TRY
initializePrimaryRow();
SOM_CATCH("Error, unable to make primary rows\n")

SOM_TRY
initializePrimitiveRepeatedFields();
SOM_CATCH("Error, unable to initialize repeated fields\n")

SOM_TRY
initializeSubMessageFields();
SOM_CATCH("Error, unable to initialize submessage fields\n")
}

/**
This function stores the given message in the database.
@param inputMessageToStore: The message to store in the database

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::store(const google::protobuf::Message &inputMessageToStore)
{
int64_t primaryKey = 0;

SOM_TRY
primaryKey = getIntegerFieldValue(inputMessageToStore, messageDescriptor->field(primaryKeyFieldNumber));
SOM_CATCH("Error, unable to get primary key value\n")

SOM_TRY
insertPrimaryRow(inputMessageToStore);
SOM_CATCH("Error, unable to insert primary row\n")

SOM_TRY
insertRepeatedFieldsInAssociatedTables(inputMessageToStore, primaryKey);
SOM_CATCH("Error, unable to insert repeated fields\n")

SOM_TRY
insertSubMessages(inputMessageToStore, primaryKey);
SOM_CATCH("Error, unable to insert submessages\n")
}

/**
This function stores the given message in the database.
@param inputPrimaryKey: The primary key to search for
@param inputMessageBuffer: The message to store the returned object in

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::retrieve(int64_t inputPrimaryKey, google::protobuf::Message &inputMessageBuffer)
{
SOM_TRY
retrievePrimaryRow(inputPrimaryKey, inputMessageBuffer);
SOM_CATCH("Error, unable to retrieve and bind singular primitive values\n")

SOM_TRY
retrievePrimitiveRepeatedFieldsFromAssociatedTables(inputMessageBuffer, inputPrimaryKey);
SOM_CATCH("Error, unable to retrieve and bind repeated primitive values\n")

SOM_TRY
retrieveSubMessages(inputMessageBuffer, inputPrimaryKey);
SOM_CATCH("Error unable to retrieve submessages\n")
}

/**
This function deletes a message with the given primary key from the database (if present).
@param inputPrimaryKey: The primary key of the message to delete

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::deleteMessage(int64_t inputPrimaryKey)
{
SOM_TRY
bindFieldValueToStatement(*deleteMessageStatement, 1, inputPrimaryKey);
SOM_CATCH("Error binding statement value\n")

SOM_TRY //Delete the message
stepAndResetSQLiteStatement(*deleteMessageStatement);
SOM_CATCH("Error, unable to step/reset SQLite statement\n")
}

/**
This function updates the primary row of the protobuf message.
@param inputPrimaryKey: The primary key to search for
@param inputFieldNumber: The field number to update in the database
@param inputValue: The integer value to assign to the field

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::update(int64_t inputPrimaryKey, uint32_t inputFieldNumber, int64_t inputValue)
{
SOM_TRY
bindFieldValueToStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber), 1, inputValue);
SOM_CATCH("Error, unable to bind value\n")

SOM_TRY
bindFieldValueToStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber), 2, inputPrimaryKey);
SOM_CATCH("Error, unable to bind primary key\n")

SOM_TRY
stepAndResetSQLiteStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber));
SOM_CATCH("Error, unable to step statement\n")
}

/**
This function updates the primary row of the protobuf message.
@param inputPrimaryKey: The primary key to search for
@param inputFieldNumber: The field number to update in the database
@param inputValue: The double value to assign to the field

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::update(int64_t inputPrimaryKey, uint32_t inputFieldNumber, double inputValue)
{
SOM_TRY
bindFieldValueToStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber), 1, inputValue);
SOM_CATCH("Error, unable to bind primary key\n")

SOM_TRY
bindFieldValueToStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber), 2, inputPrimaryKey);
SOM_CATCH("Error, unable to bind primary key\n")

SOM_TRY
stepAndResetSQLiteStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber));
SOM_CATCH("Error, unable to step statement\n")
}

/**
This function updates the primary row of the protobuf message.
@param inputPrimaryKey: The primary key to search for
@param inputFieldNumber: The field number to update in the database
@param inputValue: The string value to assign to the field

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::update(int64_t inputPrimaryKey, uint32_t inputFieldNumber, const std::string &inputValue)
{
SOM_TRY
bindFieldValueToStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber), 1, inputValue);
SOM_CATCH("Error, unable to bind primary key\n")

SOM_TRY
bindFieldValueToStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber), 2, inputPrimaryKey);
SOM_CATCH("Error, unable to bind primary key\n")

SOM_TRY
stepAndResetSQLiteStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber));
SOM_CATCH("Error, unable to step statement\n")
}

/**
This function updates the primary row of the protobuf message with a NULL value.
@param inputPrimaryKey: The primary key to search for
@param inputFieldNumber: The field number to update in the database

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::update(int64_t inputPrimaryKey, uint32_t inputFieldNumber)
{
SOM_TRY
bindFieldValueToStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber), 1);
SOM_CATCH("Error, unable to bind primary key\n")

SOM_TRY
bindFieldValueToStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber), 2, inputPrimaryKey);
SOM_CATCH("Error, unable to bind primary key\n")

SOM_TRY
stepAndResetSQLiteStatement(*singularPrimitiveFieldNumberToUpdateMessageStatement.at(inputFieldNumber));
SOM_CATCH("Error, unable to step statement\n")
}

/**
This function finds the field that holds the primary key for this particular message type.

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::setPrimaryKeyFieldIndex()
{
//Search for a primary field (first integer field)
primaryKeyFieldNumber = -1;
for(int i=0; i<messageDescriptor->field_count(); i++)
{
enum FieldDescriptor::CppType type = messageDescriptor->field(i)->cpp_type();

if(type == FieldDescriptor::CPPTYPE_INT32 || type == FieldDescriptor::CPPTYPE_INT64 || type == FieldDescriptor::CPPTYPE_UINT32 || type == FieldDescriptor::CPPTYPE_UINT64)
{
primaryKeyFieldNumber = i;
break; //Found the field number of the primary key
}
}

if(primaryKeyFieldNumber == -1)
{
throw SOMException("Message class does not have integer field to use for primary key\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
}

/**
This function initializes the vectors which have the field numbers of the different field types.

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::initializeFieldNumberVectors()
{
//Add entries for singular and repeated fields
for(int i=0; i<messageDescriptor->field_count(); i++)
{
switch(messageDescriptor->field(i)->label())
{
case FieldDescriptor::LABEL_OPTIONAL:
case FieldDescriptor::LABEL_REQUIRED:
singularFieldNumbers.push_back(i);
if(messageDescriptor->field(i)->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
{
singularPrimitiveFieldNumbers.push_back(i);
}
else
{
singularMessageFieldNumbers.push_back(i);
}
break;

case FieldDescriptor::LABEL_REPEATED:
repeatedFieldNumbers.push_back(i);
if(messageDescriptor->field(i)->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
{
repeatedPrimitiveFieldNumbers.push_back(i);
}
else
{
repeatedMessageFieldNumbers.push_back(i);
}
break;

default:
throw SOMException("Unrecognized label type\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
break;
}
}
}


/**
This function creates the primary row for the singular fields of this message and creates the associated insertion/retrieval/deletion statements.

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::initializePrimaryRow()
{
std::string primaryKeyFieldName;

for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{
fieldNumberToPrimaryTableName[singularPrimitiveFieldNumbers[i]] = messageTableName;
}

//Generate/execute table creation statement
std::string primaryTableCreationStatementString = "CREATE TABLE " + messageTableName + " (";

for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{
primaryTableCreationStatementString += messageDescriptor->field(singularPrimitiveFieldNumbers[i])->name() + " ";

SOM_TRY
primaryTableCreationStatementString += cppTypeToDatabaseTypeString(messageDescriptor->field(singularPrimitiveFieldNumbers[i])->cpp_type());
SOM_CATCH("Error determining type\n")

if(singularPrimitiveFieldNumbers[i] == primaryKeyFieldNumber)
{
primaryKeyFieldName = messageDescriptor->field(singularPrimitiveFieldNumbers[i])->name();
primaryTableCreationStatementString += " primary key";
}
else if(messageDescriptor->field(singularPrimitiveFieldNumbers[i])->label() == FieldDescriptor::LABEL_REQUIRED)
{
primaryTableCreationStatementString += " not null";
}

if(i!=(singularPrimitiveFieldNumbers.size()-1))
{
primaryTableCreationStatementString += ", ";
}
}
primaryTableCreationStatementString += "); ";

primaryTableCreationStatementString += "CREATE INDEX " + messageTableName + "__index" + " ON " + messageTableName + "(" + messageDescriptor->field(primaryKeyFieldNumber)->name() + ");";

//TODO: Add rollback protections

int returnValue = sqlite3_exec(&databaseConnection, primaryTableCreationStatementString.c_str(), nullptr, nullptr, nullptr);
if(returnValue != SQLITE_OK)
{
throw SOMException("Error executing statement: (" +std::to_string(returnValue) +")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{ //Primary key, update value
std::string SQLUpdateFieldString = "UPDATE " + messageTableName +" SET " + messageDescriptor->field(singularPrimitiveFieldNumbers[i])->name() + " = ? WHERE " + messageDescriptor->field(primaryKeyFieldNumber)->name() +" = ?;";

std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> statementPointerBuffer(nullptr, &sqlite3_finalize);

SOM_TRY
prepareStatement(statementPointerBuffer, SQLUpdateFieldString, databaseConnection);
SOM_CATCH("Error preparing insert primary row statment\n");

singularPrimitiveFieldNumberToUpdateMessageStatement.emplace(singularPrimitiveFieldNumbers[i], std::move(statementPointerBuffer));
}

//Generate/compile primary row insertion statement
std::string singleRowInsertionStatementString = "INSERT INTO " + messageTableName + " (";
std::string valuesSubstring = ") VALUES(";

for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{
singleRowInsertionStatementString += messageDescriptor->field(singularPrimitiveFieldNumbers[i])->name();
valuesSubstring += "?";

if(i!=(singularPrimitiveFieldNumbers.size()-1))
{
singleRowInsertionStatementString += ", ";
valuesSubstring+=", ";
}
}
valuesSubstring += ");";
singleRowInsertionStatementString += valuesSubstring;

//Have to create tables before compiling insert statements
SOM_TRY
prepareStatement(insertPrimaryRowStatement, singleRowInsertionStatementString, databaseConnection);
SOM_CATCH("Error preparing insert primary row statment\n");

//Create primary row retrieval statment
std::string retrievePrimaryRowStatementString = "SELECT * FROM " +  messageTableName + " WHERE " + primaryKeyFieldName + " = ?;";
SOM_TRY
prepareStatement(retrievePrimaryRowStatement, retrievePrimaryRowStatementString, databaseConnection);
SOM_CATCH("Error preparing retrieve primary row statment\n");


//Create deletion statment for primary row (which gets everything else via cascade delete)
std::string deleteMessageString = "DELETE FROM " + messageTableName + " WHERE " + primaryKeyFieldName + " = ?;";
SOM_TRY
prepareStatement(deleteMessageStatement, deleteMessageString, databaseConnection);
SOM_CATCH("Error preparing delete primary row statment\n")
}

/**
This function creates the tables associated with the repeated fields of this message and creates the associated insertion/retrieval/deletion statements.

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::initializePrimitiveRepeatedFields()
{
//Handle repeated primitive fields
for(int i=0; i<repeatedPrimitiveFieldNumbers.size(); i++)
{
//Create table for primitive repeated field
std::string repeatedFieldTableName = messageTableName + "__repeated_" + messageDescriptor->field(repeatedPrimitiveFieldNumbers[i])->name();
std::string createRepeatedPrimitiveFieldTableString = "CREATE TABLE " + messageTableName + "__repeated_" + messageDescriptor->field(repeatedPrimitiveFieldNumbers[i])->name() + " (";

createRepeatedPrimitiveFieldTableString += "repeatedFieldPrimaryKey INTEGER PRIMARY KEY, ";
createRepeatedPrimitiveFieldTableString += "repeatedFieldForeignKey INTEGER NOT NULL, ";
SOM_TRY
createRepeatedPrimitiveFieldTableString += "repeatedFieldValue " + cppTypeToDatabaseTypeString(messageDescriptor->field(repeatedPrimitiveFieldNumbers[i])->cpp_type());
SOM_CATCH("Error determining type\n");

createRepeatedPrimitiveFieldTableString += ", FOREIGN KEY(repeatedFieldForeignKey) REFERENCES " + messageTableName + "(" + messageDescriptor->field(primaryKeyFieldNumber)->name() + ") ON DELETE CASCADE);\n";

createRepeatedPrimitiveFieldTableString += "CREATE INDEX " + repeatedFieldTableName + "__index" + " ON " + repeatedFieldTableName + "(" + "repeatedFieldForeignKey);";

int returnValue = sqlite3_exec(&databaseConnection, createRepeatedPrimitiveFieldTableString.c_str(), nullptr, nullptr, nullptr);
if(returnValue != SQLITE_OK)
{
throw SOMException("Error executing statement: (" +std::to_string(returnValue) +")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Save table name
fieldNumberToPrimaryTableName[repeatedPrimitiveFieldNumbers[i]] = repeatedFieldTableName;

//Make insertion statement
std::string repeatedFieldInsertionStatementString = "INSERT INTO " + repeatedFieldTableName + " (repeatedFieldForeignKey, repeatedFieldValue) VALUES (?, ?);";

std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> statmentPointerBuffer(nullptr, &sqlite3_finalize);

//Have to create tables before compiling insert statements
SOM_TRY
prepareStatement(statmentPointerBuffer, repeatedFieldInsertionStatementString, databaseConnection);
SOM_CATCH("Error preparing insert retrieve repeated field statment\n");

repeatedFieldNumberToInsertionStatements.emplace(repeatedPrimitiveFieldNumbers[i], std::move(statmentPointerBuffer));

//Make retrieval statement
std::string repeatedFieldRetrievalStatementString = "SELECT * FROM " + repeatedFieldTableName + " WHERE repeatedFieldForeignKey = ?;";
SOM_TRY
prepareStatement(statmentPointerBuffer, repeatedFieldRetrievalStatementString, databaseConnection);
SOM_CATCH("Error preparing retrieve repeated field statment\n");
 
repeatedFieldNumberToRetrievalStatement.emplace(repeatedPrimitiveFieldNumbers[i], std::move(statmentPointerBuffer));
}

}

/**
This function creates the tables associated with the fields of this submessage and creates the associated insertion/retrieval/deletion statements.

@throw: This function can throw exceptions 
*/
void messageDatabaseDefinition::initializeSubMessageFields()
{
//Initialize singular submessage definitions
for(int i=0; i < singularMessageFieldNumbers.size(); i++)
{
std::string subMessageTableName = messageTableName + "__" + messageDescriptor->field(singularMessageFieldNumbers[i])->name();

//Save table name
fieldNumberToPrimaryTableName[singularMessageFieldNumbers[i]] = subMessageTableName;

fieldNumberToSubMessageData.emplace(singularMessageFieldNumbers[i], subMessageDatabaseDefinition(databaseConnection, *(messageDescriptor->field(singularMessageFieldNumbers[i])->message_type()), subMessageTableName, messageTableName, messageDescriptor->field(primaryKeyFieldNumber)->name(), true));
}

//Intitialize repeated submessage definitions
for(int i=0; i < repeatedMessageFieldNumbers.size(); i++)
{
std::string subMessageTableName = messageTableName + "_" + messageDescriptor->field(repeatedMessageFieldNumbers[i])->name();

//Save table name
fieldNumberToPrimaryTableName[repeatedMessageFieldNumbers[i]] = subMessageTableName;

fieldNumberToSubMessageData.emplace(repeatedMessageFieldNumbers[i], subMessageDatabaseDefinition(databaseConnection, *(messageDescriptor->field(repeatedMessageFieldNumbers[i])->message_type()), subMessageTableName, messageTableName, messageDescriptor->field(primaryKeyFieldNumber)->name(), false));
}
}

/**
This function is used in the store function to insert the primary row associated with a message.
@param inputMessageToStore: The message to store the primary row of

@throw: This function can throw exceptions 
*/
void messageDatabaseDefinition::insertPrimaryRow(const google::protobuf::Message &inputMessageToStore)
{
//Set fields for the primary row insertion statement
for(int i=0; i < singularPrimitiveFieldNumbers.size(); i++)
{
SOM_TRY
bindFieldValueToStatement(*insertPrimaryRowStatement, i+1, inputMessageToStore, messageDescriptor->field(singularPrimitiveFieldNumbers[i]));
SOM_CATCH("Error binding statement value\n")
}

//Insert the primary row
SOM_TRY
stepAndResetSQLiteStatement(*insertPrimaryRowStatement);
SOM_CATCH("Error, unable to step/reset SQLite statement\n")
}

/**
This function is used in the store function to insert the repeated fields values into their associated tables.
@param inputMessageToStore: The message to store the repeated fields of
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void messageDatabaseDefinition::insertRepeatedFieldsInAssociatedTables(const google::protobuf::Message &inputMessageToStore, int64_t inputForeignKey)
{
const google::protobuf::Reflection *messageReflection = inputMessageToStore.GetReflection();

for(int i=0; i < repeatedPrimitiveFieldNumbers.size(); i++)
{ //For each repeated field
for(int a = 0; a < messageReflection->FieldSize(inputMessageToStore, messageDescriptor->field(repeatedPrimitiveFieldNumbers[i])); a++)
{ //For each element in the repeated field
SOM_TRY
bindFieldValueToStatement(*repeatedFieldNumberToInsertionStatements.at(repeatedPrimitiveFieldNumbers[i]), 1, inputForeignKey);
SOM_CATCH("Error binding statement value\n")

SOM_TRY
bindRepeatedFieldValueToStatement(*repeatedFieldNumberToInsertionStatements.at(repeatedPrimitiveFieldNumbers[i]), 2, inputMessageToStore, messageDescriptor->field(repeatedPrimitiveFieldNumbers[i]), a);
SOM_CATCH("Error binding statement value\n")

//Insert the repeated row value
SOM_TRY
stepAndResetSQLiteStatement(*repeatedFieldNumberToInsertionStatements.at(repeatedPrimitiveFieldNumbers[i]));
SOM_CATCH("Error, unable to step/reset SQLite statement\n")
}
}

}

/**
This function inserts the submessages associated with the given object into their respective tables.
@param inputMessageToStore: The message to store the repeated fields of
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void messageDatabaseDefinition::insertSubMessages(const google::protobuf::Message &inputMessageToStore, int64_t inputForeignKey)
{
const google::protobuf::Reflection *messageReflection = inputMessageToStore.GetReflection();

for(int i=0; i<singularMessageFieldNumbers.size(); i++)
{ //Store singular messages
if(messageReflection->HasField(inputMessageToStore, messageDescriptor->field(singularMessageFieldNumbers[i])) == true)
{
fieldNumberToSubMessageData.at(singularMessageFieldNumbers[i]).store(messageReflection->GetMessage(inputMessageToStore, messageDescriptor->field(singularMessageFieldNumbers[i])), inputForeignKey);
}
}

for(int i=0; i<repeatedMessageFieldNumbers.size(); i++)
{ //Store repeated messages
for(int a = 0; a < messageReflection->FieldSize(inputMessageToStore, messageDescriptor->field(repeatedMessageFieldNumbers[i])); a++)
{
fieldNumberToSubMessageData.at(repeatedMessageFieldNumbers[i]).store(messageReflection->GetRepeatedMessage(inputMessageToStore, messageDescriptor->field(repeatedMessageFieldNumbers[i]), a), inputForeignKey);
}
}
}

/**
This function retrieves the primary row associated with this message
@param inputPrimaryKey: The primary key to search for
@param inputMessageBuffer: The message to store the returned object in

@throw: This function can throw exceptions
*/
void messageDatabaseDefinition::retrievePrimaryRow(int64_t inputPrimaryKey, google::protobuf::Message &inputMessageBuffer)
{
SOM_TRY
bindFieldValueToStatement(*retrievePrimaryRowStatement, 1, inputPrimaryKey);
SOM_CATCH("Error binding primary key to retrieval statement\n")

SOMScopeGuard statementScopeGuard([&](){sqlite3_reset(retrievePrimaryRowStatement.get());});
int returnValue = sqlite3_step(retrievePrimaryRowStatement.get());

if(returnValue != SQLITE_ROW)
{
throw SOMException("Database error occurred (" + std::to_string(returnValue)+ ")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Set fields according to retrieved results
for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{
SOM_TRY
retrieveFieldValueFromStatement(*retrievePrimaryRowStatement, i, inputMessageBuffer, messageDescriptor->field(singularPrimitiveFieldNumbers[i]));
SOM_CATCH("Error, unable to retrieve singular field from database\n")
}

}

/**
This function is used in the store function to insert the repeated fields values into their associated tables.
@param inputMessageBuffer: The message to store the returned object in
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void messageDatabaseDefinition::retrievePrimitiveRepeatedFieldsFromAssociatedTables(google::protobuf::Message &inputMessageBuffer, int64_t inputForeignKey)
{ //Primary key, foreign key, value
for(int i=0; i<repeatedPrimitiveFieldNumbers.size(); i++)
{
SOM_TRY
bindFieldValueToStatement(*repeatedFieldNumberToRetrievalStatement.at(repeatedPrimitiveFieldNumbers[i]), 1, inputForeignKey);
SOM_CATCH("Error binding foreign key to retrieval statement\n")

SOMScopeGuard statementScopeGuard([&](){sqlite3_reset(repeatedFieldNumberToRetrievalStatement.at(repeatedPrimitiveFieldNumbers[i]).get());});

while(sqlite3_step(repeatedFieldNumberToRetrievalStatement.at(repeatedPrimitiveFieldNumbers[i]).get()) == SQLITE_ROW)
{
SOM_TRY
retrieveRepeatedFieldValueFromStatement(*repeatedFieldNumberToRetrievalStatement.at(repeatedPrimitiveFieldNumbers[i]), 2, inputMessageBuffer, messageDescriptor->field(repeatedPrimitiveFieldNumbers[i]));
SOM_CATCH("Error, unable to retrieve singular field from database\n")
}

}

}

/**
This function retrieves the submessages associated with the given message from their respective tables.
@param inputMessageBuffer: The message to place the retrieved submessages into
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void messageDatabaseDefinition::retrieveSubMessages(google::protobuf::Message &inputMessageBuffer, int64_t inputForeignKey)
{
for(int i=0; i<singularMessageFieldNumbers.size(); i++)
{ //Retrieve singular messages
SOM_TRY
fieldNumberToSubMessageData.at(singularMessageFieldNumbers[i]).retrieveSingular(inputMessageBuffer, inputForeignKey, messageDescriptor->field(singularMessageFieldNumbers[i]));
SOM_CATCH("Error, unable to retrieve singular submessage\n")
}

for(int i=0; i<repeatedMessageFieldNumbers.size(); i++)
{ //Retrieve repeated messages
SOM_TRY
fieldNumberToSubMessageData.at(repeatedMessageFieldNumbers[i]).retrieveRepeated(inputMessageBuffer, inputForeignKey, messageDescriptor->field(repeatedMessageFieldNumbers[i]));
SOM_CATCH("Error, unable to retrieve repeated submessages\n")
}

}
