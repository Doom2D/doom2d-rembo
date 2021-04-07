/* Copyright (C) 1996-1997 Aleksey Volynskov
 * Copyright (C) 2011 Rambo
 * Copyright (C) 2020 SovietPony
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

#include "glob.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "error.h"
#include "files.h"
#include "memory.h"

#include "common/wadres.h"
#include "common/streams.h"

typedef struct Block {
  int id;
  int ref;
  char data[];
} Block;

static Block *blocks[MAX_RESOURCES];

void M_startup (void) {
  memset(blocks, 0, sizeof(blocks));
}

void M_shutdown (void) {
  // stub
}

void *M_lock (int id) {
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

void M_unlock (void *p) {
  if (p) {
    Block *x = p - sizeof(Block);
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

int M_locked (int id) {
  assert(id >= -1 && id < MAX_RESOURCES);
  return (id >= 0) && (blocks[id] != NULL) && (blocks[id]->ref >= 1);
}

int M_was_locked (int id) {
  assert(id >= -1 && id < MAX_RESOURCES);
  return (id >= 0) && (blocks[id] != NULL) && (blocks[id]->ref >= 0);
}
