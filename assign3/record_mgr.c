#include <string.h>
#include <stdlib.h>

#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "record_mgr.h"
#include "expr.h"
#include "tables.h"

short getNumPagesSchema(char *name);
int calculatePageCap(Schema *schema);

void printSchema(Schema *schema);

typedef struct TableHandle
{
  BM_BufferPool *bm;
  int pageCap;
  int numPagesSchema;
  int recordSize;
  short freeSpacePage;
} TableHandle;

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
	//Open file to read
  	FILE *file = fopen(name, "r+");
  	// We already have management data for storage manager stored
  	fseek(file, sizeof(SM_FileHeader) + 2*sizeof(short), SEEK_SET);
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

	long schemaSize = ftell(file) - sizeof(SM_FileHeader);

	short numPagesSchema = schemaSize/PAGE_SIZE + 1;
	short freeSpacePage = numPagesSchema;
  	fseek(file, sizeof(SM_FileHeader), SEEK_SET);
	fwrite(&numPagesSchema, sizeof(short), 1, file);
	fwrite(&freeSpacePage, sizeof(short), 1, file);

  	fclose(file);

	return RC_OK;
}
RC openTable (RM_TableData *rel, char *name){
	TableHandle *tableHandle = malloc(sizeof(TableHandle));
	Schema *schema = malloc(sizeof(Schema));
	BM_BufferPool *bm = malloc(sizeof(BM_BufferPool));
  	FILE *file = fopen(name, "r+");
  	// Reserve space for header size
  	// sizeof(short) for tableInfoSize
  	fseek(file, sizeof(SM_FileHeader), SEEK_SET);
  	fread(&(tableHandle->numPagesSchema), sizeof(short), 1, file);
  	fread(&(tableHandle->freeSpacePage), sizeof(short), 1, file);

  	fread(&(schema->numAttr), sizeof(int), 1, file);
  	schema->attrNames = malloc(schema->numAttr * sizeof(char*));
  	for(int i = 0; i < schema->numAttr;  i++){
  		short attrNameLen = 0;
  		fread(&attrNameLen, sizeof(short), 1, file);
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

	initBufferPool(bm, name, 10, RS_FIFO, NULL);
	tableHandle->bm = bm;
	tableHandle->pageCap = calculatePageCap(schema);
	tableHandle->recordSize = getRecordSize(schema);

	rel->name = name;
	rel->schema = schema;
	rel->mgmtData = tableHandle;
	// LLAMAMOS A INITBUFFERPOOL?
	// QUIEN DECIDE STRATEGY?
	//   "      "    NUMPAGES?
	return RC_OK;
}
RC closeTable (RM_TableData *rel){
	//TODO: UPDATE INFO: FREE SPACE
	//TODO: CALL BUFFER TO FINISH
	TableHandle *tableHandle = rel->mgmtData;
	shutdownBufferPool(tableHandle->bm);
	return RC_OK;
}
RC deleteTable (char *name){
	destroyPageFile(name);
	//TODO: Free other resources if necessary
	return RC_OK;
}
int getNumTuples (RM_TableData *rel){
	//TODO: 3/10
	// CONTAMOS O MANTENEMOS UN CONTADOR?
	TableHandle *tableHandle = rel->mgmtData;
	return 0;
}

// handling records in a table
RC insertRecord (RM_TableData *rel, Record *record){
	//TODO: Busqueda de pagina
	// TEMPORAL
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	//READ PAGE FROM DISK IF IS NOT ALREADY IN BUFFER
	int page = tableHandle->freeSpacePage;
	pinPage (tableHandle->bm, pageHandle, page);
	//LOOK FOR FREE SLOT
	int freeSlot = pageHandle->data[0];
	char *data = pageHandle->data + sizeof(int);
	// int freeSlot = 0;
	// char full = 0x01;
	// while(((pageHandle->data[freeSlot*sizeof(char)]) == 0x01) && freeSlot < tableHandle->pageCap){
	// 	freeSlot++;
	// }
	if(freeSlot >= 0){
		data[freeSlot*sizeof(char)] = 0x01; //SET SLOT TO FULL
		int recordOffset = tableHandle->pageCap*sizeof(char) + freeSlot*(tableHandle->recordSize+sizeof(RID));
		data[recordOffset] = page;
		recordOffset += sizeof(int);
		data[recordOffset] = freeSlot;
		recordOffset += sizeof(int);
		(record->id).page = page;
		(record->id).slot = freeSlot;
		// WRITE RECORD DATA
		memcpy(data + recordOffset, record->data, tableHandle->recordSize);
		// BUFFER MANAGER OPERATIONS
	}else{
		//FULL PAGE
		tableHandle->freeSpacePage++;
		unpinPage (tableHandle->bm, pageHandle);
		pinPage(tableHandle->bm, pageHandle, 0);
		pageHandle->data[sizeof(SM_FileHeader) + sizeof(short)] = tableHandle->freeSpacePage;
		markDirty(tableHandle->bm, pageHandle);
		unpinPage(tableHandle->bm, pageHandle);
		return insertRecord(rel, record);
	}
	int newFreeSlot = freeSlot;
	char full = 0x01;
	while(((data[newFreeSlot*sizeof(char)]) == 0x01) && newFreeSlot < tableHandle->pageCap){
		newFreeSlot++;
	}
	if(newFreeSlot < tableHandle->pageCap){
		pageHandle->data[0] = newFreeSlot;
	}else{
		pageHandle->data[0] = -1;
	}
	markDirty(tableHandle->bm, pageHandle);
	unpinPage (tableHandle->bm, pageHandle);
	return RC_OK;
}
RC deleteRecord (RM_TableData *rel, RID id){
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	pinPage (tableHandle->bm, pageHandle, id.page);
	// SET TO EMPTY IN HEADER DIRECTORY
	pageHandle->data[sizeof(int) + id.slot*sizeof(char)] = 0x00;
	// I THINK THIS IS OPTIONAL
	int recordOffset = sizeof(int) + tableHandle->pageCap * sizeof(char) + id.slot*(tableHandle->recordSize + sizeof(RID));
	memset(pageHandle->data + recordOffset,0,tableHandle->recordSize + sizeof(RID)); // Set record to 0
	// BUFFER MANAGER OPERATIONS
	if(pageHandle->data[0] > id.slot || pageHandle->data[0] == -1){
		pageHandle->data[0] = id.slot;
	}
	markDirty(tableHandle->bm, pageHandle);
	unpinPage (tableHandle->bm, pageHandle);
	if(id.page < tableHandle->freeSpacePage){
		tableHandle->freeSpacePage = id.page;
		pinPage(tableHandle->bm, pageHandle, 0);
		pageHandle->data[sizeof(SM_FileHeader) + sizeof(short)] = tableHandle->freeSpacePage;
		markDirty(tableHandle->bm, pageHandle);
		unpinPage(tableHandle->bm, pageHandle);
	}
	// GESTION DE ERRORES? SI NO HAY RECORD EN ESA UBICACION?
	return RC_OK;
}
RC updateRecord (RM_TableData *rel, Record *record){
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	RID id = record->id;
	pinPage (tableHandle->bm, pageHandle, id.page);
	int recordOffset = sizeof(int) + tableHandle->pageCap * sizeof(char) + id.slot*(tableHandle->recordSize + sizeof(RID)) + sizeof(RID);
	printf("record offset %i\n", recordOffset);
	memcpy(pageHandle->data + recordOffset, record->data, tableHandle->recordSize);
	markDirty(tableHandle->bm, pageHandle);
	unpinPage(tableHandle->bm, pageHandle);
	// GESTION DE ERRORES? SI NO HAY RECORD EN ESA UBICACION?
	return RC_OK;
}
RC getRecord (RM_TableData *rel, RID id, Record *record){
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	pinPage (tableHandle->bm, pageHandle, id.page);
	int recordOffset = sizeof(int) + tableHandle->pageCap * sizeof(char) + id.slot*(tableHandle->recordSize + sizeof(RID)) + sizeof(RID);
	printf("%i\n", recordOffset);
	char *data = malloc(tableHandle->recordSize);
	memcpy(data, pageHandle->data + recordOffset, tableHandle->recordSize);
	unpinPage(tableHandle->bm, pageHandle);
	record->id = id;
	record->data = data;
	// GESTION DE ERRORES? SI NO HAY RECORD EN ESA UBICACION?
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
	int recordSize = 0;
	for(int i = 0; i < schema->numAttr; i++){
		switch(schema->dataTypes[i]){
		  case DT_INT: 
		  	recordSize += sizeof(int);
		  	break;
		  case DT_STRING: 
		  	recordSize += schema->typeLength[i];
		  	break;
		  case DT_FLOAT: 
		  	recordSize += sizeof(float);
		  	break;
		  case DT_BOOL: 
		  	recordSize += sizeof(bool);
		  	break;
		}
	}
	return recordSize;
}
Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){
	Schema *newSchema = malloc(sizeof(Schema));
	newSchema->numAttr = numAttr;
	newSchema->attrNames = attrNames;
	newSchema->dataTypes = dataTypes;
	newSchema->typeLength = typeLength;
	newSchema->keySize = keySize;
	newSchema->keyAttrs = keys;
	return newSchema;
}
RC freeSchema (Schema *schema){
	//TODO: QUE HACE ESTO?
	return RC_OK;
}

// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema){
	//TODO
	// PORQUE ES UN DOBLE PUNTERO ??
	return RC_OK;
}
RC freeRecord (Record *record){
	// TODO
	// QUE HACE ESTO
	return RC_OK;
}
RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){
	//TODO: 2/10
	return RC_OK;
}
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
	//TODO: 2/10
	return RC_OK;
}

// Auxiliary functions

short getNumPagesSchema(char *name){
  	FILE *file = fopen(name, "r+");
  	fseek(file, sizeof(SM_FileHeader), SEEK_SET);
  	short tableInfoSize;
  	fread(&tableInfoSize, sizeof(short), 1, file);
  	return tableInfoSize;
}

int calculatePageCap(Schema *schema){
	int recordSize = getRecordSize(schema) + (int)sizeof(RID);
	// -1 is for free slot pointer
	int numRecords = (PAGE_SIZE - 1)/(sizeof(char) + recordSize);
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



