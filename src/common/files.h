#ifndef COMMON_FILES_H_INCLUDED
#define COMMON_FILES_H_INCLUDED

#include <stdio.h>
#include "common/streams.h"

typedef struct FILE_Stream {
  Stream base;
  FILE *fp;
} FILE_Stream;

void FILE_Assign (FILE_Stream *r, FILE *fp);
int  FILE_Open (FILE_Stream *r, const char *name, const char *mode);
void FILE_Close (FILE_Stream *r);

#endif /* COMMON_FILES_H_INCLUDED */