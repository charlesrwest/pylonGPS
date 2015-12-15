#include "subMessageDatabaseDefinition.hpp"

using namespace pylongps;
using namespace google::protobuf;

/**
This function initializes the maps associated with the given object types fields and creates the SQLite tables to store the associated data.
@param inputDatabaseConnection: The connection to the database to use
@param inputMessageDescriptor: The message template to make the definition for
@param inputTableNameToUse: The name to use for the primary SQLite table associated with these submessages
@param inputForeignTableName: The table name that these submessages reference
@param inputForeignTablePrimaryKeyName: The name of the column in the foreign table that these submessages reference
@param inputIsSingular: True if the field that contains this submessage is a singular field and false if it is repeated

@throws: This function can throw exceptions if one of the database operations fail or there is an unrecognized field type
*/
subMessageDatabaseDefinition::subMessageDatabaseDefinition(sqlite3 &inputDatabaseConnection, const google::protobuf::Descriptor &inputMessageDescriptor, const std::string &inputTableNameToUse, const std::string &inputForeignTableName, const std::string &inputForeignTablePrimaryKeyName, bool inputIsSingular) : databaseConnection(inputDatabaseConnection), insertPrimaryRowStatement(nullptr, &sqlite3_finalize), retrievePrimaryRowsStatement(nullptr, &sqlite3_finalize)
{
isSingular = inputIsSingular;
foreignKeyTableName = inputForeignTableName;
foreignKeyTableFieldName = inputForeignTablePrimaryKeyName;
messageTableName = inputTableNameToUse;

SOM_TRY
initializeFieldNumberVectors(inputMessageDescriptor);
SOM_CATCH("Error, unable to initialize field numbers\n")

SOM_TRY
initializePrimaryRow(inputMessageDescriptor);
SOM_CATCH("Error, unable to make primary rows\n")

SOM_TRY
initializePrimitiveRepeatedFields(inputMessageDescriptor);
SOM_CATCH("Error, unable to initialize repeated fields\n")

SOM_TRY
initializeSubMessageFields(inputMessageDescriptor);
SOM_CATCH("Error, unable to initialize submessage fields\n")
} 

/**
This function stores the given submessage in the database.
@param inputSubMessageToStore: The message to store in the database
@param inputForeignKey: The foreign key which points back to the parent message

@throw: This function can throw exceptions
*/
void subMessageDatabaseDefinition::store(const google::protobuf::Message &inputSubMessageToStore, int64_t inputForeignKey)
{
//Get primary key to use for this message
int64_t subMessageInstancePrimaryKey = subMessagePrimaryKey;
subMessagePrimaryKey++; //Increment so the keys are not reused

SOM_TRY
insertPrimaryRow(inputSubMessageToStore, subMessageInstancePrimaryKey, inputForeignKey);
SOM_CATCH("Error, unable to insert primary row\n")

SOM_TRY
insertRepeatedFieldsInAssociatedTables(inputSubMessageToStore, subMessageInstancePrimaryKey);
SOM_CATCH("Error, unable to insert repeated fields\n")

SOM_TRY
insertSubMessages(inputSubMessageToStore, subMessageInstancePrimaryKey);
SOM_CATCH("Error, unable to insert submessages\n")
}

/**
This function initializes the vectors which have the field numbers of the different field types.
@param inputMessageDescriptor: The message template to make the definition for

@throw: This function can throw exceptions
*/
void subMessageDatabaseDefinition::initializeFieldNumberVectors(const google::protobuf::Descriptor &inputMessageDescriptor)
{
//Add entries for singular and repeated fields
for(int i=0; i<inputMessageDescriptor.field_count(); i++)
{
switch(inputMessageDescriptor.field(i)->label())
{
case FieldDescriptor::LABEL_OPTIONAL:
case FieldDescriptor::LABEL_REQUIRED:
singularFieldNumbers.push_back(i);
if(inputMessageDescriptor.field(i)->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
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
if(inputMessageDescriptor.field(i)->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
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
@param inputMessageDescriptor: The message template to make the definition for

@throw: This function can throw exceptions
*/
void subMessageDatabaseDefinition::initializePrimaryRow(const google::protobuf::Descriptor &inputMessageDescriptor)
{
for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{
fieldNumberToPrimaryTableName[singularPrimitiveFieldNumbers[i]] = messageTableName;
}

//Generate/execute table creation statement
std::string primaryTableCreationStatementString = "CREATE TABLE " + messageTableName + " (primaryKey INTEGER PRIMARY KEY, foreignKey INTEGER";

if(isSingular)
{
primaryTableCreationStatementString += " UNIQUE"; //Prevent multiple entries from referencing the same base object.
}

if(singularPrimitiveFieldNumbers.size() > 0)
{
primaryTableCreationStatementString += ", ";
}

for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{
primaryTableCreationStatementString += inputMessageDescriptor.field(singularPrimitiveFieldNumbers[i])->name() + " ";

SOM_TRY
primaryTableCreationStatementString += cppTypeToDatabaseTypeString(inputMessageDescriptor.field(singularPrimitiveFieldNumbers[i])->cpp_type());
SOM_CATCH("Error determining type\n")

if(inputMessageDescriptor.field(singularPrimitiveFieldNumbers[i])->label() == FieldDescriptor::LABEL_REQUIRED)
{
primaryTableCreationStatementString += " not null";
}

if(i!=(singularPrimitiveFieldNumbers.size()-1))
{
primaryTableCreationStatementString += ", ";
}
}
primaryTableCreationStatementString += ", FOREIGN KEY(foreignKey) REFERENCES " + foreignKeyTableName + " (" + foreignKeyTableFieldName + ") ON DELETE CASCADE); ";

primaryTableCreationStatementString += "CREATE INDEX " + messageTableName + "__index" + " ON " + messageTableName + "(foreignKey);";

//TODO: Add rollback protections

int returnValue = sqlite3_exec(&databaseConnection, primaryTableCreationStatementString.c_str(), nullptr, nullptr, nullptr);
if(returnValue != SQLITE_OK)
{
throw SOMException("Error executing statement: (" +std::to_string(returnValue) +")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}


//Generate/compile primary row insertion statement
std::string singleRowInsertionStatementString = "INSERT INTO " + messageTableName + " (primaryKey, foreignKey";
std::string valuesSubstring = ") VALUES(?, ?"; //One for primary key and one for foreign key

if(singularPrimitiveFieldNumbers.size() > 0)
{
singleRowInsertionStatementString+=", ";
valuesSubstring += ", ";
}

for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{
singleRowInsertionStatementString += inputMessageDescriptor.field(singularPrimitiveFieldNumbers[i])->name();
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


std::string retrievePrimaryRowsStatementString = "SELECT * FROM " + messageTableName + " WHERE foreignKey = ?;";
SOM_TRY
prepareStatement(retrievePrimaryRowsStatement, retrievePrimaryRowsStatementString, databaseConnection);
SOM_CATCH("Error preparing retrieve primary row statment\n");
}

/**
This function creates the tables associated with the repeated fields of this message and creates the associated insertion/retrieval/deletion statements.
@param inputMessageDescriptor: The message template to make the definition for

@throw: This function can throw exceptions
*/
void subMessageDatabaseDefinition::initializePrimitiveRepeatedFields(const google::protobuf::Descriptor &inputMessageDescriptor)
{
//Handle repeated primitive fields
for(int i=0; i<repeatedPrimitiveFieldNumbers.size(); i++)
{
//Create table for primitive repeated field
std::string repeatedFieldTableName = messageTableName + "__repeated_" + inputMessageDescriptor.field(repeatedPrimitiveFieldNumbers[i])->name();
std::string createRepeatedPrimitiveFieldTableString = "CREATE TABLE " + messageTableName + "__repeated_" + inputMessageDescriptor.field(repeatedPrimitiveFieldNumbers[i])->name() + " (";

createRepeatedPrimitiveFieldTableString += "repeatedFieldPrimaryKey INTEGER PRIMARY KEY, ";
createRepeatedPrimitiveFieldTableString += "repeatedFieldForeignKey INTEGER NOT NULL, ";
SOM_TRY
createRepeatedPrimitiveFieldTableString += "repeatedFieldValue " + cppTypeToDatabaseTypeString(inputMessageDescriptor.field(repeatedPrimitiveFieldNumbers[i])->cpp_type()) + ");\n";
SOM_CATCH("Error determining type\n");

createRepeatedPrimitiveFieldTableString += "CREATE INDEX " + repeatedFieldTableName + "__index" + " ON " + repeatedFieldTableName + "(" + "repeatedFieldForeignKey);";

int returnValue = sqlite3_exec(&databaseConnection, createRepeatedPrimitiveFieldTableString.c_str(), nullptr, nullptr, nullptr);
if(returnValue != SQLITE_OK)
{
throw SOMException("Error executing statement: (" +std::to_string(returnValue) +")\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Save table name
fieldNumberToPrimaryTableName[repeatedPrimitiveFieldNumbers[i]] = repeatedFieldTableName;

std::string repeatedFieldInsertionStatementString = "INSERT INTO " + repeatedFieldTableName + " (repeatedFieldForeignKey, repeatedFieldValue) VALUES (?, ?);";

std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> statmentPointerBuffer(nullptr, &sqlite3_finalize);

//Have to create tables before compiling insert statements
SOM_TRY
prepareStatement(statmentPointerBuffer, repeatedFieldInsertionStatementString, databaseConnection);
SOM_CATCH("Error preparing insert primary row statment\n");

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
@param inputMessageDescriptor: The message template to make the definition for

@throw: This function can throw exceptions 
*/
void subMessageDatabaseDefinition::initializeSubMessageFields(const google::protobuf::Descriptor &inputMessageDescriptor)
{
//Initialize singular submessage definitions
for(int i=0; i < singularMessageFieldNumbers.size(); i++)
{
std::string subMessageTableName = messageTableName + "__" + inputMessageDescriptor.field(singularMessageFieldNumbers[i])->name();

//Save table name
fieldNumberToPrimaryTableName[singularMessageFieldNumbers[i]] = subMessageTableName;

fieldNumberToSubMessageData.emplace(singularMessageFieldNumbers[i], subMessageDatabaseDefinition(databaseConnection, *(inputMessageDescriptor.field(singularMessageFieldNumbers[i])->message_type()), subMessageTableName, messageTableName, "primaryKey", true));
}

//Intitialize repeated submessage definitions
for(int i=0; i < repeatedMessageFieldNumbers.size(); i++)
{
std::string subMessageTableName = messageTableName + "_" + inputMessageDescriptor.field(repeatedMessageFieldNumbers[i])->name();

//Save table name
fieldNumberToPrimaryTableName[repeatedMessageFieldNumbers[i]] = subMessageTableName;

fieldNumberToSubMessageData.emplace(repeatedMessageFieldNumbers[i], subMessageDatabaseDefinition(databaseConnection, *(inputMessageDescriptor.field(repeatedMessageFieldNumbers[i])->message_type()), subMessageTableName, messageTableName, "primaryKey", false));
}
}

/**
This function is used in the store function to insert the primary row associated with a submessage.
@param inputSubMessageToStore: The message to store the primary row of
@param inputPrimaryKey: The primary key to assign to the submessage
@param inputForeignKey: The foreign key which points back to the primary table of the (sub)message that owns this submessage

@throw: This function can throw exceptions 
*/
void subMessageDatabaseDefinition::insertPrimaryRow(const google::protobuf::Message &inputSubMessageToStore, int64_t inputPrimaryKey, int64_t inputForeignKey)
{
const google::protobuf::Descriptor *messageDescriptor = inputSubMessageToStore.GetDescriptor();

//Primary Key, foreign key, different field values
SOM_TRY
bindFieldValueToStatement(*insertPrimaryRowStatement, 1, inputPrimaryKey);
SOM_CATCH("Error binding statement value\n")

SOM_TRY
bindFieldValueToStatement(*insertPrimaryRowStatement, 2, inputForeignKey);
SOM_CATCH("Error binding statement value\n")

//Set fields for the primary row insertion statement
for(int i=0; i < singularPrimitiveFieldNumbers.size(); i++)
{
SOM_TRY
bindFieldValueToStatement(*insertPrimaryRowStatement, i+3, inputSubMessageToStore, messageDescriptor->field(singularPrimitiveFieldNumbers[i]));
SOM_CATCH("Error binding statement value\n")
}

//Insert the primary row
SOM_TRY
stepAndResetSQLiteStatement(*insertPrimaryRowStatement);
SOM_CATCH("Error, unable to step/reset SQLite statement\n")
}

/**
This function is used in the store function to insert the repeated fields values into their associated tables.
@param inputSubMessageToStore: The message to store the repeated fields of
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void subMessageDatabaseDefinition::insertRepeatedFieldsInAssociatedTables(const google::protobuf::Message &inputSubMessageToStore, int64_t inputForeignKey)
{
const google::protobuf::Descriptor *messageDescriptor = inputSubMessageToStore.GetDescriptor();

const google::protobuf::Reflection *messageReflection = inputSubMessageToStore.GetReflection();

for(int i=0; i < repeatedPrimitiveFieldNumbers.size(); i++)
{ //For each repeated field
for(int a = 0; a < messageReflection->FieldSize(inputSubMessageToStore, messageDescriptor->field(repeatedPrimitiveFieldNumbers[i])); a++)
{ //For each element in the repeated field
SOM_TRY
bindFieldValueToStatement(*repeatedFieldNumberToInsertionStatements.at(repeatedPrimitiveFieldNumbers[i]), 1, inputForeignKey);
SOM_CATCH("Error binding statement value\n")

SOM_TRY
bindRepeatedFieldValueToStatement(*repeatedFieldNumberToInsertionStatements.at(repeatedPrimitiveFieldNumbers[i]), 2, inputSubMessageToStore, messageDescriptor->field(repeatedPrimitiveFieldNumbers[i]), a);
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
void subMessageDatabaseDefinition::insertSubMessages(const google::protobuf::Message &inputSubMessageToStore, int64_t inputForeignKey)
{
const google::protobuf::Descriptor *messageDescriptor = inputSubMessageToStore.GetDescriptor();

const google::protobuf::Reflection *messageReflection = inputSubMessageToStore.GetReflection();

for(int i=0; i<singularMessageFieldNumbers.size(); i++)
{ //Store singular messages
if(messageReflection->HasField(inputSubMessageToStore, messageDescriptor->field(singularMessageFieldNumbers[i])) == true)
{
fieldNumberToSubMessageData.at(singularMessageFieldNumbers[i]).store(messageReflection->GetMessage(inputSubMessageToStore, messageDescriptor->field(singularMessageFieldNumbers[i])), inputForeignKey);
}
}

for(int i=0; i<repeatedMessageFieldNumbers.size(); i++)
{ //Store repeated messages
for(int a = 0; a < messageReflection->FieldSize(inputSubMessageToStore, messageDescriptor->field(repeatedMessageFieldNumbers[i])); a++)
{
fieldNumberToSubMessageData.at(repeatedMessageFieldNumbers[i]).store(messageReflection->GetRepeatedMessage(inputSubMessageToStore, messageDescriptor->field(repeatedMessageFieldNumbers[i]), a), inputForeignKey);
}
}

}

/**
This function retrieves a singular submessage and places it in the associated field of the given message if it is present in the database and clears the field otherwise.
@param inputMessageBuffer: The message to store the submessage in
@param inputForeignKey: The primary key of the message to store it in
@param inputField: The field that the message should be stored in

@throw: This function can throw exceptions
*/
void subMessageDatabaseDefinition::retrieveSingular(google::protobuf::Message &inputMessageBuffer, int64_t inputForeignKey, const google::protobuf::FieldDescriptor *inputField)
{
//Primary key, foreign key, values
const google::protobuf::Descriptor *messageDescriptor = inputMessageBuffer.GetDescriptor();

const google::protobuf::Reflection *messageReflection = inputMessageBuffer.GetReflection();

SOM_TRY
bindFieldValueToStatement(*retrievePrimaryRowsStatement, 1, inputForeignKey);
SOM_CATCH("Error binding foreign key to retrieval statement\n")

SOMScopeGuard statementScopeGuard([&](){sqlite3_reset(retrievePrimaryRowsStatement.get());});

if(sqlite3_step(retrievePrimaryRowsStatement.get()) == SQLITE_ROW)
{
google::protobuf::Message &subMessageReference = *messageReflection->MutableMessage(&inputMessageBuffer, inputField);
const google::protobuf::Descriptor *subMessageDescriptor = subMessageReference.GetDescriptor();

for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{
SOM_TRY
retrieveFieldValueFromStatement(*retrievePrimaryRowsStatement, i+2, subMessageReference, subMessageDescriptor->field(singularPrimitiveFieldNumbers[i]));
SOM_CATCH("Error, unable to retrieve singular submessage field from database\n")
}

int64_t subMessagePrimaryKey = sqlite3_column_int64(retrievePrimaryRowsStatement.get(), 0);

SOM_TRY
retrieveRepeatedFields(subMessageReference, subMessagePrimaryKey);
SOM_CATCH("Error retrieving repeated fields\n")

SOM_TRY
retrieveSubMessages(subMessageReference, subMessagePrimaryKey);
SOM_CATCH("Error retrieving repeated fields\n")
}
else
{
messageReflection->ClearField(&inputMessageBuffer, inputField);
}


}

/**
This function retrieves repeated submessages and places them in the associated field of the given message if it is present in the database.
@param inputMessageBuffer: The message to store the submessage in
@param inputForeignKey: The primary key of the message to store it in
@param inputField: The field that the message should be stored in

@throw: This function can throw exceptions
*/
void subMessageDatabaseDefinition::retrieveRepeated(google::protobuf::Message &inputMessageBuffer, int64_t inputForeignKey, const google::protobuf::FieldDescriptor *inputField)
{
//Primary key, foreign key, values
const google::protobuf::Descriptor *messageDescriptor = inputMessageBuffer.GetDescriptor();

const google::protobuf::Reflection *messageReflection = inputMessageBuffer.GetReflection();

SOM_TRY
bindFieldValueToStatement(*retrievePrimaryRowsStatement, 1, inputForeignKey);
SOM_CATCH("Error binding foreign key to retrieval statement\n")

SOMScopeGuard statementScopeGuard([&](){sqlite3_reset(retrievePrimaryRowsStatement.get());});

while(sqlite3_step(retrievePrimaryRowsStatement.get()) == SQLITE_ROW)
{
google::protobuf::Message &subMessageReference = *messageReflection->AddMessage(&inputMessageBuffer, inputField);
const google::protobuf::Descriptor *subMessageDescriptor = subMessageReference.GetDescriptor();

for(int i=0; i<singularPrimitiveFieldNumbers.size(); i++)
{
SOM_TRY
retrieveFieldValueFromStatement(*retrievePrimaryRowsStatement, i+2, subMessageReference, subMessageDescriptor->field(singularPrimitiveFieldNumbers[i]));
SOM_CATCH("Error, unable to retrieve repeated submessage field from database\n")
}

int64_t subMessagePrimaryKey = sqlite3_column_int64(retrievePrimaryRowsStatement.get(), 0);

SOM_TRY
retrieveRepeatedFields(subMessageReference, subMessagePrimaryKey);
SOM_CATCH("Error retrieving repeated fields\n")

SOM_TRY
retrieveSubMessages(subMessageReference, subMessagePrimaryKey);
SOM_CATCH("Error retrieving repeated fields\n")
}

}

/**
This function retrieves the repeated fields of a given submessage from the SQLite database.
@param inputSubMessageBuffer: The submessage to add the repeated fields values to
@param inputForeignKey: The foreign key value which the repeated field tables point back to

@throw: This function can throw exceptions  
*/
void subMessageDatabaseDefinition::retrieveRepeatedFields(google::protobuf::Message &inputSubMessageBuffer, int64_t inputForeignKey)
{
//Primary key, foreign key, value
const google::protobuf::Descriptor *messageDescriptor = inputSubMessageBuffer.GetDescriptor();

const google::protobuf::Reflection *messageReflection = inputSubMessageBuffer.GetReflection();

for(int i=0; i<repeatedPrimitiveFieldNumbers.size(); i++)
{
SOM_TRY
bindFieldValueToStatement(*repeatedFieldNumberToRetrievalStatement.at(repeatedPrimitiveFieldNumbers[i]), 1, inputForeignKey);
SOM_CATCH("Error binding foreign key to retrieval statement\n")

SOMScopeGuard statementScopeGuard([&](){sqlite3_reset(repeatedFieldNumberToRetrievalStatement.at(repeatedPrimitiveFieldNumbers[i]).get());});

while(sqlite3_step(repeatedFieldNumberToRetrievalStatement.at(repeatedPrimitiveFieldNumbers[i]).get()) == SQLITE_ROW)
{
SOM_TRY
retrieveRepeatedFieldValueFromStatement(*repeatedFieldNumberToRetrievalStatement.at(repeatedPrimitiveFieldNumbers[i]), 2, inputSubMessageBuffer, messageDescriptor->field(repeatedPrimitiveFieldNumbers[i]));
SOM_CATCH("Error, unable to retrieve singular field from database\n")
}

}

}

/**
This function retrieves the repeated fields of a given submessage from the SQLite database.
@param inputSubMessageBuffer: The submessage to add the submessage values to
@param inputForeignKey: The foreign key value which the submessage tables point back to

@throw: This function can throw exceptions  
*/
void subMessageDatabaseDefinition::retrieveSubMessages(google::protobuf::Message &inputSubMessageToRetrieve, int64_t inputForeignKey)
{
const google::protobuf::Descriptor *messageDescriptor = inputSubMessageToRetrieve.GetDescriptor();

const google::protobuf::Reflection *messageReflection = inputSubMessageToRetrieve.GetReflection();

for(int i=0; i<singularMessageFieldNumbers.size(); i++)
{ //Retrieve singular messages
SOM_TRY
fieldNumberToSubMessageData.at(singularMessageFieldNumbers[i]).retrieveSingular(inputSubMessageToRetrieve, inputForeignKey, messageDescriptor->field(singularMessageFieldNumbers[i]));
SOM_CATCH("Error, unable to retrieve singular submessage\n")
}

for(int i=0; i<repeatedMessageFieldNumbers.size(); i++)
{ //Retrieve repeated messages
SOM_TRY
fieldNumberToSubMessageData.at(repeatedMessageFieldNumbers[i]).retrieveRepeated(inputSubMessageToRetrieve, inputForeignKey, messageDescriptor->field(repeatedMessageFieldNumbers[i]));
SOM_CATCH("Error, unable to retrieve repeated submessages\n")
}

}
