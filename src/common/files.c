#include "common/files.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static long FILE_Stream_GetPos (Stream *r) {
  long pos;
  FILE_Stream *rd = (FILE_Stream*)r;
  assert(rd != NULL);
  assert(rd->fp != NULL);
  pos = ftell(rd->fp);
  assert(pos != -1); // fail
  return pos;
}

static void FILE_Stream_SetPos (Stream *r, long pos) {
  int res;
  FILE_Stream *rd = (FILE_Stream*)r;
  assert(rd != NULL);
  assert(rd->fp != NULL);
  assert(pos >= 0);
  res = fseek(rd->fp, pos, SEEK_SET);
  assert(res == 0); // fail
}

static long FILE_Stream_GetLen (Stream *r) {
  int res;
  long pos, len;
  FILE_Stream *rd = (FILE_Stream*)r;
  assert(rd != NULL);
  assert(rd->fp != NULL);
  pos = ftell(rd->fp);
  assert(pos != -1); // fail get cur pos
  res = fseek(rd->fp, 0, SEEK_END);
  assert(res == 0); // fail jump to end
  len = ftell(rd->fp);
  res = fseek(rd->fp, pos, SEEK_SET);
  assert(res == 0); // fail return
  assert(len != -1); // fail get length
  return len;
}

static void FILE_Stream_Read (Stream *r, void *data, size_t size, size_t n) {
  size_t res;
  FILE_Stream *rd = (FILE_Stream*)r;
  assert(rd != NULL);
  assert(rd->fp != NULL);
  res = fread(data, size, n, rd->fp);
  assert(res == n); // fail
}

static void FILE_Stream_Write (Stream *w, const void *data, size_t size, size_t n) {
  size_t res;
  FILE_Stream *wr = (FILE_Stream*)w;
  assert(wr != NULL);
  assert(wr->fp != NULL);
  res = fwrite(data, size, n, wr->fp);
  assert(res == n); // fail
}

void FILE_Assign (FILE_Stream *r, FILE *fp) {
  assert(r != NULL);
  assert(fp != NULL);
  r->base.getpos = FILE_Stream_GetPos;
  r->base.setpos = FILE_Stream_SetPos;
  r->base.getlen = FILE_Stream_GetLen;
  r->base.read   = FILE_Stream_Read;
  r->base.write  = FILE_Stream_Write;
  r->fp = fp;
}

int FILE_Open (FILE_Stream *r, const char *name, const char *mode) {
  FILE *fp;
  assert(r != NULL);
  assert(name != NULL);
  fp = fopen(name, mode);
  if (fp) {
    FILE_Assign(r, fp);
  }
  return fp != NULL;
}

void FILE_Close (FILE_Stream *r) {
  assert(r != NULL);
  if (r->fp != NULL) {
    fclose(r->fp);
  }
  r->base.getpos = NULL;
  r->base.setpos = NULL;
  r->base.getlen = NULL;
  r->base.read   = NULL;
  r->base.write  = NULL;
  r->fp = NULL;
}
