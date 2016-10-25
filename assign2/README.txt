
################################################################
	CS525 ASSIGNMENT 2 - BUFFER MANAGER
################################################################

+--------------------------------------------------------------+
	1.Personal information
+--------------------------------------------------------------+
Name: Sergio Penavades Suarez
CWID: A20387929
Email id: spenavadessuarez@hawk.iit.edu

Name: Victor Jose Portals Lorenzo
CWID: A20370791
Email: vportalslorenzo@hawk.iit.edu

Name: Jose Emilio Carmona Lopez
CWID: A20370791
Email: jcarmonalopez@hawk.iit.edu

Leader name: Victor Jose Portals 

+--------------------------------------------------------------+
	2.File list
+--------------------------------------------------------------+
buffer_mgr.c
buffer_mgr.h
buffer_mgr_stat.c
buffer_mgr_stat.h
dberror.c
dberror.h
_DS_Store
dt.h
Makefile
README.txt
storage_mgr.c
storage_mgr.h
test_assign1_1.c
test_assign1_2.c
test_assign2_1.c
test_helper.h


+--------------------------------------------------------------+
	3.Milestone 
+--------------------------------------------------------------+
Store manager first functional version.

+--------------------------------------------------------------+
	4.Installation instruction
+--------------------------------------------------------------+
To compile our code we will use the "Makelfile" provided.

The procedure will be to open a terminal window and compile the code excuting the "Makefile" using:

	$ make

This "Makefile" is prepared to compile all files belonged to the Buffer Manager and provide test_assign2_1 executable file.

A clean instruction has been added to Makefile, it remmoves all .o and executable files:

	$ make clean

+--------------------------------------------------------------+
	5.Function descriptions
+--------------------------------------------------------------+

#initBufferPool:

	Create a buffer pool and initializes its management data.

#shutdownBufferPool:
	
	Destroy buffer pool and free resources.

#forceFlushPool:
	
	Writes all pages marked as dirty to disk.

#markDirty:
	
	Mark a page in buffer frame as dirty.

#unpinPage:
	
	Remove pin between page frame and client.

#forcePage:
	
	Write page from buffer frame to file in disk.

#pinPage:
	
	Read page from disk if it is not already in buffer.

#getFrameContents:
	
	Returns an array of PageNumberswhere the ith element is the number of the page stored in the ith page frame.

#getDirtyFlags:
	
	Returns an array of bools where the ith element is TRUE if the page stored in the ith page frame is dirty.

#getFixCounts:
	
	Returns an array of ints where the ith element is the fix count of the page stored in the ith page frame.

#getNumReadIO:
	
	Returns the number of pages that have been read from disk since a buffer pool has been initialized.

#getNumWriteIO:
	
	Returns the number of pages that have been writed to disk since a buffer pool has been initialized.

#findPageIndex:
	
	Returns the index of a page inside frame buffer.

#searchInsertPosition:
	
	Returns the insert position in a FIFO buffer discarding fixed.

#searchLowerTime:
	
	Returns the insert position in a LRU buffer discarding fixed.





+--------------------------------------------------------------+
	6.Data structure
+--------------------------------------------------------------+
BM_BufferPool

BM_Buffer

BM_ManagementData

+--------------------------------------------------------------+
	7.Extra credit
+--------------------------------------------------------------+
As the techer assistant told as, the extra credit will be given if additional tests are implemented. Because of that, in the next section we describe the test added to the store manager code.

+--------------------------------------------------------------+
	8.Test cases
+--------------------------------------------------------------+
Additoinally, we include an extra file called "test_assign1_2.c" that include some extra test to chaeck and validity so more thoroughly the store manager coded. "test_assign1_2.c" contains the following thext cases.

	#Test testCreateOpenClose: Checks if create, open and close returns error codes correctly.

	#Test testAppendEmptyBlockEnsureCapacity: Checks if appendEmptyBlock and ensureCapacity 
		functions works as expected.
	
	#Test testWriteReadBlock: test every read and write operations

+--------------------------------------------------------------+
	9.Problems solved
+--------------------------------------------------------------+

# Problem description here

# Problem description here

+--------------------------------------------------------------+
	10.Problems to be solved
+--------------------------------------------------------------+

None
