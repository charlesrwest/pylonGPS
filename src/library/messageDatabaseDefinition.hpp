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
This class contains all of the information required to store/retrieve protobuf objects of the type it was constructed with to/from the given SQLite database connection.
*/
class messageDatabaseDefinition
{
public:
/**
This function initializes the maps associated with the given object types fields and creates the SQLite tables to store the associated data.
@param inputDatabaseConnection: The connection to the database to use
@param inputMessageDescriptor: The protobuf Descriptor associated with the message
@param inputStringToPreappendToTableNames: What to stick in front of the object template name

@throws: This function can throw exceptions if one of the database operations fail or there is an unrecognized field type
*/
messageDatabaseDefinition(sqlite3 &inputDatabaseConnection, const google::protobuf::Descriptor &inputMessageDescriptor, const std::string &inputStringToPreappendToTableNames = ""); 

/**
This function stores the given message in the database.
@param inputMessageToStore: The message to store in the database

@throw: This function can throw exceptions
*/
void store(const google::protobuf::Message &inputMessageToStore);

/**
This function stores the given message in the database.
@param inputPrimaryKey: The primary key to search for
@param inputMessageBuffer: The message to store the returned object in

@throw: This function can throw exceptions
*/
void retrieve(int64_t inputPrimaryKey, google::protobuf::Message &inputMessageBuffer);

/**
This function deletes a message with the given primary key from the database (if present).
@param inputPrimaryKey: The primary key of the message to delete

@throw: This function can throw exceptions
*/
void deleteMessage(int64_t inputPrimaryKey);

/**
This function updates the primary row of the protobuf message.
@param inputPrimaryKey: The primary key to search for
@param inputFieldNumber: The field number to update in the database
@param inputValue: The integer value to assign to the field

@throw: This function can throw exceptions
*/
void update(int64_t inputPrimaryKey, uint32_t inputFieldNumber, int64_t inputValue);

/**
This function updates the primary row of the protobuf message.
@param inputPrimaryKey: The primary key to search for
@param inputFieldNumber: The field number to update in the database
@param inputValue: The double value to assign to the field

@throw: This function can throw exceptions
*/
void update(int64_t inputPrimaryKey, uint32_t inputFieldNumber, double inputValue);

/**
This function updates the primary row of the protobuf message.
@param inputPrimaryKey: The primary key to search for
@param inputFieldNumber: The field number to update in the database
@param inputValue: The string value to assign to the field

@throw: This function can throw exceptions
*/
void update(int64_t inputPrimaryKey, uint32_t inputFieldNumber, const std::string &inputValue);

/**
This function updates the primary row of the protobuf message with a NULL value.
@param inputPrimaryKey: The primary key to search for
@param inputFieldNumber: The field number to update in the database

@throw: This function can throw exceptions
*/
void update(int64_t inputPrimaryKey, uint32_t inputFieldNumber);

sqlite3 &databaseConnection; //Reference to the database connection to use
const google::protobuf::Descriptor *messageDescriptor;
std::string messageTableName; //The name of the protobuf object with the preappended string
int64_t primaryKeyFieldNumber; //The number of the field which holds the primary key for the object
std::map<int64_t, std::string> fieldNumberToPrimaryTableName;
std::vector<int64_t> singularFieldNumbers; //The field numbers of all of the singular fields
std::vector<int64_t> singularPrimitiveFieldNumbers; //The field numbers of all of the singular fields which are not submessages
std::vector<int64_t> singularMessageFieldNumbers; //The field numbers of all of the singular fields which are submessages
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> insertPrimaryRowStatement; //The SQLite prepared statement to insert the primary row associated with this object (all singular fields that are not submessages)
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> retrievePrimaryRowStatement; //The SQLite prepared statement to retrieve the primary row associated with a given message given a primary key
std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> deleteMessageStatement; //The SQLite prepared statement to delete the primary row associated with this object (which removes all repeated fields and submessages via cascading deletes)
std::map<int64_t, std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > singularPrimitiveFieldNumberToUpdateMessageStatement;  //SQLite prepared statement to update one of the fields in the primary row of the database

std::vector<int64_t> repeatedFieldNumbers; //The field numbers associated with each of the repeated fields
std::vector<int64_t> repeatedPrimitiveFieldNumbers; //The field numbers of all of the singular fields which are not submessages
std::vector<int64_t> repeatedMessageFieldNumbers; //The field numbers of all of the singular fields which are submessages
std::map<int64_t, std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedFieldNumberToInsertionStatements; //The statements to insert a instance of a repeated field which is not a submessage
std::map<int64_t, std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> > repeatedFieldNumberToRetrievalStatement; //The statements to retrieve a instance of a repeated field which is not a submessage
std::map<int64_t, subMessageDatabaseDefinition> fieldNumberToSubMessageData; //All the data used to insert submessages

private:
/**
This function finds the field that holds the primary key for this particular message type.

@throw: This function can throw exceptions
*/
void setPrimaryKeyFieldIndex();

/**
This function initializes the vectors which have the field numbers of the different field types.

@throw: This function can throw exceptions
*/
void initializeFieldNumberVectors();

/**
This function creates the primary row for the singular fields of this message and creates the associated insertion/retrieval/deletion statements.

@throw: This function can throw exceptions
*/
void initializePrimaryRow();

/**
This function creates the tables associated with the repeated fields of this message and creates the associated insertion/retrieval/deletion statements.

@throw: This function can throw exceptions
*/
void initializePrimitiveRepeatedFields();

/**
This function creates the tables associated with the fields of this submessage and creates the associated insertion/retrieval/deletion statements.

@throw: This function can throw exceptions 
*/
void initializeSubMessageFields();

/**
This function is used in the store function to insert the primary row associated with a message.
@param inputMessageToStore: The message to store the primary row of

@throw: This function can throw exceptions 
*/
void insertPrimaryRow(const google::protobuf::Message &inputMessageToStore);

/**
This function is used in the store function to insert the repeated fields values into their associated tables.
@param inputMessageToStore: The message to store the repeated fields of
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void insertRepeatedFieldsInAssociatedTables(const google::protobuf::Message &inputMessageToStore, int64_t inputForeignKey);

/**
This function inserts the submessages associated with the given object into their respective tables.
@param inputMessageToStore: The message to store the repeated fields of
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void insertSubMessages(const google::protobuf::Message &inputMessageToStore, int64_t inputForeignKey);


/**
This function retrieves the primary row associated with this message
@param inputPrimaryKey: The primary key to search for
@param inputMessageBuffer: The message to store the returned object in

@throw: This function can throw exceptions
*/
void retrievePrimaryRow(int64_t inputPrimaryKey, google::protobuf::Message &inputMessageBuffer);

/**
This function is used in the store function to insert the repeated fields values into their associated tables.
@param inputMessageBuffer: The message to store the returned object in
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void retrievePrimitiveRepeatedFieldsFromAssociatedTables(google::protobuf::Message &inputMessageBuffer, int64_t inputForeignKey);

/**
This function retrieves the submessages associated with the given message from their respective tables.
@param inputMessageBuffer: The message to place the retrieved submessages into
@param inputForeignKey: The foreign key which points back to the main message in the primary table

@throw: This function can throw exceptions 
*/
void retrieveSubMessages(google::protobuf::Message &inputMessageBuffer, int64_t inputForeignKey);
};





} 
