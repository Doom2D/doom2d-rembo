#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "wadres.h"
#include "streams.h"
#include "cp866.h"

typedef struct Entry {
  long offset, size;
  char name[8];
  int f;
} Entry;

typedef struct Block {
  int id;
  int ref;
  char data[];
} Block;

static int n_wads;
static int n_resources;
static Stream *wads[MAX_WADS];
static Entry resources[MAX_RESOURCES];
static Block *blocks[MAX_RESOURCES];

static int s_start, s_end;

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

static int WADRES_read (int f) {
  Stream *r = wads[f];
  stream_setpos(r, 4); // skip magic
  int32_t n = stream_read32(r);
  int32_t dir = stream_read32(r);
  stream_setpos(r, dir);
  int ok = 1;
  for (int32_t i = 0; ok && i < n; ++i) {
    Entry e;
    e.offset = stream_read32(r);
    e.size = stream_read32(r);
    e.f = f;
    stream_read(e.name, 8, 1, r);
    ok = WADRES_addresource(&e) != -1;
  }
  return ok;
}

int WADRES_rehash (void) {
  int ok = 1;
  for (int i = 0; i < n_wads; ++i) {
    if (!WADRES_read(i)) {
      ok = 0;
    }
  }
  s_start = WADRES_find("S_START");
  s_end = WADRES_find("S_END");
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

int WADRES_findsprite (const char n[4], int s, int d, char *dir) {
  s += 'A';
  d += '0';
  for (int i = s_start + 1; i < s_end; i++) {
    char a, b;
    char *wn = resources[i].name;
    if (cp866_strncasecmp(wn, n, 4) == 0 && (wn[4] == s || wn[6] == s)) {
      a = wn[4] == s ? wn[5] : 0;
      b = wn[6] == s ? wn[7] : 0;
      if (a == '0' || b == '0' || a == d || b == d) {
        if (dir != NULL) {
          *dir = (a != '0' && b == '0') || (a != d && b == d);
        }
        return i;
      }
    }
  }
  return -1;
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

void *WADRES_lock (int id) {
  assert(id >= -1 && id < MAX_RESOURCES);
  if (id >= 0) {
    Block *x = blocks[id];
    if (x) {
      x->ref += 1;
      return x->data;
    } else {
      x = malloc(sizeof(Block) + WADRES_getsize(id));
      if (x) {
        x->id = id;
        x->ref = 1;
        WADRES_getdata(id, x->data);
        blocks[id] = x;
        return x->data;
      }
    }
  }
  return NULL;
}

void WADRES_unlock (void *data) {
  if (data) {
    Block *x = data - sizeof(Block);
    int id = x->id;
    assert(id >= 0 && id < MAX_RESOURCES);
    x->ref -= 1;
    assert(x->ref >= 0);
#if 0
    if (x->ref == 0) {
      blocks[id] = NULL;
      free(x);
    }
#endif
  }
}

int WADRES_locked (int id) {
  assert(id >= -1 && id < MAX_RESOURCES);
  return (id >= 0) && (blocks[id] != NULL) && (blocks[id]->ref >= 1);
}

int WADRES_was_locked (int id) {
  assert(id >= -1 && id < MAX_RESOURCES);
  return (id >= 0) && (blocks[id] != NULL) && (blocks[id]->ref >= 0);
}
