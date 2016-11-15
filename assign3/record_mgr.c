#include <string.h>
#include <stdlib.h>

#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "record_mgr.h"
#include "expr.h"
#include "tables.h"

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

	initBufferPool(bm, name, 200, RS_LRU, NULL);
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
	freeSchema(rel->schema);
	shutdownBufferPool(tableHandle->bm);
	free(tableHandle);
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
	int page, newFreeSlot, recordOffset;
	int *freeSlot;
	char *pageData;
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	//READ PAGE FROM DISK IF IS NOT ALREADY IN BUFFER
	page = tableHandle->freeSpacePage;
	pinPage(tableHandle->bm, pageHandle, page);
	//LOOK FOR FREE SLOT
	freeSlot = (int *)pageHandle->data;
	pageData = pageHandle->data + sizeof(int);
	if(*freeSlot >= 0){
		pageData[*freeSlot*sizeof(char)] = FULL_SLOT; //SET SLOT TO FULL
		recordOffset = tableHandle->pageCap*sizeof(char) + *freeSlot*(tableHandle->recordSize+sizeof(RID));
		memcpy(pageData + recordOffset, &page, sizeof(int));
		recordOffset += sizeof(int);
		memcpy(pageData + recordOffset, freeSlot, sizeof(int));
		recordOffset += sizeof(int);
		(record->id).page = page;
		(record->id).slot = *freeSlot;
		memcpy(pageData + recordOffset, record->data, tableHandle->recordSize);
		// BUFFER MANAGER OPERATIONS
	}else{
		//FULL PAGE
		tableHandle->freeSpacePage++;
		unpinPage (tableHandle->bm, pageHandle);
		pinPage(tableHandle->bm, pageHandle, 0);
		memcpy(pageHandle->data + sizeof(SM_FileHeader) + sizeof(short), &(tableHandle->freeSpacePage), sizeof(short));
		//pageHandle->data[sizeof(SM_FileHeader) + sizeof(short)] = tableHandle->freeSpacePage;
		markDirty(tableHandle->bm, pageHandle);
		unpinPage(tableHandle->bm, pageHandle);
		return insertRecord(rel, record);
	}
	newFreeSlot = *freeSlot;
	while(((pageData[newFreeSlot*sizeof(char)]) == FULL_SLOT) && newFreeSlot < tableHandle->pageCap){
		newFreeSlot++;
	}
	if(newFreeSlot < tableHandle->pageCap){
		*freeSlot = newFreeSlot;
	}else{
		int *fullPointer = (int *)pageHandle->data;
		*fullPointer = -1;
	}
	markDirty(tableHandle->bm, pageHandle);
	unpinPage (tableHandle->bm, pageHandle);
	return RC_OK;
}
RC deleteRecord (RM_TableData *rel, RID id){
	int recordOffset;
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	pinPage (tableHandle->bm, pageHandle, id.page);
	// SET TO EMPTY IN HEADER DIRECTORY
	pageHandle->data[sizeof(int) + id.slot*sizeof(char)] = 0x00;
	// I THINK THIS IS OPTIONAL
	recordOffset = sizeof(int) + tableHandle->pageCap * sizeof(char) + id.slot*(tableHandle->recordSize + sizeof(RID));
	memset(pageHandle->data + recordOffset,0,tableHandle->recordSize + sizeof(RID)); // Set record to 0
	// BUFFER MANAGER OPERATIONS
	int *freeSlot = (int *)pageHandle->data;
	if(*freeSlot > id.slot || *freeSlot == -1){
		*freeSlot = id.slot;
	}
	markDirty(tableHandle->bm, pageHandle);
	unpinPage (tableHandle->bm, pageHandle);
	if(id.page < tableHandle->freeSpacePage){
		tableHandle->freeSpacePage = id.page;
		pinPage(tableHandle->bm, pageHandle, 0);
		memcpy(pageHandle->data + sizeof(SM_FileHeader) + sizeof(short), &(tableHandle->freeSpacePage), sizeof(short));
		//pageHandle->data[sizeof(SM_FileHeader) + sizeof(short)] = tableHandle->freeSpacePage;
		markDirty(tableHandle->bm, pageHandle);
		unpinPage(tableHandle->bm, pageHandle);
	}
	// GESTION DE ERRORES? SI NO HAY RECORD EN ESA UBICACION?
	return RC_OK;
}
RC updateRecord (RM_TableData *rel, Record *record){
	int recordOffset;
	RID id;
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	id = record->id;
	pinPage (tableHandle->bm, pageHandle, id.page);
	recordOffset = sizeof(int) + tableHandle->pageCap * sizeof(char) + id.slot*(tableHandle->recordSize + sizeof(RID)) + sizeof(RID);
	memcpy(pageHandle->data + recordOffset, record->data, tableHandle->recordSize);
	markDirty(tableHandle->bm, pageHandle);
	unpinPage(tableHandle->bm, pageHandle);
	// GESTION DE ERRORES? SI NO HAY RECORD EN ESA UBICACION?
	return RC_OK;
}
RC getRecord (RM_TableData *rel, RID id, Record *record){
	int recordOffset;
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	pinPage (tableHandle->bm, pageHandle, id.page);
	recordOffset = sizeof(int) + tableHandle->pageCap * sizeof(char) + id.slot*(tableHandle->recordSize + sizeof(RID)) + sizeof(RID);
	char *data = malloc(tableHandle->recordSize);
	memcpy(data, pageHandle->data + recordOffset, tableHandle->recordSize);
	unpinPage(tableHandle->bm, pageHandle);
	record->id = id;
	record->data = data;
	// printf("RETRIEVED RECORD: ");
	// for(int i = 0; i<tableHandle->recordSize; i++){
	// 	printf("%02x ", record->data[i] & 0xff);
	// }
	// printf("\n");
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
	for(int i = 0; i < schema->numAttr && recordSize != RC_RM_UNKOWN_DATATYPE; i++){
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
		  default: 
		  	recordSize = RC_RM_UNKOWN_DATATYPE;
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
	free(schema->attrNames);
	free(schema->dataTypes);
	free(schema->typeLength);
	free(schema->keyAttrs);
	free(schema);
	return RC_OK;
}

// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema){
	*record = malloc(sizeof(Record));
	(*record)->data = calloc(1, getRecordSize(schema));
	return RC_OK;
}

RC freeRecord (Record *record){
	free(record->data);
	free(record);
	return RC_OK;
}
RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){
	int attrOffset;
	RC code = RC_OK;
	if((attrOffset = getAttrOffset(schema, attrNum)) >= 0){
		switch(schema->dataTypes[attrNum]){
		  case DT_INT: 
			*value = malloc(sizeof(DataType) + sizeof(int));
			memcpy(&((*value)->v.intV),record->data + attrOffset, sizeof(int));
		  	break;
		  case DT_STRING:
			*value = malloc(sizeof(DataType) + schema->typeLength[attrNum] + 1);
			(*value)->v.stringV = malloc(schema->typeLength[attrNum]);
			memcpy((*value)->v.stringV,record->data + attrOffset, schema->typeLength[attrNum]);
			// Adds end of string
			(*value)->v.stringV[schema->typeLength[attrNum]] = 0x00;
		  	break;
		  case DT_FLOAT:
			*value = malloc(sizeof(DataType) + sizeof(float));
			memcpy(&((*value)->v.floatV),record->data + attrOffset, sizeof(float));
		  	break;
		  case DT_BOOL:
			*value = malloc(sizeof(DataType) + sizeof(bool));
			memcpy(&((*value)->v.boolV),record->data + attrOffset, sizeof(bool));
		  	break;
		  default: 
			code = RC_RM_UNKOWN_DATATYPE;
		}
		(*value)->dt = schema->dataTypes[attrNum];
	}else{
		code = RC_RM_UNKOWN_DATATYPE;
	}
	return code;
}
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
	int attrOffset;
	RC code = RC_OK;
	if((attrOffset = getAttrOffset(schema, attrNum)) >= 0){
		switch(schema->dataTypes[attrNum]){
			case DT_INT:
				memcpy(record->data + attrOffset, &(value->v.intV), sizeof(int));
				break;
			case DT_STRING: 
				memcpy(record->data + attrOffset, value->v.stringV, strlen(value->v.stringV));
				break;
			case DT_FLOAT:  
				memcpy(record->data + attrOffset, &(value->v.floatV), sizeof(float));
				break;
			case DT_BOOL:  
				memcpy(record->data + attrOffset, &(value->v.boolV), sizeof(bool));
				break;
			default: 
				code = RC_RM_UNKOWN_DATATYPE;
		}
	}else{
		code = RC_RM_UNKOWN_DATATYPE;
	}
	return code;
}

// Auxiliary functions

int getAttrOffset(Schema *schema, int attrNum){
	int attrOffset = 0;
	for(int i = 0; i < attrNum && attrOffset >= 0; i++){
		switch(schema->dataTypes[i]){
		  case DT_INT: 
		  	attrOffset += sizeof(int);
		  	break;
		  case DT_STRING: 
		  	attrOffset += schema->typeLength[i];
		  	break;
		  case DT_FLOAT: 
		  	attrOffset += sizeof(float);
		  	break;
		  case DT_BOOL: 
		  	attrOffset += sizeof(bool);
		  	break;
		  default: 
		  	attrOffset = -1;
		}
	}
	return attrOffset;
}

short getNumPagesSchema(char *name){
  	FILE *file = fopen(name, "r+");
  	fseek(file, sizeof(SM_FileHeader), SEEK_SET);
  	short tableInfoSize;
  	fread(&tableInfoSize, sizeof(short), 1, file);
  	return tableInfoSize;
}

int calculatePageCap(Schema *schema){
	int recordSize, numRecords;
	recordSize = getRecordSize(schema) + (int)sizeof(RID);
	// -1 is for free slot pointer
	numRecords = (PAGE_SIZE - 1)/(sizeof(char) + recordSize);
}




