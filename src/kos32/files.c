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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "files.h"
#include "error.h"

#include "map.h" // MAP_load
#include "save.h" // SAVE_getname

#include "kos32/streams.h"
#include "common/streams.h"
#include "common/wadres.h"
#include "common/cp866.h"

int d_start, d_end;

char savname[SAVE_MAX][SAVE_MAXLEN];
char savok[SAVE_MAX];

static int m_start, m_end;
static int s_start, s_end;

void F_addwad (const char *fn) {
  static int i = 0;
  static KOS32_Stream wadh[MAX_WADS];
  if (i < MAX_WADS) {
    if (KOS32_Open(&wadh[i], fn)) {
      if (WADRES_addwad(&wadh[i].base)) {
        i += 1;
      } else {
        ERR_failinit("Invalid WAD %s", fn);
      }
    } else {
      ERR_failinit("Unable to add WAD %s", fn);
    }
  } else {
    ERR_failinit("Too many wads");
  }
}

void F_initwads (void) {
  if (!WADRES_rehash()) {
    ERR_failinit("F_initwads: failed rehash");
  }
  d_start = F_getresid("D_START");
  d_end = F_getresid("D_END");
  m_start = F_getresid("M_START");
  m_end = F_getresid("M_END");
  s_start = F_getresid("S_START");
  s_end = F_getresid("S_END");
}

int F_findres (const char n[8]) {
  return WADRES_find(n);
}

int F_getresid (const char n[8]) {
  int i = F_findres(n);
  if (i == -1) {
    ERR_fatal("F_getresid: resource %.8s not found", n);
  }
  return i;
}

void F_getresname (char n[8], int r) {
  WADRES_getname(r, n);
}

int F_getsprid (const char n[4], int s, int d, char *dir) {
  s += 'A';
  d += '0';
  for (int i = s_start + 1; i < s_end; i++) {
    char wn[8];
    byte a, b;
    WADRES_getname(i, wn);
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
  ERR_fatal("F_getsprid: image %.4s%c%c not found", n, s, d);
  return -1;
}

int F_getreslen (int r) {
  return WADRES_getsize(r);
}

/*
void F_nextmus (char *s) {
  int i = F_findres(s);
  if (i <= m_start || i >= m_end) {
    i = m_start;
  }
  for (++i; ; ++i) {
    if (i >= m_end) {
      i = m_start + 1;
    }
    WADRES_getname(i, s);
    if (cp866_strcasecmp(s, "MENU") == 0 ||
        cp866_strcasecmp(s, "INTERMUS") == 0 ||
        cp866_strcasecmp(s, "\x8a\x8e\x8d\x85\x96\x0") == 0) {
      continue;
    }
    if (cp866_strncasecmp(s, "DMI", 3) != 0) {
      break;
    }
  }
}

void F_randmus (char *s) {
  int i;
  int n = myrand(10);
  for (i = 0; i < n; i++) {
    F_nextmus(s);
  }
}
*/

void F_loadmap (char n[8]) {
  int id = F_getresid(n);
  if (id != -1) {
    Stream *r = WADRES_getbasereader(id);
    long offset = WADRES_getoffset(id);
    stream_setpos(r, offset);
    if (!MAP_load(r)) {
      ERR_fatal("Failed to load map");
    }
  } else {
    ERR_fatal("Failed to load map: resource %.8s not found", n);
  }
}

static char *getsavfpname (int n, int ro) {
  static char fn[] = "savgame0.dat";
  static char p[100];
  fn[7] = n + '0';
  strcpy(p, fn);
  return p;
}

void F_getsavnames (void) {
  KOS32_Stream rd;
  for (int i = 0; i < SAVE_MAX; ++i) {
    savok[i] = 0;
    char *p = getsavfpname(i, 1);
    if (KOS32_Open(&rd, p)) {
      savok[i] = SAVE_getname(&rd.base, savname[i]);
      KOS32_Close(&rd);
    }
    if (!savok[i]) {
      memset(savname[i], 0, 24);
    } else {
      savname[i][23] = 0;
    }
  }
}

void F_loadgame (int n) {
  KOS32_Stream rd;
  char *p = getsavfpname(n, 1);
  if (KOS32_Open(&rd, p)) {
    SAVE_load(&rd.base);
    KOS32_Close(&rd);
  }
}

void F_savegame (int n, char *s) {
  KOS32_Stream wr;
  char *p = getsavfpname(n, 0);
  if (KOS32_Create(&wr, p)) {
    SAVE_save(&wr.base, s);
    KOS32_Close(&wr);
  }
}
