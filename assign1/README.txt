
################################################################
	CS525 HOMEWORK 1
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
Readme.txt
_DS_Store
dberror.c
dberror.h
storage_mgr.c
storage_mgr.h
test_assign1_1.c
test_assign1_2.c
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

	$ make.

This "Makefile" is prepared to compile all files belonged to the Store Manager.

test_assign1_2.c must be compiled separately (it has another main function) by the next command:

	$ gcc test_assign1_2.c storage_mgr.c dberror.c -o test_assign1_2

+--------------------------------------------------------------+
5.Function descriptions
+--------------------------------------------------------------+

#initStoreManager:

	Funcion used to initilize the Store manager

#createPageFile:

	This function creates a new page file. It receives the name of the file and creates the file with one page and fill the page with zeros ('\0').

	Among C own functions we need for codding "createPageFile" are fopen(), fwrite() and fclose() to create, write and close the file. Additionally it returns corresponding write error and file handle error if no file exists.

#openPageFile:

	Function "openPageFile" open a whole page file that already exits from disk. It receives the file name to open and a existing file handle. If the page file exists and it is opened successfully we add the data of the file in the data handle. Furthermore, if the file does not exist, it returs a file not found error.

	In this case we use fopen() and fread() C functions, use "->" for adding values to the handle struct and return the RCs.

#closePageFile:

	It Closes an open page file. It receives the metaData (fileHandle) and using the function fclose() with the fhandle values, it closes the file. It will return an ok RC if the file was closed correctly and a file handle error if there is an error.

#destroyPageFile:

	It destroyes an open page file. It receives the file name and using the function remove(), it drestroys the file. It will return an ok RC if the file was destroyed correctly and a file not found error if it does not find a file with the name provided.

#readBlock:

	This function reads the number page block from a file provided and stores it in memory.

	It receives the index of the page(pageNum), the file handle(fHandle) and where the data will be stored(memPage). Besides, this function uses fseek(), fread(), and the auxiliar function "updateCurrentInformation". This auxiliar function updates the file metadata.

	If the file has not have the number of pages required, the program will return a non existing page error. Also, if the file has not been opened, it will return a handle not init error.

#getBlockPos:

	It returns the current page positin in the file, using the metadata raceived in the handle. If there is not a handle it returns a handle not init error.

#readFirstBlock:

	It reads the first page in the file. It receives the metadata(fHandle) and the variable where to save the page(memPage). It uses the function "readBlock" described above.

#readLastBlock:

	It reads the last page in the file. It receives the metadata(fHandle) and the variable where to save the page(memPage). It uses the function "readBlock" described above.

#readPreviousBlock:

	It reads the previous page in the file. It receives the metadata(fHandle) and the variable where to save the page(memPage). It uses the function "readBlock" described above. 

	Note: The metadata includes the information about in which page we are an the moment.

#readCurrentBlock:
	
	It reads the current page in the file. It receives the metadata(fHandle) and the variable where to save the page(memPage). It uses the function "readBlock" described above.

	Note: The metadata includes the information about in which page we are an the moment.

#readNextBlock:

	It reads the next page in the file. It receives the metadata(fHandle) and the variable where to save the page(memPage). It uses the function "readBlock" described above.

	Note: The metadata includes the information about in which page we are an the moment.

#writeBlock:

	This function writes the current page block from a file provided and stores it in memory.

	It receives the index of the page(pageNum), the file handle(fHandle) and where the data is actually (memPage). Besides, this function uses fseek() and fwrite().

	If the file has not have the number of pages required, the program will return a non existing page error. Also, if there is an error in using the file it will return a handle not init error. And if there is an error writing the file it will return a write error.

#writeCurrentBlock:

	This function writes page block in an absolute position from a file provided and stores it in memory.

	It receives the index of the page(pageNum), the file handle(fHandle) and where the data is actually (memPage). Besides, this function uses fseek() and fwrite().Additionally it calls the functino "writeBlock".

	If the file has not have the number of pages required, the program will return a non existing page error. Also, if there is an error in using the file it will return a handle not init error. And if there is an error writing the file it will return a write error.

#appendEmptyBlock:

	It ncreases by one the number of pages in the file and this new page has to be itialized whit all zeros.

	It receives thefile metadata and uses the C funcions fclose() and fwrite(), the Store manaager funtion "openPageFile" and the auxiliar function "updateCurrentInformation".

	It will return ok in case of complete the whole function, a write failed error in case of if there are problems writing and closing the file, and a file not found error if there is not a file with the name provided by the metadata.

#EnsureCapacity:

	If the file has less than numberOfPages pages this function increases the size to numberOfPages value received. Additionally, this function receives the file metadata.

	Besides, it uses the function "appendEmptyBlock" datailed above. If the funtion finish correctly, it returns ok.

#UpdateCurrentInformation:

	This is a axiliary function which updates the handle struct and writes the metadata at the start of the file.

	It receives de total number of pages (totalNumPages), the current page position(curPagePos) and the file metadata (fHandle). 

	Furthermore, it uses C functions like fseek() and fwrite() to modify and move through the file.

	It will return ok in case of complete the whole function, a write failed error in case of if there are problems writing and closing the file, and a handle not not error if there is a problem with the metadata provided during fseek() execution.


+--------------------------------------------------------------+
6.Data structure
+--------------------------------------------------------------+

In case of this store manager the information will be stored in a file which is splitted in pages. This pages length is 4096 bytes. 

Additonally, the information will be read and stored page per page.

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

# Append empty blocks to the file. We had to close current file and open in append mode. After this, we had to reopen the file in r+ mode.

# Read/Write in an absolute position. We have used fseek function to accomplish this purpose.

# Persistent metadata information. We have created a struct that will be store at the start of the file. The program will need this information the next time it opens the file.

+--------------------------------------------------------------+
10.Problems to be solved
+--------------------------------------------------------------+

None

