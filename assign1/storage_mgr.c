#include "dberror.h"
#include "storage_mgr.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern int errno;

RC updateCurrentInformation(int totalNumPages, int curPagePos, SM_FileHandle *fHandle);

typedef struct SM_FileHeader {
  int totalNumPages;
  int curPagePos;
} SM_FileHeader;

void initStorageManager(){

};

/**************************************************************************************************
 * Function Name: createPageFile
 * Description:
 *    Create a page file and write empty content. ('/0')
 *
 * Parameters:
 *    char *fileName: file name
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Main logic.
 *    2016-09-24  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Error handle.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
 *    2016-09-27  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Fixed error handle.
**************************************************************************************************/
RC createPageFile (char *fileName){
  FILE *file = fopen(fileName, "w");
  if(file){
    char eof = '\0';

    struct SM_FileHeader fileInfo;
    fileInfo.totalNumPages = 1;
    fileInfo.curPagePos = 0;

    // SM_FileHeader struct will be stored at the start of the document
    // It will only contains totalNumPages and curPagePos

    errno = 0;
    fwrite(&fileInfo, sizeof(SM_FileHeader), 1, file);
    if(errno != 0){
      fclose(file);
      return RC_WRITE_FAILED;
    }

    // We writes the first page full of '\0'
    for (int i = 0; i < PAGE_SIZE; ++i){
      errno = 0;
      fwrite(&eof, sizeof(eof), 1, file);
      if(errno != 0){
        fclose(file);
        return RC_WRITE_FAILED;
      }
    }

    // file is closed
    errno = 0;
    fclose(file);
    if(errno == 0)
      return RC_OK;
    else return RC_WRITE_FAILED;
  }else{
    return RC_FILE_HANDLE_NOT_INIT;
  }
};


/**************************************************************************************************
 * Function Name: openPageFile
 * Description:
 *    Opens the page file.
 *
 * Parameters:
 *    char *fileName        : file name
 *    SM_FileHandle *fHandle: output file handle
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Main Logic.
 *    2016-09-25  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Error handle.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC openPageFile (char *fileName, SM_FileHandle *fHandle){

  // file is opened
  FILE *file = fopen(fileName, "r+");
  struct SM_FileHeader fileInfo;

  if (file){
    // we read first the metadata at the start of the file
    errno = 0;
    fread(&fileInfo, sizeof(SM_FileHeader), 1, file);
    if(errno != 0) return RC_FILE_HANDLE_NOT_INIT;

    // now we initialize the SM_FileHandle struct
    fHandle->fileName = fileName;
    fHandle->totalNumPages = fileInfo.totalNumPages;
    fHandle->curPagePos = fileInfo.curPagePos;
    fHandle->mgmtInfo = file;
    // we are using mgmtInfo to store the file handle needed
    // to write, read, close and other I/O operations
    return RC_OK;
  }else{
    return RC_FILE_NOT_FOUND;
  }
};

/**************************************************************************************************
 * Function Name: closePageFile
 * Description:
 *    Closes the page file.
 *
 * Parameters:
 *    SM_FileHandle *fHandle: file handle
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Main Logic.
 *    2016-09-25  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Error handle.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Added header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC closePageFile (SM_FileHandle *fHandle){
  errno = 0;
  fclose(fHandle->mgmtInfo);
  if(errno != 0) return RC_FILE_HANDLE_NOT_INIT;
  return RC_OK;
};

/**************************************************************************************************
 * Function Name: destroyPageFile
 * Description:
 *    Removes the file.
 *
 * Parameters:
 *    char *fileName: file name
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main Logic.
 *    2016-09-22  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Error handle.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC destroyPageFile (char *fileName){
  errno = 0;
  remove(fileName);
  if(errno != 0) return RC_FILE_NOT_FOUND;
  return RC_OK;
};

/**************************************************************************************************
 * Function Name: readBlock
 * Description:
 *    Reads specified page number.
 *
 * Parameters:
 *    int pageNum           : page number to be read
 *    SM_FileHandle *fHandle: file handle
 *    SM_PageHandle memPage : where the block will be stored after read
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Main logic
 *    2016-09-25  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Error Handle.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
  // Checks if pageNum is inside the bounds
  if(pageNum < 0 || pageNum >= fHandle->totalNumPages)
    return RC_READ_NON_EXISTING_PAGE;

  // we use fseek to move the read pointer of our file
  // where we want to start reading
  errno = 0;
  fseek(fHandle->mgmtInfo, sizeof(SM_FileHeader)+pageNum*PAGE_SIZE, SEEK_SET);
  if(errno != 0) return RC_FILE_HANDLE_NOT_INIT;
  errno = 0;
  fread(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo);
  if(errno != 0) return RC_FILE_HANDLE_NOT_INIT;

  // this function will update metadata in fHandle struct and
  // it will write the metadata to the file
  updateCurrentInformation(fHandle->totalNumPages, pageNum, fHandle);
  return RC_OK;
};

/**************************************************************************************************
 * Function Name: getBlockPos
 * Description:
 *    Gets current page position.
 *
 * Parameters:
 *    SM_FileHandle *fHandle: file handle
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Main logic.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Error handle. 
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
int getBlockPos (SM_FileHandle *fHandle){
  if(fHandle)
    return fHandle->curPagePos;
  else return RC_FILE_HANDLE_NOT_INIT;
};

/**************************************************************************************************
 * Function Name: readFirstBlock
 * Description:
 *    Reads first page.
 *
 * Parameters:
 *    SM_FileHandle *fHandle: file handle
 *    SM_PageHandle memPage : where the block will be stored after read
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Main logic.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  // first block has pageNum = 0
  return readBlock(0, fHandle, memPage);
};

/**************************************************************************************************
 * Function Name: readLastBlock
 * Description:
 *    Reads last page in memory.
 *
 * Parameters:
 *    SM_FileHandle *fHandle: file handle
 *    SM_PageHandle memPage : where the block will be stored after read
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  // Last block has pageNum = totalNumPages
  // readBlock function will check if pageNum is inside the bounds
  return readBlock(fHandle->totalNumPages-1, fHandle, memPage);
};

/**************************************************************************************************
 * Function Name: readPreviousBlock
 * Description:
 *    Reads previous page to current.
 *
 * Parameters:
 *    SM_FileHandle *fHandle: file handle
 *    SM_PageHandle memPage : where the block will be stored after read
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Main logic.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  // Previous block has pageNum = curPagePos-1
  // readBlock function will check if pageNum is inside the bounds
  return readBlock(fHandle->curPagePos-1, fHandle, memPage);
};

/**************************************************************************************************
 * Function Name: readCurrentBlock
 * Description:
 *    Reads current page in memory.
 *
 * Parameters:
 *    SM_FileHandle *fHandle: file handle
 *    SM_PageHandle memPage : where the block will be stored after read
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main Logic.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  // Current block has pageNum = curPagePos
  // readBlock function will check if pageNum is inside the bounds
  return readBlock(fHandle->curPagePos, fHandle, memPage);
};

/**************************************************************************************************
 * Function Name: readNextBlock
 * Description:
 *    Reads next page to current.
 *
 * Parameters:
 *    SM_FileHandle *fHandle: file handle
 *    SM_PageHandle memPage : where the block will be stored after read
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  // Next block has pageNum = curPagePos+1
  // readBlock function will check if pageNum is inside the bounds
  return readBlock(fHandle->curPagePos+1, fHandle, memPage);
};

/**************************************************************************************************
 * Function Name: writeBlock
 * Description:
 *    Writes page read from memory to page file.
 *
 * Parameters:
 *    int pageNum           : number of page to be writed
 *    SM_FileHandle *fHandle: file handle
 *    SM_PageHandle memPage : where the block will be stored after read
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main logic.
 *    2016-09-25  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Error handle.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
 *    2016-09-27  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Fixed error handle.
**************************************************************************************************/
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
  // Checks if pageNum is inside the bounds
  if(pageNum < 0 || pageNum >= fHandle->totalNumPages)
    return RC_READ_NON_EXISTING_PAGE;

  // we use fseek to move the read pointer of our file
  // where we want to start writing
  errno = 0;
  fseek(fHandle->mgmtInfo, sizeof(SM_FileHeader)+pageNum*PAGE_SIZE, SEEK_SET);
  if(errno != 0) return RC_FILE_HANDLE_NOT_INIT;


  errno = 0;
  fwrite(memPage, PAGE_SIZE, 1, fHandle->mgmtInfo);
  if(errno != 0) return RC_WRITE_FAILED;
  return RC_OK;
};

/**************************************************************************************************
 * Function Name: writeCurrentBlock
 * Description:
 *    Writes page read from memory to current page.
 *
 * Parameters:
 *    SM_FileHandle *fHandle: file handle
 *    SM_PageHandle memPage : where the block will be stored after read
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Main logic
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  // Current block has pageNum = curPagePos
  // readBlock function will check if pageNum is inside the bounds
  return writeBlock(fHandle->curPagePos, fHandle, memPage);
};

/**************************************************************************************************
 * Function Name: appendEmptyBlock
 * Description:
 *    Writes empty page to the end of page file.
 *
 * Parameters:
 *    SM_FileHandle *fHandle: file handle
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization..
 *    2016-09-21  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Main logic.
 *    2016-09-25  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Error handle.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC appendEmptyBlock (SM_FileHandle *fHandle){
  // the file is opened as 'r+'
  // we need to close and reopen as 'a'
  fclose(fHandle->mgmtInfo);
  FILE *file = fopen(fHandle->fileName, "a");
  if(file){

    char eof = '\0';
    // we append a new page full of '\0' character
    for(int i = 0; i < PAGE_SIZE; i++){
      errno = 0;
      fwrite(&eof, sizeof(eof), 1, file);
      if(errno != 0) return RC_WRITE_FAILED;
    }

    // close file
    errno = 0;
    fclose(file);
    if(errno != 0) return RC_WRITE_FAILED;
    // we reopen again as 'r+'
    openPageFile(fHandle->fileName, fHandle);
    // we need to update metadata information, totalNumPages has been increased
    updateCurrentInformation(fHandle->totalNumPages+1, fHandle->curPagePos, fHandle);
    return RC_OK;
  }else{
    return RC_FILE_NOT_FOUND;
  }
};

/**************************************************************************************************
 * Function Name: ensureCapacity
 * Description:
 *    Writes empty pages while page file has not a given number of them.
 *
 * Parameters:
 *    int numberOfPages     : number of pages that page file should have
 *    SM_FileHandle *fHandle: file handle
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Main Logic,
 *    2016-09-25  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Error handle.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
  // how many pages we need to append
  int numberToEnsure = numberOfPages - fHandle->totalNumPages;
  // if the number is negative or 0, we dont need to append any page
  if(numberToEnsure > 0){
    for(int i = 0; i < numberToEnsure; i++){
      RC res = appendEmptyBlock(fHandle);
      if(res != RC_OK) return res;
    }
  }
  return RC_OK;
};

/**************************************************************************************************
 * Function Name: updateCurrentInformation
 * Description:
 *    Auxiliar functions that updates the file's metadata
 *
 * Parameters:
 *    int totalNumPages    : number of pages of current document
 *    int curPagePos       : current page position
 *    SM_FILEHANDLE fHandle: file handle
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Jose Carmona <jcarmonalopez@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-09-20  Jose Carmona     <jcarmonalopez@hawk.iit.edu>     Initialization.
 *    2016-09-21  Victor Portals   <vportalslorenzo@hawk.iit.edu>   Main Logic.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Error handle.
 *    2016-09-26  Sergio Penavades <spenavadessuarez@hawk.iit.edu>  Add header comment,
 *                                                                  add comments.
**************************************************************************************************/

RC updateCurrentInformation(int totalNumPages, int curPagePos, SM_FileHandle *fHandle){
  // fHandle is updated
  fHandle->totalNumPages = totalNumPages;
  fHandle->curPagePos = curPagePos;

  // a SM_FileHeader struct is created to be written to the file
  struct SM_FileHeader fileInfo;
  fileInfo.totalNumPages = fHandle->totalNumPages;
  fileInfo.curPagePos = fHandle->curPagePos;

  // we want to write at the start of the file
  errno = 0;
  fseek(fHandle->mgmtInfo, 0, SEEK_SET);
  if(errno != 0) return RC_FILE_HANDLE_NOT_INIT;
  errno = 0;
  fwrite(&fileInfo, sizeof(fileInfo), 1, fHandle->mgmtInfo);
  if(errno != 0) return RC_WRITE_FAILED;
  return RC_OK;
}