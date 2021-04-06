#include "files.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static long FILE_Reader_GetPos (Reader *r) {
  FILE_Reader *rd = (FILE_Reader*)r;
  assert(rd != NULL);
  assert(rd->fp != NULL);
  long pos = ftell(rd->fp);
  assert(pos != -1); // fail
  return pos;
}

static void FILE_Reader_SetPos (Reader *r, long pos) {
  FILE_Reader *rd = (FILE_Reader*)r;
  assert(rd != NULL);
  assert(rd->fp != NULL);
  assert(pos >= 0);
  int res = fseek(rd->fp, pos, SEEK_SET);
  assert(res == 0); // fail
}

static void FILE_Reader_Read (Reader *r, void *data, size_t size, size_t n) {
  FILE_Reader *rd = (FILE_Reader*)r;
  assert(rd != NULL);
  assert(rd->fp != NULL);
  size_t res = fread(data, size, n, rd->fp);
  assert(res == n); // fail
}

void FILE_AssignReader (FILE_Reader *r, FILE *fp) {
  assert(r != NULL);
  assert(fp != NULL);
  r->base.getpos = FILE_Reader_GetPos;
  r->base.setpos = FILE_Reader_SetPos;
  r->base.read = FILE_Reader_Read;
  r->fp = fp;
}

int FILE_OpenReader (FILE_Reader *r, const char *name) {
  assert(r != NULL);
  assert(name != NULL);
  FILE *fp = fopen(name, "rb");
  if (fp) {
    FILE_AssignReader(r, fp);
  }
  return fp != NULL;
}

void FILE_CloseReader (FILE_Reader *r) {
  assert(r != NULL);
  if (r->fp != NULL) {
    fclose(r->fp);
  }
  r->base.getpos = NULL;
  r->base.setpos = NULL;
  r->base.read = NULL;
  r->fp = NULL;
}

static long FILE_Writer_GetPos (Writer *w) {
  FILE_Writer *wr = (FILE_Writer*)w;
  assert(wr != NULL);
  assert(wr->fp != NULL);
  long pos = ftell(wr->fp);
  assert(pos != -1); // fail
  return pos;
}

static void FILE_Writer_SetPos (Writer *w, long pos) {
  FILE_Writer *wr = (FILE_Writer*)w;
  assert(wr != NULL);
  assert(wr->fp != NULL);
  assert(pos >= 0);
  int res = fseek(wr->fp, pos, SEEK_SET);
  assert(res == 0); // fail
}

static void FILE_Writer_Write (Writer *w, const void *data, size_t size, size_t n) {
  FILE_Writer *wr = (FILE_Writer*)w;
  assert(wr != NULL);
  assert(wr->fp != NULL);
  size_t res = fwrite(data, size, n, wr->fp);
  assert(res == n); // fail
}

void FILE_AssignWriter (FILE_Writer *w, FILE *fp) {
  assert(w != NULL);
  assert(fp != NULL);
  w->base.getpos = FILE_Writer_GetPos;
  w->base.setpos = FILE_Writer_SetPos;
  w->base.write = FILE_Writer_Write;
  w->fp = fp;
}

int FILE_OpenWriter (FILE_Writer *w, const char *name) {
  assert(w != NULL);
  assert(name != NULL);
  FILE *fp = fopen(name, "wb");
  if (fp) {
    FILE_AssignWriter(w, fp);
  }
  return fp != NULL;
}

void FILE_CloseWriter (FILE_Writer *w) {
  assert(w != NULL);
  if (w->fp != NULL) {
    fclose(w->fp);
  }
  w->base.getpos = NULL;
  w->base.setpos = NULL;
  w->base.write = NULL;
  w->fp = NULL;
}
