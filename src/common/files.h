#ifndef COMMON_FILES_H_INCLUDED
#define COMMON_FILES_H_INCLUDED

#include <stdio.h>
#include "streams.h"

typedef struct FILE_Reader {
  Reader base;
  FILE *fp;
} FILE_Reader;

typedef struct FILE_Writer {
  Writer base;
  FILE *fp;
} FILE_Writer;

void FILE_AssignReader (FILE_Reader *r, FILE *fp);
int  FILE_OpenReader (FILE_Reader *r, const char *name);
void FILE_CloseReader (FILE_Reader *r);

void FILE_AssignWriter (FILE_Writer *w, FILE *fp);
int  FILE_OpenWriter (FILE_Writer *w, const char *name);
void FILE_CloseWriter (FILE_Writer *w);

#endif /* COMMON_FILES_H_INCLUDED */