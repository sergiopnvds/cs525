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
  fwrite(&eof, sizeof(eof), PAGE_SIZE, file);
  
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
    return RC_OK;
  }else{
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
  if(pageNum < 0 || pageNum >= fHandle->totalNumPages)
    return RC_READ_NON_EXISTING_PAGE;
  memPage = malloc(PAGE_SIZE);
  int offset = pageNum - fHandle->curPagePos;
  fread(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo + offset*PAGE_SIZE);
  return RC_OK;
};

int getBlockPos (SM_FileHandle *fHandle){
	return fHandle->curPagePos;
};

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	return readBlock(0, fHandle, memPage);
};

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	return readBlock(fHandle->totalNumPages-1, fHandle, memPage);
};

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	return readBlock(fHandle->curPagePos-1, fHandle, memPage);
};
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	return readBlock(fHandle->curPagePos, fHandle, memPage);
};
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	return readBlock(fHandle->curPagePos+1, fHandle, memPage);
};

/* writing blocks to a page file */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
  if(pageNum < 0 || pageNum >= fHandle->totalNumPages)
    return RC_READ_NON_EXISTING_PAGE;
  int offset = pageNum - fHandle->curPagePos;
  fwrite(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo + offset*PAGE_SIZE);
};


RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	return writeBlock(fHandle->curPagePos, fHandle, memPage);
};

RC appendEmptyBlock (SM_FileHandle *fHandle){
  int i = 0;
  char eof = '\0';
  fwrite(&eof, sizeof(eof), PAGE_SIZE, fHandle->mgmtInfo);
};

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
	int numberToEnsure = numberOfPages - fHandle->totalNumPages;
	if(numberToEnsure > 0){
		int i = 0;
		for(i = 0; i < numberToEnsure; i++)
			appendEmptyBlock(fHandle);
	}
};

