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
	return forceFlushPool(bm);
};

RC forceFlushPool(BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *LRUBuffer = mgmtData->buffer;
	for(int i=0;i < bm->numPages;i++){
		if(LRUBuffer->dirtyFlags[i] == TRUE){
			int err = writeBlock (LRUBuffer->pageIndex[i], &mgmtData->fileHandle, LRUBuffer->frameBuffer[i]);
			if(err != RC_OK) return err;
			LRUBuffer->dirtyFlags[i] = FALSE;
			mgmtData->numWriteIO++;
		}
	}
	return RC_OK;
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *LRUBuffer = mgmtData->buffer;
	int index = findPageIndex(page->pageNum, bm->numPages,LRUBuffer->pageIndex);
	LRUBuffer->dirtyFlags[index] = TRUE;
	return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *fifoBuffer = mgmtData->buffer;

	int index = findPageIndex(page->pageNum, bm->numPages,fifoBuffer->pageIndex);
	if(index != NO_PAGE) fifoBuffer->fixCount[index]--;

	return RC_OK;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *LRUBuffer = mgmtData->buffer;
	int index = findPageIndex(page->pageNum, bm->numPages,LRUBuffer->pageIndex);
	int err = writeBlock (page->pageNum, &mgmtData->fileHandle, LRUBuffer->frameBuffer[index]);
	if(err != RC_OK) return err;
	mgmtData->numWriteIO++;
	LRUBuffer->dirtyFlags[index] = FALSE;
	return RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *buffer = mgmtData->buffer;

	int index = findPageIndex(pageNum, bm->numPages,buffer->pageIndex);
	if(index == NO_PAGE){
		int insertionIndex = -1;
		if(bm->strategy == RS_LRU){
			insertionIndex = searchLowerTime(buffer->lastUseTime, buffer->fixCount, bm->numPages);
			if(insertionIndex < 0) return RC_OK;
		}else if(bm->strategy == RS_FIFO){
			insertionIndex = buffer->insertPos;
			if(buffer->fixCount[insertionIndex] > 0){
				buffer->insertPos = (insertionIndex + 1)%bm->numPages;
				return pinPage(bm, page, pageNum);
			}
		}
		if(buffer->dirtyFlags[insertionIndex] == TRUE){
			int err = writeBlock (buffer->pageIndex[insertionIndex], &mgmtData->fileHandle, buffer->frameBuffer[insertionIndex]);
			if(err != RC_OK) return err;
			mgmtData->numWriteIO++;
		}
		if((mgmtData->fileHandle).totalNumPages <=  pageNum){
			ensureCapacity(pageNum + 1, &mgmtData->fileHandle);
		}
		SM_PageHandle memPage = malloc(PAGE_SIZE);
		RC code = readBlock (pageNum, &mgmtData->fileHandle, memPage);
		if(code != RC_OK) return code;
		mgmtData->numReadIO++;
		page->pageNum = pageNum;
		page->data = memPage;

		buffer->frameBuffer[insertionIndex] = page->data;
		buffer->pageIndex[insertionIndex] = page->pageNum;
		buffer->dirtyFlags[insertionIndex] = FALSE;
		buffer->fixCount[insertionIndex] = 1;
		buffer->insertPos = (insertionIndex + 1)%bm->numPages;
		buffer->lastUseTime[insertionIndex] = buffer->timeCounter++;
	}else{
		page->pageNum = pageNum;
		page->data = buffer->frameBuffer[index];
		buffer->fixCount[index]++;
		if(bm->strategy == RS_LRU) buffer->lastUseTime[index] = buffer->timeCounter++;
	}
	return RC_OK;

};

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *LRUBuffer = mgmtData->buffer;
	return (PageNumber *)LRUBuffer->pageIndex;
}

bool *getDirtyFlags (BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *LRUBuffer = mgmtData->buffer;
	return LRUBuffer->dirtyFlags;
}

int *getFixCounts (BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_Buffer *LRUBuffer = mgmtData->buffer;
	return LRUBuffer->fixCount;
}

int getNumReadIO (BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	return mgmtData->numReadIO;
};
int getNumWriteIO (BM_BufferPool *const bm){
	BM_Mgmtdata *mgmtData = bm->mgmtData;
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

int searchLowerTime(long *lastUseTime, int *fixCount, int totalPages){
	int lowerIndex = 0;
	for(int i = 1; i < totalPages; i++){
		if(lastUseTime[lowerIndex] > lastUseTime[i] && fixCount[i] == 0) lowerIndex = i;
	}
	return fixCount[lowerIndex] == 0 ? lowerIndex : -1;
}

