#include "btree/column.h"
#include "fileprocessor.h"
#include "memory.h"
#include <stdlib.h>
#include <string.h>

void _get_column_size_and_dt(FILE *database_file, short size, Column *column) {
  switch (size) {
  case 0: {
    column->datatype = "NULL";
    column->size = 0;
    break;
  }
  case 1: {
    column->datatype = "INT8";
    column->size = 1;
    break;
  }
  case 2: {
    column->datatype = "INT16";
    column->size = 2;
    break;
  }
  case 3: {
    column->datatype = "INT24";
    column->size = 3;
    break;
  }
  case 4: {
    column->datatype = "INT32";
    column->size = 4;
    break;
  }
  case 5: {
    column->datatype = "INT48";
    column->size = 6;
    break;
  }
  case 6: {
    column->datatype = "INT64";
    column->size = 8;
    break;
  }
  case 7: {
    column->datatype = "FLOAT";
    column->size = 8;
    break;
  }
  case 8: {
    column->datatype = "0";
    column->size = 0;
    break;
  }
  case 9: {
    column->datatype = "1";
    column->size = 0;
    break;
  }
  default: {
    if (size >= 12 && size % 2 == 0) {
      column->datatype = "BLOB";
      column->size = (size - 12) / 2;
    } else if (size >= 13 && size % 2 != 0) {
      column->datatype = "TEXT";
      column->size = (size - 13) / 2;
    } else {
      column->datatype = "UNKNOWN";
    }
  }
  }
}

Column **get_columns_from_a_record(FILE *database_file, int offset,
                                   short *no_of_columns) {
  unsigned char buffer[1];
  if (read_bytes(database_file, offset + 2, 1, buffer) != 0) {
    perror("Failed to read the number of columns");
    return NULL;
  }

  *no_of_columns = (short)(buffer[0]) - 1;
  Column **columns = (Column **)mallox(*no_of_columns * sizeof(Column *));

  for (short i = 0; i < *no_of_columns; ++i) {
    columns[i] = (Column *)malloc(sizeof(Column));
    if (columns[i] == NULL) {
      perror("Failed to allocate memory for column");
      free_columns(columns, i);
      return NULL;
    }

    int current_offset = offset + 3 + i;
    unsigned char buffer[1];

    if (read_bytes(database_file, current_offset, 1, buffer) != 0) {
      perror("Failed to read the column size");
      free_columns(columns, i + 1);
      return NULL;
    }

    _get_column_size_and_dt(database_file, (short)buffer[0], columns[i]);
    if (strcmp(columns[i]->datatype, "UNKNOWN") == 0) {
      fprintf(stderr, "Unknown datatype for column %d\n", i + 1);
      free_columns(columns, i + 1);
      return NULL;
    }
  }

  return columns;
}

void free_columns(Column **columns, short no_of_columns) {
  if (columns != NULL) {
    for (int i = 0; i < no_of_columns; ++i) {
      if (columns[i] != NULL) {
        free(columns[i]);
        columns[i] = NULL;
      }
    }
    free(columns);
    columns = NULL;
  }
}
