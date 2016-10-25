
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

	Funcion description here.

#initBufferPool:
	
	Function description here.

#shutdownBufferPool:
	
	Function description here.

#forceFlushPool:
	
	Function description here.

#markDirty:
	
	Function description here.

#unpinPage:
	
	Function description here.

#forcePage:
	
	Function description here.

#pinPage:
	
	Function description here.

#getFrameContents:
	
	Function description here.

#getDirtyFlags:
	
	Function description here.

#getFixCounts:
	
	Function description here.

#getNumReadIO:
	
	Function description here.

#getNumWriteIO:
	
	Function description here.

#findPageIndex:
	
	Function description here.

#searchInsertPosition:
	
	Function description here.

#searchLowerTime:
	
	Function description here.





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
