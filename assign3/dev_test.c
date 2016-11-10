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
  record->data = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  insertRecord(rel, record);
  insertRecord(rel, record);
  closeTable(rel);
  return 0;
}
