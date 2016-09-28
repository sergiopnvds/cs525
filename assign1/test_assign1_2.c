#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes for test functions */
static void testCreateOpenClose(void);
static void testAppendEmptyBlockEnsureCapacity(void);
static void testWriteReadBlock(void);

/* main function running all tests */
int
main (void)
{
  testName = "";
  
  initStorageManager();

  //testCreateOpenClose();
  testAppendEmptyBlockEnsureCapacity();
  testWriteReadBlock();

  return 0;
}


/* check a return code. If it is not RC_OK then output a message, error description, and exit */
/* Try to create, open, and close a page file */
void
testCreateOpenClose(void)
{
  SM_FileHandle fh;

  testName = "test create open and close methods";
  
  // checks if createPageFile returns an error if an illegal name is given as argument
  ASSERT_ERROR(createPageFile("/\0.txt"), "Expected an RC_FILE_NOT_FOUND error");
  
  // checks if openPageFile returns an error if the file doesnt exist
  ASSERT_ERROR(openPageFile(TESTPF, &fh), "Expected an RC_FILE_NOT_FOUND error");

  // checks if closePageFile returns an error if the file is not opened.
  ASSERT_ERROR(closePageFile(&fh), "Expected RC_FILE_HANDLE_NOT_INIT error");
  
  TEST_DONE();
}

/* Use appendEmptyBlock and ensureCapacity and checks the totalNumPages */
void
testAppendEmptyBlockEnsureCapacity(void)
{
  SM_FileHandle fh;

  testName = "test appendEmptyBlock and ensureCapacity";

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));

  // add a new page
  TEST_CHECK(appendEmptyBlock(&fh));

  ASSERT_TRUE((fh.totalNumPages == 2), "totalNumPages has the expected value");

  // add pages until the document have 5
  TEST_CHECK(ensureCapacity(5, &fh));

  ASSERT_TRUE((fh.totalNumPages == 5), "totalNumPages has the expected value");

  TEST_CHECK(destroyPageFile(TESTPF));
  TEST_DONE();
}

/* write blocks and read them */
void
testWriteReadBlock(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test write and read operations";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));

  // Ensures that document has atleast 4 pages
  TEST_CHECK(ensureCapacity(4, &fh));

  // Writes each page with different information
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = 'A';
  TEST_CHECK(writeCurrentBlock(&fh, ph));

  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = 'B';
  TEST_CHECK(writeBlock(1, &fh, ph));

  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = 'C';
  TEST_CHECK(writeBlock(2, &fh, ph));

  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = 'D';
  TEST_CHECK(writeBlock(3, &fh, ph));

  // Reads the current page -> 0
  TEST_CHECK(readCurrentBlock(&fh, ph));

  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 'A'), "character in page read from disk is the one we expected.");

  // Reads the second page
  TEST_CHECK(readBlock(1, &fh, ph));

  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 'B'), "character in page read from disk is the one we expected.");

  // Reads the next block -> 3 (fourth page)
  TEST_CHECK(readNextBlock(&fh, ph));
  
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 'C'), "character in page read from disk is the one we expected.");

  // Reads the first block
  TEST_CHECK(readFirstBlock(&fh, ph));
  
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 'A'), "character in page read from disk is the one we expected.");

  // Reads the last block
  TEST_CHECK(readLastBlock(&fh, ph));
  
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 'D'), "character in page read from disk is the one we expected.");

  TEST_CHECK(destroyPageFile(TESTPF));
  TEST_DONE();
}
