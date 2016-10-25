#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**************************************************************************************************
 * Function Name: initBufferPool
 * Description:
 *    Create a buffer pool and initializes its management data
 *
 * Parameters:
 *    BM_BufferPool *const bm
 *		const char *const pageFileName
 * 		const int numPages
 *		ReplacementStrategy strategy
 *		oid *stratData
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData){
	SM_FileHandle fileHandle;
	char *pageFileName2 = strdup(pageFileName); // Avoid 'discard const' warning.
	if(openPageFile (pageFileName2, &fileHandle) == RC_FILE_NOT_FOUND) {
		return RC_FILE_NOT_FOUND;
	} else {
		//Initializes buffer structure (defined in buffer_mgr.h)
		BM_Buffer *buffer = malloc(sizeof(BM_Buffer));
		// Initializes buffer management arrays
		SM_PageHandle *frameBuffer = malloc(PAGE_SIZE * numPages);
		PageNumber *pageIndex = malloc(sizeof(int)*numPages);
		bool *dirtyFlags = malloc(sizeof(bool)*numPages);
		int *fixCount = malloc(sizeof(int)*numPages);
		long *lastUseTime = malloc(sizeof(long)*numPages);
		// Gives starting values for each management array
		for(int i = 0; i < numPages;i++){
			pageIndex[i] = NO_PAGE;
			dirtyFlags[i] = FALSE;
			fixCount[i] = 0;
			lastUseTime[i] = -1; 
		}
		// Save data in structure
		buffer->frameBuffer = frameBuffer;
		buffer->pageIndex = pageIndex;
		buffer->dirtyFlags = dirtyFlags;
		buffer->fixCount = fixCount;
		buffer->insertPos = 0;
		buffer->timeCounter = 0;
		buffer->lastUseTime = lastUseTime;

		// Save and initialize our aux buffer pool management data
		BM_Mgmtdata *mgmtData = malloc(sizeof(BM_Mgmtdata));
		mgmtData->fileHandle = fileHandle;
		mgmtData->buffer = buffer;
		mgmtData->numReadIO = 0;
		mgmtData->numWriteIO = 0;
		// Save and initialize buffer pool management data for given structure
		bm->pageFile = pageFileName2;
		bm->numPages = numPages;
		bm->strategy = strategy;
		bm->mgmtData = mgmtData;
		return RC_OK;
	}
}

/**************************************************************************************************
 * Function Name: shutdownBufferPool
 * Description:
 *    Destroy buffer pool
 *
 * Parameters:
 *    BM_BufferPool *const bm
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
RC shutdownBufferPool(BM_BufferPool *const bm){
	// Forces all dirty pages to be writed in disk
	return forceFlushPool(bm);
}

/**************************************************************************************************
 * Function Name: forceFlushPool
 * Description:
 *    Writes all pages marked as dirty to disk.
 *
 * Parameters:
 *    BM_BufferPool *const bm
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
RC forceFlushPool(BM_BufferPool *const bm){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	// Checks what frames in buffer are marked as dirty
	for(int i=0;i < bm->numPages;i++){
		if(buffer->dirtyFlags[i] == TRUE){
			// and writes them to disk.
			int err = writeBlock (buffer->pageIndex[i], &mgmtData->fileHandle, buffer->frameBuffer[i]);
			if(err != RC_OK) return err;
			// setting flag to false
			buffer->dirtyFlags[i] = FALSE;
			// and increasing writing count
			mgmtData->numWriteIO++;
		}
	}
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: markDirty
 * Description:
 *    Mark a page in buffer frame as dirty
 *
 * Parameters:
 *    	BM_BufferPool *const bm
 *		BM_PageHandle *const page
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	//Search page position in buffer
	int index = findPageIndex(page->pageNum, bm->numPages,buffer->pageIndex);
	// and sets its flag to true
	buffer->dirtyFlags[index] = TRUE;
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: unpinPage
 * Description:
 *      Remove pin between page frame and client.
 *
 * Parameters:
 *    	BM_BufferPool *const bm
 *		BM_PageHandle *const page
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *fifoBuffer = mgmtData->buffer;
	//Search page position in buffer
	int index = findPageIndex(page->pageNum, bm->numPages,fifoBuffer->pageIndex);
	// And decrease fix count
	if(index != NO_PAGE) fifoBuffer->fixCount[index]--;

	return RC_OK;
}

/**************************************************************************************************
 * Function Name: forcePage
 * Description:
 *      Write page from buffer frame to file in disk.
 *
 * Parameters:
 *    	BM_BufferPool *const bm
 *		BM_PageHandle *const page
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	//Search page position in buffer
	int index = findPageIndex(page->pageNum, bm->numPages,buffer->pageIndex);
	// Writes it to disk
	int err = writeBlock (page->pageNum, &mgmtData->fileHandle, buffer->frameBuffer[index]);
	if(err != RC_OK) return err;
	// increases writing count
	mgmtData->numWriteIO++;
	// and sets dirty flag to true 
	buffer->dirtyFlags[index] = FALSE;
	return RC_OK;
}

/**************************************************************************************************
 * Function Name: pinPage
 * Description:
 *      Read page from disk if it is not already in buffer.
 *
 * Parameters:
 *    	BM_BufferPool *const bm
 *		BM_PageHandle *const page
 * 		const PageNumber pageNum
 *
 * Return:
 *    RC: returned code
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	//Search page position in buffer
	int index = findPageIndex(pageNum, bm->numPages,buffer->pageIndex);
	if(index == NO_PAGE){
		// If it is not already in buffer
		int insertionIndex = -1;
		// Look for insert position according to replacement strategy.
		// Discarding if frame is being used.
		if(bm->strategy == RS_LRU){
			insertionIndex = searchLowerTime(buffer->lastUseTime, buffer->fixCount, bm->numPages);
		}else if(bm->strategy == RS_FIFO){
			insertionIndex = searchInsertPosition(buffer->insertPos,buffer->fixCount, bm->numPages);
		}
		if(insertionIndex < 0) return RC_OK; // DEFINE NEW CODE FOR THIS?

		// If page to be replace is marked as dirty 
		if(buffer->dirtyFlags[insertionIndex] == TRUE){
			int err = writeBlock (buffer->pageIndex[insertionIndex], &mgmtData->fileHandle, buffer->frameBuffer[insertionIndex]);
			if(err != RC_OK) return err;
			mgmtData->numWriteIO++;
		}
		// If trying to read a page that is not in file yet
		if((mgmtData->fileHandle).totalNumPages <=  pageNum){
			// extends capacity of file to allocate it.
			ensureCapacity(pageNum + 1, &mgmtData->fileHandle);
		}
		// Read block from file
		SM_PageHandle memPage = malloc(PAGE_SIZE);
		RC code = readBlock (pageNum, &mgmtData->fileHandle, memPage);
		if(code != RC_OK) return code;
		// Increases reading counter
		mgmtData->numReadIO++;
		// Save read page data in BM_PageHandle structure
		page->pageNum = pageNum;
		page->data = memPage;

		// Inserts page in buffer frames and updates management data
		buffer->frameBuffer[insertionIndex] = page->data;
		buffer->pageIndex[insertionIndex] = page->pageNum;
		buffer->dirtyFlags[insertionIndex] = FALSE;
		buffer->fixCount[insertionIndex] = 1;
		buffer->insertPos = (insertionIndex + 1)%bm->numPages;
		buffer->lastUseTime[insertionIndex] = buffer->timeCounter++;
	}else{
		// If it is already in buffer
		// Save read page data in BM_PageHandle structure
		page->pageNum = pageNum;
		page->data = buffer->frameBuffer[index];
		// Increases fix count for this frame in buffer
		buffer->fixCount[index]++;
		// If replacement strategy is LRU, updates time counter.
		if(bm->strategy == RS_LRU) buffer->lastUseTime[index] = buffer->timeCounter++;
	}
	return RC_OK;

};

/**************************************************************************************************
 * Function Name: getFrameContents
 * Description:
 *      Returns an array of PageNumberswhere the ith element is the number of the page stored in the ith page frame.
 *
 * Parameters:
 *    	BM_BufferPool *const bm
 *
 * Return:
 *    PageNumber*: array of page numbers
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
PageNumber *getFrameContents (BM_BufferPool *const bm){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	// return our pageIndex from buffer 
	return (PageNumber *)buffer->pageIndex;
}

/**************************************************************************************************
 * Function Name: getDirtyFlags
 * Description:
 *      Returns an array of bools where the ith element is TRUE if the page stored in the ith page frame is dirty.
 *
 * Parameters:
 *    	BM_BufferPool *const bm
 *
 * Return:
 *    bool*: array of dirty flags
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
bool *getDirtyFlags (BM_BufferPool *const bm){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	// return our dirtyFlags from buffer 
	return buffer->dirtyFlags;
}

/**************************************************************************************************
 * Function Name: getFixCounts
 * Description:
 *      Returns an array of ints where the ith element is the fix count of the page stored in the ith page frame.
 *
 * Parameters:
 *    	BM_BufferPool *const bm
 *
 * Return:
 *    int*: array of fix count.
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
int *getFixCounts (BM_BufferPool *const bm){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	// return our fixCount from buffer
	return buffer->fixCount;
}

/**************************************************************************************************
 * Function Name: getNumReadIO
 * Description:
 *      Returns the number of pages that have been read from disk since a buffer pool has been initialized.
 *
 * Parameters:
 *    	BM_BufferPool *const bm
 *
 * Return:
 *    int: count of read operations.
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
int getNumReadIO (BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	// return out reading counter from buffer pool management data
	return mgmtData->numReadIO;
}

/**************************************************************************************************
 * Function Name: getNumWriteIO
 * Description:
 *      Returns the number of pages that have been writed to disk since a buffer pool has been initialized.
 *
 * Parameters:
 *    	BM_BufferPool *const bm
 *
 * Return:
 *    int: count of write operations.
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-10  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
int getNumWriteIO (BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	// return out reading counter from buffer pool management data
	return mgmtData->numWriteIO;
}

/**************************************************************************************************
 * Function Name: findPageIndex
 * Description:
 *      Returns the index of a page inside frame buffer.
 *
 * Parameters:
 *    	int numPage  
 *		int totalPages
 *		PageNumber *pageIndex
 *
 * Return:
 *    int: index of page in frame buffer. -1 if is not in buffer.
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-15  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
int findPageIndex (int numPage, int totalPages, PageNumber *pageIndex){
	PageNumber position = NO_PAGE;
	for(int i = 0; i < totalPages && position < 0;i++){
		if (numPage == pageIndex[i]){
			position = i;
		}
	}
	return position;
}

/**************************************************************************************************
 * Function Name: searchInsertPosition
 * Description:
 *      Returns the insert position in a FIFO buffer discarding fixed.
 *
 * Parameters:
 *    	int currentPos
 *		int *fixCount
 *		int totalPages
 *
 * Return:
 *    int: index of frame in buffer to write page. Returns -1 if is not space available.
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-25  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
int searchInsertPosition(int currentPos, int *fixCount, int totalPages){
	int insertPosition = -1;
	for(int i = currentPos; (i < totalPages + currentPos && (insertPosition < 0)); i++){
		if(fixCount[i] == 0) insertPosition = i;
	}
	return insertPosition;
}

/**************************************************************************************************
 * Function Name: searchLowerTime
 * Description:
 *      Returns the insert position in a FIFO buffer discarding fixed.
 *
 * Parameters:
 *    	long *lastUseTime
 *		int *fixCount
 *		int totalPages
 *
 * Return:
 *    int: index of frame in buffer to write page. Returns -1 if is not space available.
 *
 * Author:
 *    Victor Portals <vportalslorenzo@hawk.iit.edu>
 *
 * History:
 *    Date        Name                                              Content
 *    ----------  ------------------------------------------------  ------------------------------
 *    2016-10-24  Victor Portals     <vportalslorenzo@hawk.iit.edu>     Initialization.
**************************************************************************************************/
int searchLowerTime(long *lastUseTime, int *fixCount, int totalPages){
	int lowerIndex = 0;
	for(int i = 1; i < totalPages; i++){
		if(lastUseTime[lowerIndex] > lastUseTime[i] && fixCount[i] == 0) lowerIndex = i;
	}
	return fixCount[lowerIndex] == 0 ? lowerIndex : -1;
}

