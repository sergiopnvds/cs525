#include "dberror.h"
#include "storage_mgr.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern int errno;

typedef struct SM_FileHeader {
  int totalNumPages;
  int curPagePos;
} SM_FileHeader;

void initStorageManager(){

}


void updateCurrentInformation(int totalNumPages, int curPagePos, SM_FileHandle *fHandle){
  fHandle->totalNumPages = totalNumPages;
  fHandle->curPagePos = curPagePos;
  struct SM_FileHeader fileInfo;
  fileInfo.totalNumPages = fHandle->totalNumPages;
  fileInfo.curPagePos = fHandle->curPagePos;
  fseek(fHandle->mgmtInfo, 0, SEEK_SET);
  fwrite(&fileInfo, sizeof(fileInfo), 1, fHandle->mgmtInfo);
}

RC createPageFile (char *fileName){
  FILE *file = fopen(fileName, "w");
  
  char eof = '\0';

  struct SM_FileHeader fileInfo;
  fileInfo.totalNumPages = 1;
  fileInfo.curPagePos = 0;

  fwrite(&fileInfo, sizeof(SM_FileHeader), 1, file);
  for (int i = 0; i < PAGE_SIZE; ++i)
    fwrite(&eof, sizeof(eof), 1, file);
  fclose(file);
  
  return RC_OK;
};

RC openPageFile (char *fileName, SM_FileHandle *fHandle){
  FILE *file = fopen(fileName, "r+");
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
  fseek(fHandle->mgmtInfo, sizeof(SM_FileHeader)+pageNum*PAGE_SIZE, SEEK_SET);
  fread(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo);
  updateCurrentInformation(fHandle->totalNumPages, pageNum, fHandle);
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
  fseek(fHandle->mgmtInfo, sizeof(SM_FileHeader)+pageNum*PAGE_SIZE, SEEK_SET);
  fwrite(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo);
  return RC_OK;
};


RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  return writeBlock(fHandle->curPagePos, fHandle, memPage);
};

RC appendEmptyBlock (SM_FileHandle *fHandle){
  fclose(fHandle->mgmtInfo);
  FILE *file = fopen(fHandle->fileName, "a");
  char eof = '\0';
  for(int i = 0; i < PAGE_SIZE; i++)
    fwrite(&eof, sizeof(eof), 1, file);
  fclose(file);
  openPageFile(fHandle->fileName, fHandle);
  updateCurrentInformation(fHandle->totalNumPages+1, fHandle->curPagePos, fHandle);
  return RC_OK;
};

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
  int numberToEnsure = numberOfPages - fHandle->totalNumPages;
  if(numberToEnsure > 0){
    for(int i = 0; i < numberToEnsure; i++)
      appendEmptyBlock(fHandle);
  }
  return RC_OK;
};