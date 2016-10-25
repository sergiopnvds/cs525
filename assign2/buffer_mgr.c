#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
};
RC shutdownBufferPool(BM_BufferPool *const bm){
	// Forces all dirty pages to be writed in disk
	return forceFlushPool(bm);
};

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

// Buffer Manager Interface Access Pages
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

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	// return our pageIndex from buffer 
	return (PageNumber *)buffer->pageIndex;
}

bool *getDirtyFlags (BM_BufferPool *const bm){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	// return our dirtyFlags from buffer 
	return buffer->dirtyFlags;
}

int *getFixCounts (BM_BufferPool *const bm){
	// Load basic structures
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;
	// return our fixCount from buffer
	return buffer->fixCount;
}

int getNumReadIO (BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	// return out reading counter from buffer pool management data
	return mgmtData->numReadIO;
};
int getNumWriteIO (BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	// return out reading counter from buffer pool management data
	return mgmtData->numWriteIO;
};

int findPageIndex (int numPage, int totalPages, PageNumber *pageIndex){
	PageNumber position = NO_PAGE;
	for(int i = 0; i < totalPages && position < 0;i++){
		if (numPage == pageIndex[i]){
			position = i;
		}
	}
	return position;
}

int searchInsertPosition(int currentPos, int *fixCount, int totalPages){
	int insertPosition = -1;
	for(int i = currentPos; (i < totalPages + currentPos && (insertPosition < 0)); i++){
		if(fixCount[i] == 0) insertPosition = i;
	}
	return insertPosition;
}

int searchLowerTime(long *lastUseTime, int *fixCount, int totalPages){
	int lowerIndex = 0;
	for(int i = 1; i < totalPages; i++){
		if(lastUseTime[lowerIndex] > lastUseTime[i] && fixCount[i] == 0) lowerIndex = i;
	}
	return fixCount[lowerIndex] == 0 ? lowerIndex : -1;
}

