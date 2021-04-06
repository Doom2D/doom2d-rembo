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

#include "save.h"

#include "dots.h"
#include "fx.h"
#include "game.h"
#include "items.h"
#include "monster.h"
#include "player.h"
#include "smoke.h"
#include "switch.h"
#include "view.h"
#include "weapons.h"

#include "render.h"
#include "music.h"

#include <string.h>
#ifdef UNIX
#  include <sys/stat.h>
#endif
#include <assert.h>
#include "files.h"

#include "common/streams.h"
#include "common/files.h"

char savname[SAVE_MAX][SAVE_MAXLEN];
char savok[SAVE_MAX];

static void DOT_savegame (Writer *h) {
  int i, n;
  for (i = n = 0; i < MAXDOT; ++i) {
    if (dot[i].t) {
      ++n;
    }
  }
  stream_write32(n, h);
  for (i = 0; i < MAXDOT; ++i) {
    if (dot[i].t) {
      stream_write32(dot[i].o.x, h);
      stream_write32(dot[i].o.y, h);
      stream_write32(dot[i].o.xv, h);
      stream_write32(dot[i].o.yv, h);
      stream_write32(dot[i].o.vx, h);
      stream_write32(dot[i].o.vy, h);
      stream_write32(dot[i].o.r, h);
      stream_write32(dot[i].o.h, h);
      stream_write8(dot[i].c, h);
      stream_write8(dot[i].t, h);
    }
  }
}

static void DOT_loadgame (Reader *h) {
  int i, n;
  n = stream_read32(h);
  for (i = 0; i < n; i++) {
    dot[i].o.x = stream_read32(h);
    dot[i].o.y = stream_read32(h);
    dot[i].o.xv = stream_read32(h);
    dot[i].o.yv = stream_read32(h);
    dot[i].o.vx = stream_read32(h);
    dot[i].o.vy = stream_read32(h);
    dot[i].o.r = stream_read32(h);
    dot[i].o.h = stream_read32(h);
    dot[i].c = stream_read8(h);
    dot[i].t = stream_read8(h);
  }
}

static void FX_savegame (Writer *h) {
  int i, n;
  for (i = n = 0; i < MAXFX; ++i) {
    if (fx[i].t) {
      ++n;
    }
  }
  stream_write32(n, h);
  for (i = 0; i < MAXFX; ++i) {
    if (fx[i].t) {
      stream_write32(fx[i].x, h);
      stream_write32(fx[i].y, h);
      stream_write32(fx[i].xv, h);
      stream_write32(fx[i].yv, h);
      stream_write8(fx[i].t, h);
      stream_write8(fx[i].s, h);
    }
  }
}

static void FX_loadgame (Reader *h) {
  int i, n;
  n = stream_read32(h);
  for (i = 0; i < n; i++) {
    fx[i].x = stream_read32(h);
    fx[i].y = stream_read32(h);
    fx[i].xv = stream_read32(h);
    fx[i].yv = stream_read32(h);
    fx[i].t = stream_read8(h);
    fx[i].s = stream_read8(h);
  }
}

static void G_savegame (Writer *h) {
  stream_write8(_2pl, h);
  stream_write8(g_dm, h);
  stream_write8(g_exit, h);
  stream_write8(g_map, h);
  stream_write32(g_time, h);
  stream_write32(dm_pl1p, h);
  stream_write32(dm_pl2p, h);
  stream_write32(dm_pnum, h);
  int i = 0;
  while (i < dm_pnum) {
    stream_write32(dm_pos[i].x, h);
    stream_write32(dm_pos[i].y, h);
    stream_write8(dm_pos[i].d, h);
    i += 1;
  }
  stream_write8(cheat, h);
  stream_write(g_music, 8, 1, h);
}

static void G_loadgame (Reader *h) {
  _2pl = stream_read8(h);
  g_dm = stream_read8(h);
  g_exit = stream_read8(h);
  g_map = stream_read8(h);
  g_time = stream_read32(h);
  dm_pl1p = stream_read32(h);
  dm_pl2p = stream_read32(h);
  dm_pnum = stream_read32(h);
  int i = 0;
  while (i < dm_pnum) {
    dm_pos[i].x = stream_read32(h);
    dm_pos[i].y = stream_read32(h);
    dm_pos[i].d = stream_read8(h);
    i += 1;
  }
  cheat = stream_read8(h);
  stream_read(g_music, 8, 1, h);
  MUS_load(g_music);
}

static void IT_savegame (Writer *h) {
  int i, n;
  for (n = MAXITEM - 1; n >= 0 && it[n].t == 0; n--) {
    // empty
  }
  n += 1;
  stream_write32(n, h);
  for (i = 0; i < n; i++) {
    stream_write32(it[i].o.x, h);
    stream_write32(it[i].o.y, h);
    stream_write32(it[i].o.xv, h);
    stream_write32(it[i].o.yv, h);
    stream_write32(it[i].o.vx, h);
    stream_write32(it[i].o.vy, h);
    stream_write32(it[i].o.r, h);
    stream_write32(it[i].o.h, h);
    stream_write32(it[i].t, h);
    stream_write32(it[i].s, h);
  }
  stream_write32(itm_rtime, h);
}

static void IT_loadgame (Reader *h) {
  int i, n;
  n = stream_read32(h);
  for (i = 0; i < n; i++) {
    it[i].o.x = stream_read32(h);
    it[i].o.y = stream_read32(h);
    it[i].o.xv = stream_read32(h);
    it[i].o.yv = stream_read32(h);
    it[i].o.vx = stream_read32(h);
    it[i].o.vy = stream_read32(h);
    it[i].o.r = stream_read32(h);
    it[i].o.h = stream_read32(h);
    it[i].t = stream_read32(h);
    it[i].s = stream_read32(h);
  }
  itm_rtime = stream_read32(h);
}

static void MN_savegame (Writer *h) {
  int i, n;
  for (n = MAXMN - 1; n >= 0 && mn[n].t == 0; n--) {
    // empty
  }
  n += 1;
  stream_write32(n, h);
  for (i = 0; i < n; i++) {
    stream_write32(mn[i].o.x, h);
    stream_write32(mn[i].o.y, h);
    stream_write32(mn[i].o.xv, h);
    stream_write32(mn[i].o.yv, h);
    stream_write32(mn[i].o.vx, h);
    stream_write32(mn[i].o.vy, h);
    stream_write32(mn[i].o.r, h);
    stream_write32(mn[i].o.h, h);
    stream_write8(mn[i].t, h);
    stream_write8(mn[i].d, h);
    stream_write8(mn[i].st, h);
    stream_write8(mn[i].ftime, h);
    stream_write32(mn[i].fobj, h);
    stream_write32(mn[i].s, h);
    stream_write32(0, h); // mn[i].ap useless, changed after load
    stream_write32(mn[i].aim, h);
    stream_write32(mn[i].life, h);
    stream_write32(mn[i].pain, h);
    stream_write32(mn[i].ac, h);
    stream_write32(mn[i].tx, h);
    stream_write32(mn[i].ty, h);
    stream_write32(mn[i].ammo, h);
    stream_write16(mn[i].atm, h);
  }
  stream_write32(mnum, h);
  stream_write32(gsndt, h);
}

static void MN_loadgame (Reader *h) {
  int i, n, c;
  n = stream_read32(h);
  for (i = 0; i < n; i++) {
    mn[i].o.x = stream_read32(h);
    mn[i].o.y = stream_read32(h);
    mn[i].o.xv = stream_read32(h);
    mn[i].o.yv = stream_read32(h);
    mn[i].o.vx = stream_read32(h);
    mn[i].o.vy = stream_read32(h);
    mn[i].o.r = stream_read32(h);
    mn[i].o.h = stream_read32(h);
    mn[i].t = stream_read8(h);
    mn[i].d = stream_read8(h);
    mn[i].st = stream_read8(h);
    mn[i].ftime = stream_read8(h);
    mn[i].fobj = stream_read32(h);
    mn[i].s = stream_read32(h);
    mn[i].ap = NULL; stream_read32(h); // useless, changed after loading
    mn[i].aim = stream_read32(h);
    mn[i].life = stream_read32(h);
    mn[i].pain = stream_read32(h);
    mn[i].ac = stream_read32(h);
    mn[i].tx = stream_read32(h);
    mn[i].ty = stream_read32(h);
    mn[i].ammo = stream_read32(h);
    mn[i].atm = stream_read16(h);
  }
  mnum = stream_read32(h);
  gsndt = stream_read32(h);
  for (n = 0; n < MAXMN; n++) {
    if (mn[n].t) {
      c = mn[n].ac;
      setst(n, mn[n].st);
      mn[n].ac = c;
    }
  }
}

static void PL_save_player (player_t *p, Writer *h) {
  stream_write32(p->o.x, h);
  stream_write32(p->o.y, h);
  stream_write32(p->o.xv, h);
  stream_write32(p->o.yv, h);
  stream_write32(p->o.vx, h);
  stream_write32(p->o.vy, h);
  stream_write32(p->o.r, h);
  stream_write32(p->o.h, h);
  stream_write32(p->looky, h);
  stream_write32(p->st, h);
  stream_write32(p->s, h);
  stream_write32(p->life, h);
  stream_write32(p->armor, h);
  stream_write32(p->hit, h);
  stream_write32(p->hito, h);
  stream_write32(p->pain, h);
  stream_write32(p->air, h);
  stream_write32(p->invl, h);
  stream_write32(p->suit, h);
  stream_write8(p->d, h);
  stream_write32(p->frag, h);
  stream_write32(p->ammo, h);
  stream_write32(p->shel, h);
  stream_write32(p->rock, h);
  stream_write32(p->cell, h);
  stream_write32(p->fuel, h);
  stream_write32(p->kills, h);
  stream_write32(p->secrets, h);
  stream_write8(p->fire, h);
  stream_write8(p->cwpn, h);
  stream_write8(p->csnd, h);
  stream_write8(p->amul, h);
  stream_write16(p->wpns, h);
  stream_write8(p->wpn, h);
  stream_write8(p->f, h);
  stream_write8(p->drawst, h);
  stream_write8(p->color, h);
  stream_write32(p->id, h);
  stream_write8(p->keys, h);
  stream_write8(p->lives, h);
  // k* not saved
}

static void PL_savegame (Writer *h) {
  PL_save_player(&pl1, h);
  if (_2pl) {
    PL_save_player(&pl2, h);
  }
  stream_write32(PL_JUMP, h);
  stream_write32(PL_RUN, h);
  stream_write8(p_immortal, h);
}

static void PL_load_player (player_t *p, Reader *h) {
  p->o.x = stream_read32(h);
  p->o.y = stream_read32(h);
  p->o.xv = stream_read32(h);
  p->o.yv = stream_read32(h);
  p->o.vx = stream_read32(h);
  p->o.vy = stream_read32(h);
  p->o.r = stream_read32(h);
  p->o.h = stream_read32(h);
  p->looky = stream_read32(h);
  p->st = stream_read32(h);
  p->s = stream_read32(h);
  p->life = stream_read32(h);
  p->armor = stream_read32(h);
  p->hit = stream_read32(h);
  p->hito = stream_read32(h);
  p->pain = stream_read32(h);
  p->air = stream_read32(h);
  p->invl = stream_read32(h);
  p->suit = stream_read32(h);
  p->d = stream_read8(h);
  p->frag = stream_read32(h);
  p->ammo = stream_read32(h);
  p->shel = stream_read32(h);
  p->rock = stream_read32(h);
  p->cell = stream_read32(h);
  p->fuel = stream_read32(h);
  p->kills = stream_read32(h);
  p->secrets = stream_read32(h);
  p->fire = stream_read8(h);
  p->cwpn = stream_read8(h);
  p->csnd = stream_read8(h);
  p->amul = stream_read8(h);
  p->wpns = stream_read16(h);
  p->wpn = stream_read8(h);
  p->f = stream_read8(h);
  p->drawst = stream_read8(h);
  p->color = stream_read8(h);
  p->id = stream_read32(h);
  p->keys = stream_read8(h);
  p->lives = stream_read8(h);
  // k* not saved
}

static void PL_loadgame (Reader *h) {
  PL_load_player(&pl1, h);
  if (_2pl) {
    PL_load_player(&pl2, h);
  }
  PL_JUMP = stream_read32(h);
  PL_RUN = stream_read32(h);
  p_immortal = stream_read8(h);
}

static void SMK_savegame (Writer *h) {
  int i, n;
  for (i = n = 0; i < MAXSMOK; ++i) {
    if (sm[i].t) {
      ++n;
    }
  }
  stream_write32(n, h);
  for (i = 0; i < MAXSMOK; ++i) {
    if (sm[i].t) {
      stream_write32(sm[i].x, h);
      stream_write32(sm[i].y, h);
      stream_write32(sm[i].xv, h);
      stream_write32(sm[i].xv, h);
      stream_write8(sm[i].t, h);
      stream_write8(sm[i].s, h);
      stream_write16(sm[i].o, h);
    }
  }
}

static void SMK_loadgame (Reader *h) {
  int i, n;
  n = stream_read32(h);
  for (i = 0; i < n; ++i) {
    sm[i].x = stream_read32(h);
    sm[i].y = stream_read32(h);
    sm[i].xv = stream_read32(h);
    sm[i].xv = stream_read32(h);
    sm[i].t = stream_read8(h);
    sm[i].s = stream_read8(h);
    sm[i].o = stream_read16(h);
  }
}

static void SW_savegame (Writer *h) {
  int i, n;
  for (n = MAXSW - 1; n >= 0 && sw[n].t == 0; n--) {
    // empty
  }
  n += 1;
  stream_write32(n, h);
  for (i = 0; i < n; i++) {
    stream_write8(sw[i].x, h);
    stream_write8(sw[i].y, h);
    stream_write8(sw[i].t, h);
    stream_write8(sw[i].tm, h);
    stream_write8(sw[i].a, h);
    stream_write8(sw[i].b, h);
    stream_write8(sw[i].c, h);
    stream_write8(sw[i].d, h);
    stream_write8(sw[i].f, h);
  }
  stream_write32(sw_secrets, h);
}

static void SW_loadgame (Reader *h) {
  int i, n;
  n = stream_read32(h);
  for (i = 0; i < n; i++) {
    sw[i].x = stream_read8(h);
    sw[i].y = stream_read8(h);
    sw[i].t = stream_read8(h);
    sw[i].tm = stream_read8(h);
    sw[i].a = stream_read8(h);
    sw[i].b = stream_read8(h);
    sw[i].c = stream_read8(h);
    sw[i].d = stream_read8(h);
    sw[i].f = stream_read8(h);
  }
  sw_secrets = stream_read32(h);
}

static void W_savegame (Writer* h) {
  char s[8];
  int i;
  stream_write32(sky_type, h);
  for(i = 1; i < 256; ++i) {
    R_get_name(i, s);
    stream_write(s, 8, 1, h);
  }
  for (i = 0; i < 256; i++) {
    stream_write32(walf[i], h);
  }
  for (i = 0; i < 256; i++) {
    stream_write8(R_get_swp(i), h);
  }
  stream_write(fldb, FLDW*FLDH, 1, h);
  stream_write(fld, FLDW*FLDH, 1, h);
  stream_write(fldf, FLDW*FLDH, 1, h);
}

static void W_loadgame (Reader *h) {
  int i;
  char s[8];
  sky_type = stream_read32(h);
  R_loadsky(sky_type);
  R_begin_load();
  for (i = 1; i < 256; ++i) {
    stream_read(s, 8, 1, h);
    if (s[0]) {
      R_load(s);
    }
  }
  R_end_load();
  for (i = 0; i < 256; i++) {
    stream_read32(h); // useless
  }
  for (i = 0; i < 256; i++) {
    walf[i] = stream_read8(h);
  }
  stream_read(fldb, FLDW*FLDH, 1, h);
  stream_read(fld, FLDW*FLDH, 1, h);
  stream_read(fldf, FLDW*FLDH, 1, h);
}

static void WP_savegame (Writer *h) {
  int i, n;
  for (n = MAXWPN - 1; n >= 0 && wp[n].t == 0; n--) {
    // empty
  }
  n += 1;
  stream_write32(n, h);
  for (i = 0; i < n; i++) {
    stream_write32(wp[i].o.x, h);
    stream_write32(wp[i].o.y, h);
    stream_write32(wp[i].o.xv, h);
    stream_write32(wp[i].o.yv, h);
    stream_write32(wp[i].o.vx, h);
    stream_write32(wp[i].o.vy, h);
    stream_write32(wp[i].o.r, h);
    stream_write32(wp[i].o.h, h);
    stream_write8(wp[i].t, h);
    stream_write8(wp[i].s, h);
    stream_write32(wp[i].own, h);
    stream_write16(wp[i].target, h);
  }
}

static void WP_loadgame (Reader *h) {
  int i, n;
  n = stream_read32(h);
  for (i = 0; i < n; i++) {
    wp[i].o.x = stream_read32(h);
    wp[i].o.y = stream_read32(h);
    wp[i].o.xv = stream_read32(h);
    wp[i].o.yv = stream_read32(h);
    wp[i].o.vx = stream_read32(h);
    wp[i].o.vy = stream_read32(h);
    wp[i].o.r = stream_read32(h);
    wp[i].o.h = stream_read32(h);
    wp[i].t = stream_read8(h);
    wp[i].s = stream_read8(h);
    wp[i].own = stream_read32(h);
    wp[i].target = stream_read16(h);
  }
}

static char *getsavfpname (int n, int ro) {
  static char fn[] = "savgame0.dat";
  static char p[100];
  fn[7] = n + '0';
#ifdef UNIX
  char *e = getenv("HOME");
  strncpy(p, e, 60);
  strcat(p, "/.flatwaifu");
  if (!ro) {
    mkdir(p, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
  strcat(p, "/");
  strcat(p, fn);
#else
  strcpy(p, fn);
#endif
  return p;
}

void SAVE_save (Writer *w, const char name[24]) {
  assert(w != NULL);
  stream_write(name, 24, 1, w); // slot name
  stream_write16(3, w); // version
  G_savegame(w);
  W_savegame(w);
  DOT_savegame(w);
  SMK_savegame(w);
  FX_savegame(w);
  IT_savegame(w);
  MN_savegame(w);
  PL_savegame(w);
  SW_savegame(w);
  WP_savegame(w);
}

void F_savegame (int n, char *s) {
  FILE_Writer wr;
  char *p = getsavfpname(n, 0);
  if (FILE_OpenWriter(&wr, p)) {
    SAVE_save(&wr.base, s);
    FILE_CloseWriter(&wr);
  }
}

void SAVE_load (Reader *h) {
  int16_t version;
  h->setpos(h, 24); // skip name
  version = stream_read16(h);
  if (version == 3) {
    G_loadgame(h);
    W_loadgame(h);
    DOT_loadgame(h);
    SMK_loadgame(h);
    FX_loadgame(h);
    IT_loadgame(h);
    MN_loadgame(h);
    PL_loadgame(h);
    SW_loadgame(h);
    WP_loadgame(h);
  }
}

void F_getsavnames (void) {
  int i;
  char *p;
  FILE_Reader rd;
  int16_t version;
  for (i = 0; i < 7; ++i) {
    p = getsavfpname(i, 1);
    memset(savname[i], 0, 24);
    savok[i] = 0;
    if (FILE_OpenReader(&rd, p)) {
      version = -1;
      stream_read(savname[i], 24, 1, &rd.base);
      version = stream_read16(&rd.base);
      savname[i][23] = 0;
      savok[i] = version == 3;
      FILE_CloseReader(&rd);
    }
  }
}

void F_loadgame (int n) {
  FILE_Reader rd;
  char *p = getsavfpname(n, 1);
  if (FILE_OpenReader(&rd, p)) {
    SAVE_load(&rd.base);
    FILE_CloseReader(&rd);
  }
}
