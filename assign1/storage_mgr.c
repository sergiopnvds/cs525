#include "dberror.h"
#include "storage_mgr.h"

#include <stdio.h>

void initStorageManager (){
	return;
};

RC createPageFile (char *fileName){
	FILE *file = fopen(filename, "w");
	int i = 0;
	char eof = '\0';
	for(i = 0; i <= PAGE_SIZE; i++)
		fwrite(&eof, sizeof(eof), 1, file);
	fclose(file);
};

RC openPageFile (char *fileName, SM_FileHandle *fHandle){
	RC dummy = 0;
	return dummy;
};
RC closePageFile (SM_FileHandle *fHandle){
	RC dummy = 0;
	return dummy;
};
RC destroyPageFile (char *fileName){
	RC dummy = 0;
	return dummy;
};

/* reading blocks from disc */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC dummy = 0;
	return dummy;
};
int getBlockPos (SM_FileHandle *fHandle){
	int dummy = 0;
	return dummy;
};
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC dummy = 0;
	return dummy;
};
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC dummy = 0;
	return dummy;
};
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC dummy = 0;
	return dummy;
};
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC dummy = 0;
	return dummy;
};
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC dummy = 0;
	return dummy;
};

/* writing blocks to a page file */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC dummy = 0;
	return dummy;
};
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC dummy = 0;
	return dummy;
};
RC appendEmptyBlock (SM_FileHandle *fHandle){
	RC dummy = 0;
	return dummy;
};
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
	RC dummy = 0;
	return dummy;
};

