/* Copyright (C) 2020 SovietPony
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License ONLY.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "streams.h"
#include "kos32.h"
#include <assert.h>
#include <string.h>

static inline int ReadFile (int offset, void *data, size_t len, int enc, const void *path, int *count) {
  struct FileExt info = { KOS32_READ_FILE, offset, 0/*high_offset*/, (int)len, (int)data, enc, path };
  return FileExt(&info, count);
}

static inline int CreateFile (const void *data, size_t len, int enc, const void *path, int *count) {
  struct FileExt info = { KOS32_CREATE_FILE, 0, 0, (int)len, (int)data, enc, path };
  return FileExt(&info, count);
}

static inline int WriteFile (int offset, const void *data, size_t len, int enc, const void *path, int *count) {
  struct FileExt info = { KOS32_WRITE_FILE, offset, 0/*high_offset*/, (int)len, (int)data, enc, path };
  return FileExt(&info, count);
}

static inline int InfoFile (int flags, struct FileInfo *bdfe, int enc, const void *path) {
  struct FileExt info = { KOS32_GET_INFO, 0, flags, 0, (int)bdfe, enc, path };
  return FileExt(&info, NULL);
}

static inline int StartApp (int flags, const char *params, int enc, const void *path) {
  struct FileExt info = { KOS32_START_APP, flags, (int)params, 0, 0, enc, path };
  return FileExt(&info, NULL);
}

static inline int DeleteFile (int enc, const void *path) {
  struct FileExt info = { KOS32_DELETE, 0, 0, 0, 0, enc, path };
  return FileExt(&info, NULL);
}

static long KOS32_GetPos (Stream *r) {
  KOS32_Stream *rd = (KOS32_Stream*)r;
  assert(rd != NULL);
  assert(rd->name[0] != 0);
  return rd->pos;
}

static void KOS32_SetPos (Stream *r, long pos) {
  KOS32_Stream *rd = (KOS32_Stream*)r;
  assert(rd != NULL);
  assert(rd->name[0] != 0);
  assert(pos >= 0);
  rd->pos = pos;
}

static long KOS32_GetLen (Stream *r) {
  KOS32_Stream *rd = (KOS32_Stream*)r;
  assert(rd != NULL);
  assert(rd->name[0] != 0);
  struct FileInfo info;
  int res = InfoFile(0, &info, KOS32_UTF8, rd->name);
  assert(res == KOS32_FILE_SUCCESS);
  return info.size;
}

static void KOS32_Read (Stream *r, void *data, size_t size, size_t n) {
  KOS32_Stream *rd = (KOS32_Stream*)r;
  assert(rd != NULL);
  assert(rd->name[0] != 0);
  long long len = (long long)size * n;
  int count = 0;
  int res = ReadFile(rd->pos, data, len, KOS32_UTF8, rd->name, &count);
  assert(res == KOS32_FILE_SUCCESS);
  assert(count == len);
  rd->pos += len;
}

static void KOS32_Write (Stream *w, const void *data, size_t size, size_t n) {
  KOS32_Stream *wr = (KOS32_Stream*)w;
  assert(wr != NULL);
  assert(wr->name[0] != 0);
  long long len = (long long)size * n;
  int count = 0;
  int res = WriteFile(wr->pos, data, len, KOS32_UTF8, wr->name, &count);
  assert(res == KOS32_FILE_SUCCESS);
  assert(count == len);
  wr->pos += len;
}

void KOS32_Assign (KOS32_Stream *s, const char *name, long pos) {
  assert(s != NULL);
  assert(name != NULL);
  assert(pos >= 0);
  s->base.getpos = KOS32_GetPos;
  s->base.setpos = KOS32_SetPos;
  s->base.getlen = KOS32_GetLen;
  s->base.read   = KOS32_Read;
  s->base.write  = KOS32_Write;
  strncpy(s->name, name, 264);
  s->pos = pos;
}

static void getpath (char *buf, int len, const char *name) {
  int i = GetCurrentFolderEnc(buf, len, KOS32_UTF8);
  buf[i - 1] = '/';
  strcpy(&buf[i], name);
}

static inline is_directory (struct FileInfo *info) {
  return !!(info->attr & (KOS32_ATTR_MASK_VOLUME | KOS32_ATTR_MASK_FOLDER));
}

int KOS32_Open (KOS32_Stream *s, const char *name) {
  assert(s != NULL);
  assert(name != NULL);
  char path[264];
  getpath(path, 264, name);
  struct FileInfo info;
  int res = InfoFile(0, &info, KOS32_UTF8, path);
  if (res == KOS32_FILE_SUCCESS && !is_directory(&info)) {
    KOS32_Assign(s, name, 0);
    return 1;
  }
  return 0;
}

int KOS32_Create (KOS32_Stream *s, const char *name) {
  assert(s != NULL);
  assert(name != NULL);
  char path[264];
  getpath(path, 264, name);
  int res = CreateFile(NULL, 0, KOS32_UTF8, path, NULL);
  if (res == KOS32_FILE_SUCCESS) {
    KOS32_Assign(s, name, 0);
    return 1;
  }
  return 0;
}

void KOS32_Close (KOS32_Stream *s) {
  assert(s != NULL);
  s->base.getpos = NULL;
  s->base.setpos = NULL;
  s->base.getlen = NULL;
  s->base.read   = NULL;
  s->base.write  = NULL;
  s->name[0]     = 0;
  s->pos         = 0;
}
