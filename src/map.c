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
#include <assert.h>
#include "my.h"
#include "error.h"
#include "cp866.h"

#include "common/streams.h"
#include "common/files.h"

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

static int G_load (Reader *h) {
  switch (blk.t) {
    case MB_MUSIC:
      stream_read(g_music, 8, 1, h);
      //if (music_random) {
      //  F_randmus(g_music);
      //}
      MUS_load(g_music);
      return 1;
  }
  return 0;
}

static int IT_load (Reader *h) {
  int m, i, j;
  old_thing_t t;
  switch (blk.t) {
	case MB_THING:
	  for (i = 0; blk.sz > 0; ++i, blk.sz -= 8) {
      t.x = stream_read16(h);
      t.y = stream_read16(h);
      t.t = stream_read16(h);
      t.f = stream_read16(h);
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
        logo("Player 1 point not exists on the map\n");
        return 0; // error
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
        logo("Player 2 point not exists on the map\n");
        return 0; // error
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
      logo("Required at least two DM points on the map\n");
      return 0; // error
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

static int SW_load (Reader *h) {
  int i;
  switch(blk.t) {
    case MB_SWITCH2:
      sw_secrets = 0;
      for (i = 0; i < MAXSW && blk.sz > 0; ++i, blk.sz -= 9) {
        sw[i].x = stream_read8(h);
        sw[i].y = stream_read8(h);
        sw[i].t = stream_read8(h);
        sw[i].tm = stream_read8(h); // unused
        sw[i].a = stream_read8(h);
        sw[i].b = stream_read8(h);
        sw[i].c = stream_read8(h);
        sw[i].d = stream_read8(h); // unused
        sw[i].f = stream_read8(h);
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

static int read_array (void *p, Reader *h) {
  void *buf;
  switch (blk.st) {
    case 0:
      stream_read(p, FLDW * FLDH, 1, h);
      break;
    case 1:
      buf = malloc(blk.sz);
      if (buf == NULL) {
        logo("Out of memory\n");
        return 0; // error
      }
      stream_read(buf, blk.sz, 1, h);
      unpack(buf, blk.sz, p);
      free(buf);
      break;
    default:
      return 0;
  }
  return 1;
}

static int W_load (Reader *h) {
  int i;
  char s[8];
  switch (blk.t) {
  case MB_WALLNAMES:
    R_begin_load();
    memset(walf, 0, sizeof(walf));
    for (i = 1; i < 256 && blk.sz > 0; i++, blk.sz -= 9) {
      stream_read(s, 8, 1, h);
      walf[i] = stream_read8(h) ? 1 : 0; // ???
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
    sky_type = stream_read16(h);
    R_loadsky(sky_type);
    return 1;
  }
  return 0;
}

int MAP_load (Reader *r) {
  assert(r != NULL);
  int ok = 0;
  map_header_t hdr;
  W_init(); // reset all game data
  stream_read(hdr.id, 8, 1, r);
  hdr.ver = stream_read16(r);
  if (memcmp(hdr.id, "Doom2D\x1A", 8) == 0) {
    ok = 1;
    while (ok) {
      blk.t = stream_read16(r);
      blk.st = stream_read16(r);
      blk.sz = stream_read32(r);
      long off = r->getpos(r) + blk.sz;
      switch (blk.t) {
        case MB_MUSIC:
          ok = G_load(r);
          break;
        case MB_WALLNAMES:
        case MB_BACK:
        case MB_WTYPE:
        case MB_FRONT:
        case MB_SKY:
          ok = W_load(r);
          break;
        case MB_THING:
          ok = IT_load(r);
          break;
        case MB_SWITCH2:
          ok = SW_load(r);
          break;
        case MB_COMMENT:
          /* skip */
          break;
        case MB_END:
          return ok;
        default:
          logo("Unknown block %d(%d)\n", blk.t, blk.st);
          return 0; // error
      }
      r->setpos(r, off);
    }
  } else {
    logo("Invalid map header\n");
    ok = 0;
  }
  return ok;
}

void F_loadmap (char n[8]) {
  FILE_Reader rd;
  int r = F_getresid(n);
  FILE *h = wadh[wad[r].f];
  fseek(h, wad[r].o, SEEK_SET);
  FILE_AssignReader(&rd, h);
  if (!MAP_load(&rd.base)) {
    ERR_fatal("Failed to load map");
  }
}
