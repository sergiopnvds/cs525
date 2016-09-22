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

  
  printf("HOLA1\n");
  SM_FileHeader *fileInfo;
  printf("HOLA2\n");
  fileInfo->totalNumPages = 1;
  printf("HOLA3\n");
  fileInfo->curPagePos = 0;
  printf("HOLA4\n");


  printf("sizeof(SM_FileHeader): %lu\n", sizeof(SM_FileHeader));


  fwrite(&fileInfo, sizeof(SM_FileHeader), 1, file);
  
  for(i = sizeof(SM_FileHeader); i < PAGE_SIZE + sizeof(SM_FileHeader); i++)
    fwrite(&eof, sizeof(eof), 1, file);
  
  fclose(file);
  
  return RC_OK;
};

RC openPageFile (char *fileName, SM_FileHandle *fHandle){
  FILE *file = fopen(fileName, "r");
  SM_FileHeader *fileInfo;
  if (file){
    fread(&fileInfo, sizeof(SM_FileHeader), 1, file);
    fHandle->fileName = fileName;
    fHandle->totalNumPages = fileInfo->totalNumPages;
    fHandle->curPagePos = fileInfo->curPagePos;
    return RC_OK;
  }else{
    return RC_FILE_NOT_FOUND;
  }
};


int main (void)
{
  createPageFile("hola_k_ase.txt");
  SM_FileHandle *fHandle;
  openPageFile("hola_k_ase.txt", fHandle);
  printf("fileName: %s, totalNumPages: %i, curPagePos: %i", fHandle->fileName, fHandle->totalNumPages, fHandle->curPagePos);
  return 0;
}