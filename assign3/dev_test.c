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
  int typeLength[3] = {0,10,0};
  int keyAttrs[2] = {0, 2};
  Schema *people = createSchema(3, attrNames, dataTypes, typeLength, 2, keyAttrs);

  createTable("people", people);
  RM_TableData *rel = malloc(sizeof(RM_TableData));
  openTable(rel, "people");
  Value *value1 = malloc(sizeof(Value));
  value1->dt = DT_INT;
  value1->v.intV = 5;
  Value *value2 = malloc(sizeof(Value));
  value2->dt = DT_STRING;
  value2->v.stringV = "123";
  Value *value3 = malloc(sizeof(Value));
  value3->dt = DT_FLOAT;
  value3->v.intV = 5;
  Record *record6;
  createRecord(&record6, people);
  setAttr(record6, people, 0, value1);
  setAttr(record6, people, 2, value3);
  setAttr(record6, people, 1, value2);
  insertRecord(rel,record6);
  Record *record7;
  createRecord(&record7, people);
  getRecord(rel,record6->id,record7);
  Value *value5;
  getAttr(record7, people, 1, &value5);
  printf("%s\n", value5->v.stringV);
  closeTable(rel);
  return 0;
}



