#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern int errno;

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData){
	return RC_OK;
};
RC shutdownBufferPool(BM_BufferPool *const bm){
	return RC_OK;
};
RC forceFlushPool(BM_BufferPool *const bm){
	return RC_OK;
};

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
	return RC_OK;
};
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
	return RC_OK;
};
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
	return RC_OK;
};
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum){
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