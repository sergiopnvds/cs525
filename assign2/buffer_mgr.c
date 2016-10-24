#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int errno;



typedef struct BM_Mgmtdata{
	SM_FileHandle fileHandle;
	void* buffer;
} BM_Mgmtdata;

typedef struct BM_FIFOBuffer{
	SM_PageHandle* frameBuffer;
	int* pageIndex;
	char* dirtyFlags;
	int insertPos;
} BM_FIFOBuffer;

void createFIFOBuffer(BM_FIFOBuffer* buffer, const int numFrames);
void insertFIFO(BM_BufferPool *const bm, BM_PageHandle *ph);
void removeFIFO(BM_BufferPool *const bm, PageNumber pageNum);
int findPageIndexFIFO(int numPage, int totalPages, int *pageIndex);
RC markDirtyFIFO(BM_BufferPool *const bm, BM_PageHandle *const page);
void forcePageFIFO (BM_BufferPool *const bm, int pageNum);

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData){
	SM_FileHandle fileHandle;
	char *pageFileName2 = strdup(pageFileName);
	if(openPageFile (pageFileName2, &fileHandle) == RC_FILE_NOT_FOUND) {
		return RC_FILE_NOT_FOUND;
	} else {
		void* buffer;
		switch(strategy){
			case RS_FIFO:
				buffer = malloc(sizeof(BM_FIFOBuffer));
				createFIFOBuffer(buffer, numPages);
				break;
			case RS_LRU:
				break;
			case RS_CLOCK:
				break;
			case RS_LFU:
				break;
			case RS_LRU_K:
				break;
			default:
				return -1;
		}
		BM_Mgmtdata *mgmtData = malloc(sizeof(BM_Mgmtdata));
		mgmtData->fileHandle = fileHandle;
		mgmtData->buffer = buffer;
		// Write BufferPool data and fifo buffer to mgmtData
		bm->pageFile = pageFileName2;
		bm->numPages = numPages;
		bm->strategy = strategy;
		bm->mgmtData = mgmtData;
		return RC_OK;
	}


};
RC shutdownBufferPool(BM_BufferPool *const bm){
	/*TODO*/
	return RC_OK;
};
RC forceFlushPool(BM_BufferPool *const bm){
	printf("Flushing pool\n");
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_FIFOBuffer *fifoBuffer = mgmtData->buffer;
	for(int i=0;i < bm->numPages;i++){
		if(fifoBuffer->dirtyFlags[i] == 0x01)
			printf("Flushing page %i\n", fifoBuffer->pageIndex[i]);
			writeBlock (fifoBuffer->pageIndex[i], &mgmtData->fileHandle, fifoBuffer->frameBuffer[i]);
	}
	/*TODO*/
	return RC_OK;
};

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
	switch(bm->strategy){
		case RS_FIFO:
			return markDirtyFIFO(bm, page);
			break;
		case RS_LRU:
			break;
		case RS_CLOCK:
			break;
		case RS_LFU:
			break;
		case RS_LRU_K:
			break;
		default:
			return -1;
	}
	return RC_OK;
};
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
	//TODO: Force Page if marked dirty?
	switch(bm->strategy){
		case RS_FIFO:
			removeFIFO(bm, page->pageNum);
			break;
		case RS_LRU:
			break;
		case RS_CLOCK:
			break;
		case RS_LFU:
			break;
		case RS_LRU_K:
			break;
		default:
			return -1;
	}
	return RC_OK;
};
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
	switch(bm->strategy){
		case RS_FIFO:
			forcePageFIFO(bm,page->pageNum);
			break;
		case RS_LRU:
			break;
		case RS_CLOCK:
			break;
		case RS_LFU:
			break;
		case RS_LRU_K:
			break;
		default:
			return -1;
	}
	return RC_OK;
};
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
	
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	// Read file block to memPage
	SM_PageHandle memPage = malloc(PAGE_SIZE);
	readBlock (pageNum, &mgmtData->fileHandle, memPage);

	// Stores page data and number in BM_PageHandle
	page->pageNum = pageNum;
	page->data = memPage;

	switch(bm->strategy){
		case RS_FIFO:
			insertFIFO(bm,page);
			break;
		case RS_LRU:
			break;
		case RS_CLOCK:
			break;
		case RS_LFU:
			break;
		case RS_LRU_K:
			break;
		default:
			return -1;
	} 
	return RC_OK;
};

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){
	return RC_OK;
};
bool *getDirtyFlags (BM_BufferPool *const bm){
	bool *temp = malloc(sizeof(*temp));
    *temp=true;
	return temp;
};
int *getFixCounts (BM_BufferPool *const bm){
	int *temp = malloc(sizeof(*temp));
    *temp = 0;
	return temp;
};
int getNumReadIO (BM_BufferPool *const bm){
	return -1;
};
int getNumWriteIO (BM_BufferPool *const bm){
	return -1;
};

void createFIFOBuffer(BM_FIFOBuffer* fifoBuffer, const int numFrames){
	printf("Creating FIFO Buffer\n");
	// We create a fifoBuffer structure
	// We create buffer structure to allocate frames as BM_PageHandle structures.
	SM_PageHandle *frameBuffer = malloc(PAGE_SIZE * numFrames);
	// We create index array of same size as buffer to allocate info for replacement. 
	int *pageIndex = malloc(sizeof(int)*numFrames);
	char *dirtyFlags = malloc(sizeof(char)*numFrames);
	// Initializes index array values to -1
	for(int i = 0; i < numFrames;i++){
		pageIndex[i] = -1;
		dirtyFlags[i] = 0x00;
	}
	for(int i = 0; i < numFrames;i++){
		printf("%i,", pageIndex[i]);
	}
	printf("\n");
	// Write buffer and index arrays to fifo structure. First inserting position is 0.
	fifoBuffer->frameBuffer = frameBuffer;
	fifoBuffer->pageIndex = pageIndex;
	fifoBuffer->dirtyFlags = dirtyFlags;
	fifoBuffer->insertPos = 0;

	return;
}

void insertFIFO(BM_BufferPool *const bm, BM_PageHandle *ph){
	//TODO: Search for empty space  
	printf("insertFIFO\n");
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_FIFOBuffer *fifoBuffer = mgmtData->buffer;

	int index = findPageIndexFIFO(ph->pageNum, bm->numPages,fifoBuffer->pageIndex);

	printf("Inserting page %i\n", ph->pageNum);
	if(index < 0){
		if(fifoBuffer->pageIndex[fifoBuffer->insertPos] >= 0 && fifoBuffer->dirtyFlags[fifoBuffer->insertPos] == 0x01){
			printf("Flushing page %i\n", fifoBuffer->pageIndex[fifoBuffer->insertPos]);
			writeBlock (fifoBuffer->pageIndex[fifoBuffer->insertPos], &mgmtData->fileHandle, fifoBuffer->frameBuffer[fifoBuffer->insertPos]);
		}
		fifoBuffer->frameBuffer[fifoBuffer->insertPos] = ph->data;
		fifoBuffer->pageIndex[fifoBuffer->insertPos] = ph->pageNum;
		fifoBuffer->dirtyFlags[fifoBuffer->insertPos] = 0x00;
		fifoBuffer->insertPos = (fifoBuffer->insertPos + 1)%bm->numPages;
	}else{
		printf("Page is already in buffer, nothing to insert\n");
	}
	for(int i = 0; i < bm->numPages;i++){
		printf("%i,", fifoBuffer->pageIndex[i]);
	}
	printf("\n");
	return;
}

void removeFIFO(BM_BufferPool *const bm, PageNumber pageNum){
	// TODO: fill buffer frame with zeroes (?)
	printf("removeFIFO\n");
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_FIFOBuffer *fifoBuffer = mgmtData->buffer;

	printf("Removing page %i from buffer\n",pageNum);

	int index = findPageIndexFIFO(pageNum, bm->numPages,fifoBuffer->pageIndex);

	if(index < 0){
		printf("Page not found, nothing will be removed\n");
	}else{
		if(fifoBuffer->dirtyFlags[index] == 0x01){
			printf("Flushing page %i\n", fifoBuffer->pageIndex[index]);
			writeBlock (pageNum, &mgmtData->fileHandle, fifoBuffer->frameBuffer[index]);
		}
		fifoBuffer->pageIndex[index] = -1;
		fifoBuffer->dirtyFlags[index] = 0x00;
		printf("Page %i removed\n",pageNum);
	}
	for(int i = 0; i < bm->numPages;i++){
		printf("%i,", fifoBuffer->pageIndex[i]);
	}
	printf("\n");
	return;
}

RC markDirtyFIFO (BM_BufferPool *const bm, BM_PageHandle *const page){
	printf("Marking page %i as dirty\n", page->pageNum);
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_FIFOBuffer *fifoBuffer = mgmtData->buffer;
	int index = findPageIndexFIFO(page->pageNum, bm->numPages,fifoBuffer->pageIndex);
	fifoBuffer->dirtyFlags[index] = 0x01;
	return RC_OK;
}

void forcePageFIFO (BM_BufferPool *const bm, int pageNum){
	printf("Write page %i to file.\n", pageNum);
	BM_Mgmtdata *mgmtData = bm->mgmtData;
	BM_FIFOBuffer *fifoBuffer = mgmtData->buffer;
	int index = findPageIndexFIFO(pageNum, bm->numPages,fifoBuffer->pageIndex);
	writeBlock (pageNum, &mgmtData->fileHandle, fifoBuffer->frameBuffer[index]);

}

int findPageIndexFIFO (int numPage, int totalPages, int *pageIndex){
	int position = -1;
	for(int i = 0; i < totalPages && position < 0;i++){
		if (numPage == pageIndex[i]){
			position = i;
		}
	}
	//printf("%i\n", position);
	return position;
}