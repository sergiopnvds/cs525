CC=gcc
CFLAGS=-I.
DEPS = dberror.h storage_mgr.h test_helper.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test_assign1_1: test_assign1_1.o storage_mgr.o dberror.o 
	$(CC) -o $@ $^ -I.	
 

