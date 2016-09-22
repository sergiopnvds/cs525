#include "dberror.h"
#include "storage_mgr.h"

#include <stdio.h>

void initStorageManager (){
	return;
};

RC createPageFile (char *fileName){
  FILE *file = fopen(fileName, "w");
  
  int i = 0;
  char eof = '\0';

  struct SM_FileHeader fileInfo;
  fileInfo.totalNumPages = 1;
  fileInfo.curPagePos = 0;

  fwrite(&fileInfo, sizeof(SM_FileHeader), 1, file);
  
  for(i = sizeof(SM_FileHeader); i < PAGE_SIZE + sizeof(SM_FileHeader); i++)
    fwrite(&eof, sizeof(eof), 1, file);
  
  fclose(file);
  
  return RC_OK;
};

RC openPageFile (char *fileName, SM_FileHandle *fHandle){
  FILE *file = fopen(fileName, "r");
  struct SM_FileHeader fileInfo;
  if (file){
    fread(&fileInfo, sizeof(SM_FileHeader), 1, file);
    fHandle->fileName = fileName;
    fHandle->totalNumPages = fileInfo.totalNumPages;
    fHandle->curPagePos = fileInfo.curPagePos;
    fHandle->mgmtInfo = file;
    fclose(file);
    return RC_OK;
  }else{
    fclose(file);
    return RC_FILE_NOT_FOUND;
  }
};

RC closePageFile (SM_FileHandle *fHandle){
  fclose(fHandle->mgmtInfo);
  return RC_OK;
};

RC destroyPageFile (char *fileName){
  remove(fileName);
  return RC_OK;
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

