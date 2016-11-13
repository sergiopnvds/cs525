#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"

// main method
int 
main (void) 
{
  printf("DEV TEST. NOT TO SUBMIT\n");

  char *attrNames[3] = {"id","name","age"};
  DataType dataTypes[3] = {DT_INT, DT_STRING, DT_INT};
  int typeLength[3] = {4,255,4};
  int keyAttrs[2] = {0, 2};
  Schema *people = createSchema(3, attrNames, dataTypes, typeLength, 2, keyAttrs);

  createTable("prueba1", people);
  RM_TableData *rel = malloc(sizeof(RM_TableData));
  openTable(rel, "prueba1");
  Record *record = malloc(sizeof(Record));
  record->data = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  insertRecord(rel, record);
  deleteRecord(rel, record->id);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  insertRecord(rel, record);
  Record *record2 = malloc(sizeof(Record));
  (record2->id).page = 1;
  (record2->id).slot = 12;
  deleteRecord(rel, record2->id);
  record2->data = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
  insertRecord(rel, record2);
  insertRecord(rel, record2);
  Record *record3 = malloc(sizeof(Record));
  (record3->id).page = 1;
  (record3->id).slot = 10;
  deleteRecord(rel, record3->id);
  record3->data = "ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";
  insertRecord(rel, record3);
  insertRecord(rel, record3);
  closeTable(rel);
  return 0;
}
