#pragma once

#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "sqlite3.h"
#include<memory>
#include<map>
#include<google/protobuf/message.h>
#include "subMessageDatabaseDefinition.hpp"
#include "utilityFunctions.hpp"

namespace pylongps
{

/**
This class contains all of the information required to store/retrieve protobuf objects of the type it was constructed with from the given SQLite database connection.  This class is typically used for submessages in the messageDatabaseDefinition class.
*/
class subMessageDatabaseDefinition
{
public:
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
subMessageDatabaseDefinition(sqlite3 &inputDatabaseConnection, const google::protobuf::Descriptor &inputMessageDescriptor, const std::string &inputTableNameToUse, const std::string &inputForeignTableName, const std::string &inputForeignTablePrimaryKeyName, bool inputIsSingular = true);

/**
This function stores the given submessage in the database.
@param inputSubMessageToStore: The message to store in the database
@param inputForeignKey: The foreign key which points back to the parent message

@throw: This function can throw exceptions
*/
void store(const google::protobuf::Message &inputSubMessageToStore, int64_t inputForeignKey);

/**
This function retrieves a singular submessage and places it in the given message buffer.
@param inputSubMessageBuffer: The message to store the submessage in
@param inputForeignKey: The primary key of the message to store it in

@throw: This function can throw exceptions
*/
void retrieve(google::protobuf::Message &inputSubMessageBuffer, int64_t inputForeignKey);

/**
This function retrieves a singular submessage and places it in the associated field of the given message if it is present in the database and clears the field otherwise.
@param inputMessageBuffer: The message to store the submessage in
@param inputForeignKey: The primary key of the message to store it in
@param inputField: The field that the message should be stored in

@throw: This function can throw exceptions
*/
void retrieveSingular(google::protobuf::Message &inputMessageBuffer, int64_t inputForeignKey, const google::protobuf::FieldDescriptor *inputField);

/**
This function retrieves repeated submessages and places them in the associated field of the given message if it is present in the database.
@param inputMessageBuffer: The message to store the submessage in
@param inputForeignKey: The primary key of the message to store it in
@param inputField: The field that the message should be stored in

@throw: This function can throw exceptions
*/
void retrieveRepeated(google::protobuf::Message &inputMessageBuffer, int64_t inputForeignKey, const google::protobuf::FieldDescriptor *inputField);

sqlite3 &databaseConnection; //Reference to the database connection to use
int64_t subMessagePrimaryKey = 0; //The primary key to assign to the next message stored
bool isSingular; //Whether or not this field is singular
std::string foreignKeyTableName; //Name of the table this object's foreign key references
std::string foreignKeyTableFieldName; //Name of the field in the foreign table that holds the primary key
std::string messageTableName; //The name of the protobuf object with the preappended string
std::map<int64_t, std::string> fieldNumberToPrimaryTableName;
std::vector<int64_t> singularFieldNumbers; //The field numbers of all of the singular fields
std::vector<int64_t> singularPrimitiveFieldNumbers; //The field numbers of all of the singular fields which are not submessages
std::vector<int64_t> singularMessageFieldNumbers; //The field numbers of all of the singular fields which are submessages
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> insertPrimaryRowStatement; //The SQLite prepared statement to insert the primary row associated with this object (all singular fields that are not submessages)
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> retrievePrimaryRowsStatement; //The SQLite prepared statement to retrieve the primary row associated with a given message given a primary key
std::vector<int64_t> repeatedFieldNumbers; //The field numbers associated with each of the repeated fields
std::vector<int64_t> repeatedPrimitiveFieldNumbers; //The field numbers of all of the singular fields which are not submessages
std::vector<int64_t> repeatedMessageFieldNumbers; //The field numbers of all of the singular fields which are submessages
std::map<int64_t, std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedFieldNumberToInsertionStatements; //The statements to insert a instance of a repeated field which is not a submessage
std::map<int64_t, std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedFieldNumberToRetrievalStatement; //The statements to retrieve a instance of a repeated field which is not a submessage
std::map<int64_t, subMessageDatabaseDefinition> fieldNumberToSubMessageData; //All the data used to insert submessages

private:
/**
This function initializes the vectors which have the field numbers of the different field types.
@param inputMessageDescriptor: The message template to make the definition for

@throw: This function can throw exceptions
*/
void initializeFieldNumberVectors(const google::protobuf::Descriptor &inputMessageDescriptor);

/**
This function creates the primary row for the singular fields of this message and creates the associated insertion/retrieval/deletion statements.
@param inputMessageDescriptor: The message template to make the definition for

@throw: This function can throw exceptions
*/
void initializePrimaryRow(const google::protobuf::Descriptor &inputMessageDescriptor);

/**
This function creates the tables associated with the repeated fields of this message and creates the associated insertion/retrieval/deletion statements.
@param inputMessageDescriptor: The message template to make the definition for

@throw: This function can throw exceptions
*/
void initializePrimitiveRepeatedFields(const google::protobuf::Descriptor &inputMessageDescriptor);

/**
This function creates the tables associated with the fields of this submessage and creates the associated insertion/retrieval/deletion statements.
@param inputMessageDescriptor: The message template to make the definition for

@throw: This function can throw exceptions 
*/
void initializeSubMessageFields(const google::protobuf::Descriptor &inputMessageDescriptor);

/**
This function is used in the store function to insert the primary row associated with a submessage.
@param inputSubMessageToStore: The message to store the primary row of
@param inputPrimaryKey: The primary key to assign to the submessage
@param inputForeignKey: The foreign key which points back to the primary table of the (sub)message that owns this submessage

@throw: This function can throw exceptions 
*/
void insertPrimaryRow(const google::protobuf::Message &inputSubMessageToStore, int64_t inputPrimaryKey, int64_t inputForeignKey);

/**
This function is used in the store function to insert the repeated fields values into their associated tables.
@param inputSubMessageToStore: The message to store the repeated fields of
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void insertRepeatedFieldsInAssociatedTables(const google::protobuf::Message &inputSubMessageToStore, int64_t inputForeignKey);

/**
This function inserts the submessages associated with the given object into their respective tables.
@param inputMessageToStore: The message to store the repeated fields of
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void insertSubMessages(const google::protobuf::Message &inputSubMessageToStore, int64_t inputForeignKey);

/**
This function retrieves the repeated fields of a given submessage from the SQLite database.
@param inputSubMessageBuffer: The submessage to add the repeated fields values to
@param inputForeignKey: The foreign key value which the repeated field tables point back to

@throw: This function can throw exceptions  
*/
void retrieveRepeatedFields(google::protobuf::Message &inputSubMessageBuffer, int64_t inputForeignKey);

/**
This function retrieves the repeated fields of a given submessage from the SQLite database.
@param inputSubMessageBuffer: The submessage to add the submessage values to
@param inputForeignKey: The foreign key value which the submessage tables point back to

@throw: This function can throw exceptions  
*/
void retrieveSubMessages(google::protobuf::Message &inputSubMessageToRetrieve, int64_t inputForeignKey);
};




}  
