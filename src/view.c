/*
   Copyright (C) Prikol Software 1996-1997
   Copyright (C) Aleksey Volynskov 1996-1997
   Copyright (C) <ARembo@gmail.com> 2011

   This file is part of the Doom2D:Rembo project.

   Doom2D:Rembo is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   Doom2D:Rembo is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/> or
   write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "glob.h"
#include <string.h>
#include <stdlib.h>
#include "memory.h"
#include "files.h"
#include "error.h"
#include "view.h"
#include "dots.h"
#include "smoke.h"
#include "weapons.h"
#include "items.h"
#include "switch.h"
#include "fx.h"
#include "player.h"
#include "monster.h"
#include "misc.h"
#include "map.h"
#include "sound.h"
#include "my.h"
#include "render.h"

byte w_horiz=ON;
int sky_type=1;
dword walf[256];
byte walswp[256];
byte fldb[FLDH][FLDW];
byte fldf[FLDH][FLDW];
byte fld[FLDH][FLDW];

void W_savegame (FILE* h) {
  char s[8];
  int i;
  myfwrite32(sky_type, h);
  for(i = 1; i < 256; ++i) {
    R_get_name(i, s);
    myfwrite(s, 8, 1, h);
  }
  for (i = 0; i < 256; i++) {
    myfwrite32(walf[i], h);
  }
  for (i = 0; i < 256; i++) {
    myfwrite8(walswp[i], h);
  }
  myfwrite(fldb, FLDW*FLDH, 1, h);
  myfwrite(fld, FLDW*FLDH, 1, h);
  myfwrite(fldf, FLDW*FLDH, 1, h);
}

void W_loadgame (FILE* h) {
  int i;
  char s[256][8];
  sky_type = myfread32(h);
  R_loadsky(sky_type);
  for (i = 1; i < 256; ++i) {
    myfread(s[i], 8, 1, h);
  }
  R_begin_load();
  i = myfread32(h); // ignore
  for (i = 1; i < 256; i++) {
    walf[i] = myfread32(h);
    R_load(s[i], walf[i] & 1);
  }
  for (i = 0; i < 256; i++) {
    //walswp[i] = myfread8(h);
    (void)myfread8(h); // useless in new code
  }
  myfread(fldb, FLDW*FLDH, 1, h);
  myfread(fld, FLDW*FLDH, 1, h);
  myfread(fldf, FLDW*FLDH, 1, h);
  R_end_load();
}

void W_init (void) {
  DOT_init();
  SMK_init();
  FX_init();
  WP_init();
  IT_init();
  SW_init();
  PL_init();
  MN_init();
  R_loadsky(1);
}

static void unpack (void *buf, int len, void *obuf) {
  int i = 0;
  int j = 0;
  unsigned char *p = buf;
  unsigned char *q = obuf;
  while (i < len) {
    int id = p[i];
    int step = 1;
    i += 1;
    if (id == 0xff) {
      step = p[i] | p[i + 1] << 8;
      id = p[i + 2];
      i += 3;
    }
    memset(&q[j], id, step);
    j += step;
  }
}

int W_load (FILE *h) {
  char s[8];
  int i, j, t;
  void *p, *buf;
  switch (blk.t) {
  case MB_WALLNAMES:
    R_begin_load();
    for (i = 1; i < 256 && blk.sz > 0; i++, blk.sz -= 9) {
      myfread(s, 8, 1, h);
      t = myfread8(h);
      R_load(s, t);
      if (strncasecmp(s, "VTRAP01", 8) == 0) {
        walf[i] |= 2;
      }
    }
    R_end_load();
    return 1;
  case MB_BACK:
    p = fldb;
    goto unp;
  case MB_WTYPE:
    p = fld;
    goto unp;
  case MB_FRONT:
    p = fldf;
  unp:
    switch (blk.st) {
      case 0:
        myfread(p, FLDW * FLDH, 1, h);
        break;
      case 1:
        buf = malloc(blk.sz);
        if (buf == NULL) {
          ERR_fatal("Не хватает памяти");
        }
        myfread(buf, blk.sz, 1, h);
        unpack(buf, blk.sz, p);
        free(buf);
        break;
      default:
        return 0;
    }
    return 1;
  case MB_SKY:
    sky_type = myfread16(h);
    R_loadsky(sky_type);
    return 1;
  }
  return 0;
}
