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

#include "map.h"

#include "game.h"
#include "items.h"
#include "things.h"
#include "monster.h"
#include "switch.h"
#include "view.h"

#include "music.h"
#include "files.h"
#include "render.h"

#include <stdio.h>
#include <string.h>
#include "my.h"
#include "error.h"
#include "cp866.h"

enum {
  MB_COMMENT = -1, MB_END = 0,
  MB_WALLNAMES, MB_BACK, MB_WTYPE, MB_FRONT, MB_THING, MB_SWITCH,
  MB_MUSIC, MB_SKY,
  MB_SWITCH2,
  MB__UNKNOWN
};

typedef struct map_header_t {
  char id[8];
  short ver;
} map_header_t;

typedef struct map_block_t {
  short t;
  short st;
  int sz;
} map_block_t;

typedef struct old_thing_t {
  short x, y;
  short t;
  unsigned short f;
} old_thing_t;

static map_block_t blk;

static int G_load (FILE *h) {
  switch (blk.t) {
    case MB_MUSIC:
      myfread(g_music, 8, 1, h);
      //if (music_random) {
      //  F_randmus(g_music);
      //}
      MUS_load(g_music);
      return 1;
  }
  return 0;
}

static int IT_load (FILE *h) {
  int m, i, j;
  old_thing_t t;
  switch (blk.t) {
	case MB_THING:
	  for (i = 0; blk.sz > 0; ++i, blk.sz -= 8) {
      t.x = myfread16(h);
      t.y = myfread16(h);
      t.t = myfread16(h);
      t.f = myfread16(h);
      it[i].o.x = t.x;
      it[i].o.y = t.y;
      it[i].t = t.t;
      it[i].s = t.f;
      if (it[i].t && (it[i].s & THF_DM) && !g_dm) {
        it[i].t=0;
      }
	  }
    m = i;
	  for (i = 0, j = -1; i < m; ++i) {
      if (it[i].t == TH_PLR1) {
        j = i;
        it[i].t = 0;
      }
    }
	  if (!g_dm) {
      if (j == -1) {
        ERR_fatal("Player 1 point not exists on the map");
      }
      dm_pos[0].x = it[j].o.x;
      dm_pos[0].y = it[j].o.y;
      dm_pos[0].d = it[j].s & THF_DIR;
	  }
	  for (i = 0, j = -1; i < m; ++i) {
      if (it[i].t == TH_PLR2) {
        j = i;
        it[i].t = 0;
      }
    }
	  if (!g_dm && _2pl) {
      if (j == -1) {
        ERR_fatal("Player 2 point not exists on the map");
      }
      dm_pos[1].x = it[j].o.x;
      dm_pos[1].y = it[j].o.y;
      dm_pos[1].d = it[j].s & THF_DIR;
	  }
	  for (i = 0, j = 0; i < m; ++i) {
      if (it[i].t == TH_DMSTART) {
        if (g_dm) {
          dm_pos[j].x = it[i].o.x;
          dm_pos[j].y = it[i].o.y;
          dm_pos[j].d = it[i].s & THF_DIR;
        }
        it[i].t = 0;
        ++j;
      }
    }
	  if (g_dm && j < 2) {
      ERR_fatal("Required at least two DM points on the map");
    }
	  if (g_dm) {
	    dm_pnum = j;
	    dm_pl1p = myrand(dm_pnum);
	    do {
        dm_pl2p = myrand(dm_pnum);
      } while (dm_pl2p == dm_pl1p);
	  } else {
      dm_pl1p = 0;
      dm_pl2p = 1;
      dm_pnum = 2;
    }
	  PL_spawn(&pl1, dm_pos[dm_pl1p].x, dm_pos[dm_pl1p].y, dm_pos[dm_pl1p].d);
	  if (_2pl) {
      PL_spawn(&pl2, dm_pos[dm_pl2p].x, dm_pos[dm_pl2p].y, dm_pos[dm_pl2p].d);
    }
	  for (i = 0; i < m; ++i) {
      if (it[i].t >= TH_CLIP && it[i].t < TH_DEMON) {
        it[i].s = 0;
        it[i].t = it[i].t - TH_CLIP + I_CLIP;
        if (it[i].t >= I_KEYR && it[i].t <= I_KEYB) {
          it[i].t |= 0x8000;
        }
      } else if (it[i].t >= TH_DEMON) {
        MN_spawn(it[i].o.x, it[i].o.y, it[i].s & THF_DIR, it[i].t - TH_DEMON + MN_DEMON);
        it[i].t = 0;
      }
    }
	  return 1;
  }
  return 0;
}

static int SW_load (FILE *h) {
  int i;
  switch(blk.t) {
    case MB_SWITCH2:
      sw_secrets = 0;
      for (i = 0; i < MAXSW && blk.sz > 0; ++i, blk.sz -= 9) {
        sw[i].x = myfread8(h);
        sw[i].y = myfread8(h);
        sw[i].t = myfread8(h);
        sw[i].tm = myfread8(h); // unused
        sw[i].a = myfread8(h);
        sw[i].b = myfread8(h);
        sw[i].c = myfread8(h);
        sw[i].d = myfread8(h); // unused
        sw[i].f = myfread8(h);
        sw[i].tm = 0;
        sw[i].d = 0;
        sw[i].f |= 0x80;
        if (sw[i].t == SW_SECRET) {
          ++sw_secrets;
        }
      }
      return 1;
  }
  return 0;
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

static int read_array (void *p, FILE *h) {
  void *buf;
  switch (blk.st) {
    case 0:
      myfread(p, FLDW * FLDH, 1, h);
      break;
    case 1:
      buf = malloc(blk.sz);
      if (buf == NULL) {
        ERR_fatal("Out of memory");
      }
      myfread(buf, blk.sz, 1, h);
      unpack(buf, blk.sz, p);
      free(buf);
      break;
    default:
      return 0;
  }
  return 1;
}

static int W_load (FILE *h) {
  int i;
  char s[8];
  switch (blk.t) {
  case MB_WALLNAMES:
    R_begin_load();
    memset(walf, 0, sizeof(walf));
    for (i = 1; i < 256 && blk.sz > 0; i++, blk.sz -= 9) {
      myfread(s, 8, 1, h);
      walf[i] = myfread8(h) ? 1 : 0; // ???
      R_load(s);
      if (cp866_strncasecmp(s, "VTRAP01", 8) == 0) {
        walf[i] |= 2;
      }
    }
    R_end_load();
    return 1;
  case MB_BACK:
    return read_array(fldb, h);
  case MB_WTYPE:
    return read_array(fld, h);
  case MB_FRONT:
    return read_array(fldf, h);
  case MB_SKY:
    sky_type = myfread16(h);
    R_loadsky(sky_type);
    return 1;
  }
  return 0;
}

void F_loadmap (char n[8]) {
  int r, o;
  FILE *h;
  map_header_t hdr;
  W_init();
  r = F_getresid(n);
  h = wadh[wad[r].f];
  fseek(h, wad[r].o, SEEK_SET);
  myfread(hdr.id, 8, 1, h);
  hdr.ver = myfread16(h);
  if (memcmp(hdr.id, "Doom2D\x1A", 8) != 0) {
    ERR_fatal("%.8s not map", n);
  }
  for (;;) {
    blk.t = myfread16(h);
    blk.st = myfread16(h);
    blk.sz = myfread32(h);
    if(blk.t == MB_END) {
      break;
    }
    if(blk.t == MB_COMMENT) {
      fseek(h, blk.sz, SEEK_CUR);
      continue;
    }
    o = ftell(h) + blk.sz;
    if(!G_load(h)) {
      if(!W_load(h)) {
        if(!IT_load(h)) {
          if(!SW_load(h)) {
            ERR_fatal("Unknown block %d(%d) on map %.8s", blk.t, blk.st, n);
          }
        }
      }
    }
    fseek(h, o, SEEK_SET);
  }
}
