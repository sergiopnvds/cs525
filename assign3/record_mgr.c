#include <string.h>
#include <stdlib.h>

#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "record_mgr.h"
#include "expr.h"
#include "tables.h"


int writeTableInfo(char *name, Schema *schema);
Schema readSchema(char *name);
void printSchema(Schema *schema);

// table and manager
RC initRecordManager (void *mgmtData){
	return RC_OK;
}
RC shutdownRecordManager (){
	return RC_OK;
}
RC createTable (char *name, Schema *schema){
	RC createStatus = createPageFile(name);
	if(createStatus != RC_OK){
		return createStatus;
	}
    writeTableInfo(name, schema);
    Schema schema2 = readSchema(name);
    printSchema(&schema2);
	return RC_OK;
}
RC openTable (RM_TableData *rel, char *name){
	return RC_OK;
}
RC closeTable (RM_TableData *rel){
	return RC_OK;
}
RC deleteTable (char *name){
	return RC_OK;
}
int getNumTuples (RM_TableData *rel){
	return 0;
}

// handling records in a table
RC insertRecord (RM_TableData *rel, Record *record){
	return RC_OK;
}
RC deleteRecord (RM_TableData *rel, RID id){
	return RC_OK;
}
RC updateRecord (RM_TableData *rel, Record *record){
	return RC_OK;
}
RC getRecord (RM_TableData *rel, RID id, Record *record){
	return RC_OK;
}

// scans
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){
	return RC_OK;
}
RC next (RM_ScanHandle *scan, Record *record){
	return RC_OK;
}
RC closeScan (RM_ScanHandle *scan){
	return RC_OK;
}

// dealing with schemas
int getRecordSize (Schema *schema){
	return 0;
}
Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){
	return RC_OK;
}
RC freeSchema (Schema *schema){
	return RC_OK;
}

// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema){
	return RC_OK;
}
RC freeRecord (Record *record){
	return RC_OK;
}
RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){
	return RC_OK;
}
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
	return RC_OK;
}

// Auxiliary functions

int writeTableInfo(char *name, Schema *schema){
	//Open file to read
  	FILE *file = fopen(name, "r+");
  	// We already have management data for storage manager stored
  	fseek(file, sizeof(SM_FileHeader), SEEK_SET);
  	// Write number of attributes
  	fwrite(&(schema->numAttr), sizeof(int), 1, file);
  	// Write each string preceded by its lenght
  	for(int i = 0; i < schema->numAttr; i++){
  		short attrlen = (short)strlen(schema->attrNames[i]);
  		fwrite(&attrlen, sizeof(short), 1, file);
  		fwrite(schema->attrNames[i], sizeof(char), attrlen, file);
  	}
  	// writes 
	fwrite(schema->dataTypes, sizeof(int), schema->numAttr, file);
	fwrite(schema->typeLength, sizeof(int), schema->numAttr, file);
  	fwrite(&(schema->keySize), sizeof(int), 1, file);
	fwrite(schema->keyAttrs, sizeof(int), schema->keySize, file);

  	fclose(file);
}

Schema readSchema(char *name){
  	FILE *file = fopen(name, "r+");
  	// Reserve space for header size
  	fseek(file, sizeof(SM_FileHeader), SEEK_SET);
  	Schema *schema = malloc(sizeof(Schema));

  	fread(&(schema->numAttr), sizeof(int), 1, file);
  	schema->attrNames = malloc(schema->numAttr * sizeof(char*));
  	for(int i = 0; i < schema->numAttr; i++){
  		short attrNameLen = 0;
  		fread(&attrNameLen, sizeof(short), 1, file);
  		printf("%i\n", attrNameLen);
  		schema->attrNames[i] = malloc(attrNameLen);
  		fread(schema->attrNames[i], attrNameLen, 1, file);
  	}
  	schema->dataTypes = malloc(sizeof(int) * schema->numAttr);
	fread(schema->dataTypes, sizeof(int), schema->numAttr, file);
  	schema->typeLength = malloc(sizeof(int) * schema->numAttr);
	fread(schema->typeLength, sizeof(int), schema->numAttr, file);
  	fread(&(schema->keySize), sizeof(int), 1, file);
  	schema->keyAttrs = malloc(sizeof(int) * schema->keySize);
	fread(schema->keyAttrs, sizeof(int), schema->keySize, file);
  	fclose(file);

  	return *schema;
}



void printSchema(Schema *schema){
	printf("numAttr: %i\n", schema->numAttr);
	printf("attrNames: \n");
  	for(int i = 0; i < schema->numAttr; i++){
  		printf("\t%s\n", schema->attrNames[i]);
  	}
	printf("dataTypes: \n");
  	for(int i = 0; i < schema->numAttr; i++){
  		printf("\t%i\n", schema->dataTypes[i]);
  	}
	printf("typeLength: \n");
  	for(int i = 0; i < schema->numAttr; i++){
  		printf("\t%i\n", schema->typeLength[i]);
  	}
	printf("keyAttrs: \n");
  	for(int i = 0; i < schema->keySize; i++){
  		printf("\t%i\n", schema->keyAttrs[i]);
  	}
	printf("keySize: %i\n", schema->keySize);
}
