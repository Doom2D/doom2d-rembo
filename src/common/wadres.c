#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "wadres.h"
#include "streams.h"
#include "cp866.h"

typedef struct Entry {
  long offset, size;
  char name[8];
  int f;
} Entry;

static int n_wads;
static int n_resources;
static Stream *wads[MAX_WADS];
static Entry resources[MAX_RESOURCES];

static int check_header (Stream *r) {
  assert(r != NULL);
  char ident[4];
  stream_setpos(r, 0); // !!!
  stream_read(ident, 4, 1, r);
  return (memcmp(ident, "IWAD", 4) == 0) || (memcmp(ident, "PWAD", 4) == 0);
}

int WADRES_addwad (Stream *r) {
  assert(r != NULL);
  if (n_wads < MAX_WADS && check_header(r)) {
    wads[n_wads] = r;
    n_wads += 1;
    return 1;
  } else {
    return 0;
  }
}

static int WADRES_addresource (const Entry *e) {
  assert(e != NULL);
  for (int i = 0; i < n_resources; ++i) {
    if (cp866_strncasecmp(resources[i].name, e->name, 8) == 0) {
      memcpy(&resources[i], e, sizeof(Entry));
      return i;
    }
  }
  if (n_resources < MAX_RESOURCES) {
    memcpy(&resources[n_resources], e, sizeof(Entry));
    n_resources += 1;
    return n_resources - 1;
  }
  return -1;
}

static int WADRES_read (Stream *r) {
  stream_setpos(r, 4); // skip magic
  int32_t n = stream_read32(r);
  int32_t dir = stream_read32(r);
  stream_setpos(r, dir);
  int ok = 1;
  for (int32_t i = 0; ok && i < n; ++i) {
    Entry e;
    e.offset = stream_read32(r);
    e.size = stream_read32(r);
    stream_read(e.name, 8, 1, r);
    ok = WADRES_addresource(&e) != -1;
  }
  return ok;
}

int WADRES_rehash (void) {
  int ok = 1;
  for (int i = 0; i < n_wads; ++i) {
    if (!WADRES_read(wads[i])) {
      ok = 0;
    }
  }
  return ok;
}

int WADRES_find (const char name[8]) {
  int i = n_resources - 1;
  while (i >= 0 && cp866_strncasecmp(name, resources[i].name, 8) != 0) {
    i -= 1;
  }
  return i;
}

int WADRES_maxids (void) {
  return n_resources;
}

Stream *WADRES_getbasereader (int id) {
  assert(id >= 0 && id < n_resources);
  return wads[resources[id].f];
}

long WADRES_getoffset (int id) {
  assert(id >= 0 && id < n_resources);
  return resources[id].offset;
}

long WADRES_getsize (int id) {
  assert(id >= 0 && id < n_resources);
  return resources[id].size;
}

void WADRES_getname (int id, char *name) {
  assert(id >= 0 && id < n_resources);
  strncpy(name, resources[id].name, 8);
}

void WADRES_getdata (int id, void *data) {
  assert(id >= 0 && id < n_resources);
  Stream *r = wads[resources[id].f];
  long pos = stream_getpos(r);
  stream_setpos(r, resources[id].offset);
  stream_read(data, resources[id].size, 1, r);
  stream_setpos(r, pos);
}
