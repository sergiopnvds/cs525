CC=gcc
CFLAGS=-g
DEPS = dberror.h storage_mgr.h buffer_mgr.h buffer_mgr_stat.h record_mgr.h expr.h rm_serializer.c

%.o: %.c $(DEPS)
	$(CC) -std=gnu99 -c -o $@ $< $(CFLAGS)

test_assign3: test_assign3_1

test_assign3_1:  test_assign3_1.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o record_mgr.o rm_serializer.o expr.o dberror.o 
	$(CC) -o $@ $^ -I.

test_assign2: test_assign2_1 test_assign2_2

test_assign2_1: test_assign2_1.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o dberror.o 
	$(CC) -o $@ $^ -I.	

test_assign2_2: test_assign2_2.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o dberror.o 
	$(CC) -o $@ $^ -I.	

test_assign1_1: test_assign1_1.o storage_mgr.o dberror.o 
	$(CC) -o $@ $^ -I.	
 
test_assign1_2: test_assign1_2.o storage_mgr.o dberror.o 
	$(CC) -o $@ $^ -I.

dev_test:  dev_test.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o record_mgr.o rm_serializer.o expr.o dberror.o 
	$(CC) -o $@ $^ -I.

clean:
	rm -f *.o test_assign1_1 test_assign1_2 test_assign2_1 test_assign2_2 test_assign3_1