
################################################################
	CS525 ASSIGNMENT 3 - RECORD MANAGER
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
Makefile
README.txt
buffer_mgr.c
buffer_mgr.h
buffer_mgr_stat.c
buffer_mgr_stat.h
dberror.c
dberror.h
dt.h
expr.c
expr.h
record_mgr.c
record_mgr.h
rm_serializer.c
storage_mgr.c
storage_mgr.h
tables.h
test_assign1_1.c
test_assign1_2.c
test_assign2_1.c
test_assign2_2.c
test_assign3_1.c
test_expr.c
test_helper.h


+--------------------------------------------------------------+
	3.Milestone 
+--------------------------------------------------------------+
Record manager first functional version.

+--------------------------------------------------------------+
	4.Installation instruction
+--------------------------------------------------------------+
To compile our code we will use the "Makefile" provided.

The procedure will be to open a terminal window and compile the code excuting the "Makefile" using:

	$ make

This "Makefile" is prepared to compile all files belonged to the Buffer Manager and provide test_assign3_1 file.

A clean instruction has been added to Makefile, it removes all .o and executable files:

	$ make clean

+--------------------------------------------------------------+
	5.Function descriptions
+--------------------------------------------------------------+

#initRecordManager:

	Initialize the record manager.

#shutdownRecordManager:
	
	Shut down the record manager.

#createTable:
	
	Create a new table with schema and name given.

#openTable:
	
	Open the table with the name given.

#closeTable:
	
	Close the opened table.

#deleteTable:
	
	Delete the table.

#getNumTuples:
	
	Obtain the total number of records.

#insertRecord:
	
	Insert a new record in the table.

#deleteRecord:
	
	Delete the record from the table.

#udpateRecord:
	
	Update the attributes of a certain record.

#getRecord:
	
	Retrieve a certain record.

#startScan:
	
	Initialize the scan.

#next:
	
	Retrieve the next record that matches with the expression given.

#closeScan:
	
	Close the scan.

#getRecordSize:
	
	Retrieves the size that a record occupies.

#createSchema:
	
	Create a Schema struct with the attributes given.

#freeSchema:
	
	Releases the memory occupied by the schema.

#createRecord:
	
	Allocates the necessary space in memory of a record.

#freeRecord:
	
	Releases the memory occupied by a record.

#getAttr:
	
	Retrieve the certain attribute from a record.

#setAttr:
	
	Set the certain attribute from a record.

#getAttrOffset:
	
	Return the relative offset of the attribute in a record.

#getNumPagesSchema:
	
	Return the number of pages that occupies the schema.

#calculatePageCap:
	
	Return the number of records that fits into a single page.

+--------------------------------------------------------------+
	6.Data structure
+--------------------------------------------------------------+
Schema

Record

RM_TableData

TableHandle

RM_ScanMgmt

+--------------------------------------------------------------+
        7.Extra credit
+--------------------------------------------------------------+

None

+--------------------------------------------------------------+
        8.Test cases
+--------------------------------------------------------------+

No extra test cases added

+--------------------------------------------------------------+
        9.Problems solved
+--------------------------------------------------------------+

# Schema handling and slot mapping.

+--------------------------------------------------------------+
        10.Problems to be solved
+--------------------------------------------------------------+

None
