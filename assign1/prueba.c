#include "dberror.h"
#include "storage_mgr.h"

#include <stdio.h>

typedef struct SM_FileHeader {
  int totalNumPages;
  int curPagePos;
} SM_FileHeader;

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

int main (void)
{
  createPageFile("hola_k_ase.txt");
  struct SM_FileHandle fHandle;
  if(openPageFile("hola_k_ase.txt", &fHandle) == RC_FILE_NOT_FOUND){
    printf("NO ENCONTRADO\n");
  }else{
    printf("fileName: %s, totalNumPages: %i, curPagePos: %i\n", fHandle.fileName, fHandle.totalNumPages, fHandle.curPagePos);
  }
  closePageFile(&fHandle);
  destroyPageFile("hola_k_ase.txt");
  return 0;
}