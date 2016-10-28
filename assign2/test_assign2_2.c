#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// var to store the current test's name
char *testName;

// check whether two the content of a buffer pool is the same as an expected content 
// (given in the format produced by sprintPoolContent)
#define ASSERT_EQUALS_POOL(expected,bm,message)			        \
  do {									\
    char *real;								\
    char *_exp = (char *) (expected);                                   \
    real = sprintPoolContent(bm);					\
    if (strcmp((_exp),real) != 0)					\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, _exp, real, message); \
	free(real);							\
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, _exp, real, message); \
    free(real);								\
  } while(0)

// test and helper methods
static void createDummyPages(BM_BufferPool *bm, int num);
static void testCLOCK (void);
static void testLFU (void);
static void testLRUK (void);

// main method
int 
main (void) 
{
  initStorageManager();
  testName = "";

  testCLOCK();
  testLFU();
  testLRUK();
}



void 
createDummyPages(BM_BufferPool *bm, int num)
{
  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
  
  for (i = 0; i < num; i++)
    {
      CHECK(pinPage(bm, h, i));
      sprintf(h->data, "%s-%i", "Page", h->pageNum);
      CHECK(markDirty(bm, h));
      CHECK(unpinPage(bm,h));
    }

  CHECK(shutdownBufferPool(bm));

  free(h);
}


void
testCLOCK ()
{
  // expected results
  const char *poolContents[] = { 
    "[0 0],[-1 0],[-1 0]" , 
    "[0 0],[1 0],[-1 0]", 
    "[0 0],[1 0],[2 0]", 
    "[3 0],[1 0],[2 0]", 
    "[3 0],[1 0],[2 0]",
    "[3 0],[1 0],[2 0]",
    "[3 0],[1 0],[4 0]",
    "[3 0],[1 1],[4 0]",
    "[2 0],[1 1],[4 0]",
    "[2 0],[1 1],[6 0]",
    "[2 0],[1 0],[6 0]",
    "[2 0],[7 0],[6 0]"
  };
  const int requests[] = {0,1,2,3,1,3,4,1,2,6,7};
  const int numLinRequests = 7;
  const int numChangeRequests = 2;

  int i;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing CLOCK page replacement";

  CHECK(createPageFile("testbuffer.bin"));

  createDummyPages(bm, 100);

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_CLOCK, NULL));

  // reading some pages linearly with direct unpin and no modifications
  for(i = 0; i < numLinRequests; i++)
    {
      pinPage(bm, h, requests[i]);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content without fixed pages");
    }

  // pin one page and test remainder
  i = numLinRequests;
  pinPage(bm, h, requests[i]);
  ASSERT_EQUALS_POOL(poolContents[i],bm,"pool content after pin page");

  // read pages and mark them as dirty
  for(i = numLinRequests + 1; i < numLinRequests + numChangeRequests + 1; i++)
    {
      pinPage(bm, h, requests[i]);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content with fixed page");
    }

  i = numLinRequests + numChangeRequests + 1;
  h->pageNum = 1;
  unpinPage(bm, h);
  ASSERT_EQUALS_POOL(poolContents[i],bm,"unpin page");
  
  pinPage(bm, h, requests[i]);
  unpinPage(bm, h);
  ASSERT_EQUALS_POOL(poolContents[++i],bm,"pool content after page unpinning");

  // check number of write IOs
  ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
  ASSERT_EQUALS_INT(8, getNumReadIO(bm), "check number of read I/Os");

  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();
}


void
testLFU ()
{
  // expected results
  const char *poolContents[] = { 
    "[1 0],[-1 0],[-1 0]", 
    "[1 0],[-1 0],[-1 0]", 
    "[1 0],[-1 0],[-1 0]", 
    "[1 0],[2 0],[-1 0]", 
    "[1 0],[2 0],[-1 0]", 
    "[1 0],[2 0],[3 0]",
    "[1 0],[2 0],[4 1]",
    "[1 0],[5 0],[4 1]",
    "[1 0],[5 0],[4 1]",
    "[1 0],[5 0],[4 1]",
    "[7 0],[5 0],[4 1]",
    "[7 0],[5 0],[4 1]",
    "[7 0],[5 0],[4 0]",
    "[7 0],[5 0],[8 0]",
  };
  const int requests[] = {1,1,1,2,2,3,4,5,5,5,7,7,8};
  const int numLinRequests = 6;
  const int numChangeRequests = 5;

  int i;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing LFU page replacement";

  CHECK(createPageFile("testbuffer.bin"));

  createDummyPages(bm, 100);

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_LFU, NULL));

  // reading some pages linearly with direct unpin and no modifications
  for(i = 0; i < numLinRequests; i++)
    {
      pinPage(bm, h, requests[i]);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content without fixed pages");
    }

  // pin one page and test remainder
  i = numLinRequests;
  pinPage(bm, h, requests[i]);
  ASSERT_EQUALS_POOL(poolContents[i],bm,"pool content after pin page");

  // read pages and mark them as dirty
  for(i = numLinRequests + 1; i < numLinRequests + numChangeRequests + 1; i++)
    {
      pinPage(bm, h, requests[i]);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content with fixed page");
    }


  i = numLinRequests + numChangeRequests + 1;
  h->pageNum = 4;
  unpinPage(bm, h);
  ASSERT_EQUALS_POOL(poolContents[i],bm,"unpin page");
  
  pinPage(bm, h, requests[i]);
  unpinPage(bm, h);
  ASSERT_EQUALS_POOL(poolContents[++i],bm,"pool content after page unpinning");

  // check number of write IOs
  ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
  ASSERT_EQUALS_INT(7, getNumReadIO(bm), "check number of read I/Os");

  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();
}

void
testLRUK ()
{
  // expected results
  const char *poolContents[] = { 
    "[1 0],[-1 0],[-1 0]",
    "[1 0],[2 0],[-1 0]",
    "[1 0],[2 0],[-1 0]",
    "[1 0],[2 0],[3 0]",
    "[1 0],[4 0],[3 0]",
    "[1 0],[4 0],[3 0]",
    "[1 0],[4 0],[3 0]",
    "[5 0],[4 0],[3 0]",
    "[6 0],[4 0],[3 0]",
  };
  const int requests[] = {1,2,1,3,4,4,3,5,6};
  const int numLinRequests = 9;
  const int numChangeRequests = 5;

  int i;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing LRU_K page replacement";

  CHECK(createPageFile("testbuffer.bin"));

  createDummyPages(bm, 100);

  long k = 2;
  void *stratData = &k;
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_LRU_K, stratData));

  // reading some pages linearly with direct unpin and no modifications
  for(i = 0; i < numLinRequests; i++)
    {
      pinPage(bm, h, requests[i]);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content without fixed pages");
    }

  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();
}

