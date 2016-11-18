#include <string.h>
#include <stdlib.h>

#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "record_mgr.h"
#include "expr.h"
#include "tables.h"

typedef struct RM_ScanMgmt {
    Expr *cond;
    RID *id;
    int totalNumPages;
    int totalNumSlots;
} RM_ScanMgmt;

/**************************************************************************************************
 * Function Name: initRecordManager
 * Description:
 *	Initialize the record manager.
 *
 * Parameters:
 *	void *mgmtData
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC initRecordManager (void *mgmtData){
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: shutdownRecordManager
 * Description:
 *	Shut down the record manager.
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC shutdownRecordManager (){
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: createTable
 * Description:
 *	Create a new table with schema and name given.
 *
 * Parameters:
 *  char   *name
 *	Schema *schema
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC createTable (char *name, Schema *schema){

	RC createStatus = createPageFile(name);

	if(createStatus != RC_OK){
		return createStatus;
	}
	//Open file to read
	FILE *file = fopen(name, "r+");
	// We already have management data for storage manager stored
	fseek(file, sizeof(SM_FileHeader) + 3*sizeof(short), SEEK_SET);
	// Write number of attributes
	fwrite(&(schema->numAttr), sizeof(int), 1, file);
	// Write each string preceded by its lenght
	int i;
	for(i = 0; i < schema->numAttr; i++){
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
	short totalRecords = 0;
	short numPagesSchema = schemaSize/PAGE_SIZE + 1;
	short freeSpacePage = numPagesSchema;
	fseek(file, sizeof(SM_FileHeader), SEEK_SET);
	fwrite(&numPagesSchema, sizeof(short), 1, file);
	fwrite(&freeSpacePage, sizeof(short), 1, file);
	fwrite(&totalRecords, sizeof(short), 1, file);
	fclose(file);

	return RC_OK;
}


/**************************************************************************************************
 * Function Name: openTable
 * Description:
 *	Open the table with the name given.
 *
 * Parameters:
 *  RM_TableData *rel
 *	char         *name
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
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
	fread(&(tableHandle->totalRecords), sizeof(short), 1, file);

	fread(&(schema->numAttr), sizeof(int), 1, file);
	schema->attrNames = malloc(schema->numAttr * sizeof(char*));
	int i;
	for(i = 0; i < schema->numAttr;  i++){
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
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: closeTable
 * Description:
 *	Close the opened table.
 *
 * Parameters:
 *  RM_TableData *rel
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC closeTable (RM_TableData *rel){
	TableHandle *tableHandle = rel->mgmtData;
	// Store totalRecords
	FILE *file = fopen(rel->name, "r+");
	fseek(file, sizeof(SM_FileHeader) + 2*sizeof(short), SEEK_SET);
	fwrite(&(tableHandle->totalRecords), sizeof(short), 1, file);
	fclose(file);
	// End store totalRecords
	freeSchema(rel->schema);
	shutdownBufferPool(tableHandle->bm);
	free(tableHandle);
	return RC_OK;
}


/**************************************************************************************************
 * Function Name: deleteTable
 * Description:
 *	Delete the table.
 *
 * Parameters:
 *  char *name
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC deleteTable (char *name){
	destroyPageFile(name);
	//TODO: Free other resources if necessary
	return RC_OK;
}


/**************************************************************************************************
 * Function Name: getNumTuples
 * Description:
 *	Obtain the total number of records.
 *
 * Parameters:
 *  RM_TableData *rel
 *
 * Return:
 *	int: the number of tuples
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
int getNumTuples (RM_TableData *rel){
	TableHandle *tableHandle = rel->mgmtData;
	return tableHandle->totalRecords;
}


/**************************************************************************************************
 * Function Name: insertRecord
 * Description:
 *	Insert a new record in the table.
 *
 * Parameters:
 *  RM_TableData *rel
 *	Record       *record
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC insertRecord (RM_TableData *rel, Record *record){
	int page, newFreeSlot, recordOffset;
	int *freeSlot;
	char *pageData;
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	// READ PAGE FROM DISK IF IS NOT ALREADY IN BUFFER
	page = tableHandle->freeSpacePage;
	pinPage(tableHandle->bm, pageHandle, page);
	// LOOK FOR FREE SLOT
	freeSlot = (int *)pageHandle->data;
	pageData = pageHandle->data + sizeof(int);
	if(*freeSlot >= 0){
		pageData[*freeSlot*sizeof(char)] = FULL_SLOT; // SET SLOT TO FULL
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
	tableHandle->totalRecords++;
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: deleteRecord
 * Description:
 *	Delete the record from the table.
 *
 * Parameters:
 *  RM_TableData *rel
 *	RID          id
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC deleteRecord (RM_TableData *rel, RID id){
	int recordOffset;
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	pinPage (tableHandle->bm, pageHandle, id.page);

	pageHandle->data[sizeof(int) + id.slot*sizeof(char)] = 0x00;

	recordOffset = sizeof(int) + tableHandle->pageCap * sizeof(char) + id.slot*(tableHandle->recordSize + sizeof(RID));
	memset(pageHandle->data + recordOffset,0,tableHandle->recordSize + sizeof(RID)); // Set record to 0

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
		markDirty(tableHandle->bm, pageHandle);
		unpinPage(tableHandle->bm, pageHandle);
	}
	tableHandle->totalRecords--;
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: udpateRecord
 * Description:
 *	Update the attributes of a certain record.
 *
 * Parameters:
 *  RM_TableData *rel
 *	Record       *record
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
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


/**************************************************************************************************
 * Function Name: getRecord
 * Description:
 *	Retrieve a certain record.
 *
 * Parameters:
 *  RM_TableData *rel
 *  RID           id
 *	Record       *record
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC getRecord (RM_TableData *rel, RID id, Record *record){
	int recordOffset, code;
	TableHandle *tableHandle = rel->mgmtData;
	BM_PageHandle *pageHandle = malloc(sizeof(BM_PageHandle));
	pinPage (tableHandle->bm, pageHandle, id.page);
	if(pageHandle->data[sizeof(int) + id.slot*sizeof(char)] == 0x00){
		code = -1;
	}else{
		recordOffset = sizeof(int) + tableHandle->pageCap * sizeof(char) + id.slot*(tableHandle->recordSize + sizeof(RID)) + sizeof(RID);
		char *data = malloc(tableHandle->recordSize);
		memcpy(data, pageHandle->data + recordOffset, tableHandle->recordSize);
		unpinPage(tableHandle->bm, pageHandle);
		record->id = id;
		record->data = data;
		code = RC_OK;
	}
	return code;
}


/**************************************************************************************************
 * Function Name: startScan
 * Description:
 *	Initialize the scan.
 *
 * Parameters:
 *  RM_TableData  *rel
 *  RM_ScanHandle *scan
 *	Expr          *cond
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){

	TableHandle *tableHandle = rel->mgmtData;

	SM_FileHandle *fileHandle = (SM_FileHandle*)malloc(sizeof(SM_FileHandle));
	RM_ScanMgmt *scanMgmt = (RM_ScanMgmt*)malloc(sizeof(RM_ScanMgmt));

	openPageFile(rel->name, fileHandle);
	scanMgmt->totalNumPages = fileHandle->totalNumPages - tableHandle->numPagesSchema;
	closePageFile(fileHandle);
	scanMgmt->totalNumSlots = tableHandle->pageCap;
	scanMgmt->cond = cond;

	scanMgmt->id = (RID *) calloc(1,sizeof(RID));
	scanMgmt->id->page = tableHandle->numPagesSchema;
	scanMgmt->id->slot = 0;

	scan->rel = rel;
	scan->mgmtData = scanMgmt;

	return RC_OK;
}


/**************************************************************************************************
 * Function Name: next
 * Description:
 *	Retrieve the next record that matches with the expression given.
 *
 * Parameters:
 *  RM_ScanHandle *scan
 *	Record        *record
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC next (RM_ScanHandle *scan, Record *record){
	
	Value *result = malloc(sizeof(Value));
	RM_ScanMgmt* scanMgmt = scan->mgmtData;
	RID *id = scanMgmt->id;

	bool found = false;
	int i;
	for (i = id->page; i < scanMgmt->totalNumPages+1; i++){
		int j;
		for (j = id->slot; j < scanMgmt->totalNumSlots; j++){
			id->page = i;
			id->slot = j;
			if(getRecord (scan->rel, *id, record) == RC_OK){
				if(scanMgmt->cond) {
					evalExpr(record, scan->rel->schema, scanMgmt->cond, &result);
				}
				if(!scanMgmt->cond || (result->dt == DT_BOOL && result->v.boolV))
				{
					found = true;
					id->slot++;
					break;
				}
			}
		}
	}
	freeVal(result);


	if(found){
		return RC_OK;
	}else {
		return RC_RM_NO_MORE_TUPLES;
	}

}

/**************************************************************************************************
 * Function Name: closeScan
 * Description:
 *	Close the scan.
 *
 * Parameters:
 *  RM_ScanHandle *scan
 *
 * Return:
 *	RC: returned code
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC closeScan (RM_ScanHandle *scan){
	free(scan->mgmtData);
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: getRecordSize
 * Description:
 *	Retrieves the size that a record occupies.
 *
 * Parameters:
 *  RM_ScanHandle *scan
 *
 * Return:
 *	int: the size of the record.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
int getRecordSize (Schema *schema){
	int recordSize = 0;
	int i;
	for(i = 0; i < schema->numAttr && recordSize != RC_RM_UNKOWN_DATATYPE; i++){
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

/**************************************************************************************************
 * Function Name: createSchema
 * Description:
 *	Create a Schema struct with the attributes given.
 *
 * Parameters:
 *  int numAttr
 *	char **attrNames
 *	DataType *dataTypes
 *	int *typeLength
 *  int keySize
 *  int *keys	
 *
 * Return:
 *	Schema: the schema struct.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
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

/**************************************************************************************************
 * Function Name: freeSchema
 * Description:
 *	Releases the memory occupied by the schema.
 *
 * Parameters:
 *	Schema *schema
 *
 * Return:
 *	RC: returned code.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC freeSchema (Schema *schema){
	free(schema->attrNames);
	free(schema->dataTypes);
	free(schema->typeLength);
	free(schema->keyAttrs);
	free(schema);
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: createRecord
 * Description:
 *	Allocates the necessary space in memory of a record.
 *
 * Parameters:
 *	Record **record
 *	Schema *schema
 *
 * Return:
 *	RC: returned code.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC createRecord (Record **record, Schema *schema){
	*record = malloc(sizeof(Record));
	(*record)->data = calloc(1, getRecordSize(schema));
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: freeRecord
 * Description:
 *	Releases the memory occupied by a record.
 *
 * Parameters:
 *	Record **record
 *
 * Return:
 *	RC: returned code.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
RC freeRecord (Record *record){
	free(record->data);
	free(record);
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: getAttr
 * Description:
 *	Retrieve the certain attribute from a record.
 *
 * Parameters:
 *	Record **record
 *	Schema *schema
 *  int attrNum
 *  Value **value
 *
 * Return:
 *	RC: returned code.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
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

/**************************************************************************************************
 * Function Name: setAttr
 * Description:
 *	Set the certain attribute from a record.
 *
 * Parameters:
 *	Record **record
 *	Schema *schema
 *  int attrNum
 *  Value **value
 *
 * Return:
 *	RC: returned code.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
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

/**************************************************************************************************
 * Function Name: getAttrOffset
 * Description:
 *	Return the relative offset of the attribute in a record.
 *
 * Parameters:
 *	Schema *schema
 *  int attrNum
 *
 * Return:
 *	int: the offset.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
int getAttrOffset(Schema *schema, int attrNum){
	int attrOffset = 0;
	int i;
	for(i = 0; i < attrNum && attrOffset >= 0; i++){
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

/**************************************************************************************************
 * Function Name: getNumPagesSchema
 * Description:
 *	Return the number of pages that occupies the schema.
 *
 * Parameters:
 *  char *name
 *
 * Return:
 *	short: the number of pages.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
short getNumPagesSchema(char *name){
  	FILE *file = fopen(name, "r+");
  	fseek(file, sizeof(SM_FileHeader), SEEK_SET);
  	short tableInfoSize;
  	fread(&tableInfoSize, sizeof(short), 1, file);
  	return tableInfoSize;
}

/**************************************************************************************************
 * Function Name: calculatePageCap
 * Description:
 *	Return the number of records that fits into a single page.
 *
 * Parameters:
 *  Schema *schema
 *
 * Return:
 *	int: number of records.
 *
 * Author:
 *	Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *	Date        Name                                              Content
 *	----------  ------------------------------------------------  ------------------------------
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Initialization.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *	2016-11-10  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Add header comment,
 *                                                                  	add comments.
**************************************************************************************************/
int calculatePageCap(Schema *schema){
	int recordSize, numRecords;
	recordSize = getRecordSize(schema) + (int)sizeof(RID);
	// -1 is for free slot pointer
	numRecords = (PAGE_SIZE - 1)/(sizeof(char) + recordSize);
	return numRecords;
}