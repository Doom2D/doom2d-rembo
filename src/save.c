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

#include <stdio.h> // FILE
#include <string.h>
#include <sys/stat.h>
#include "files.h"
#include "my.h"

char savname[SAVE_MAX][SAVE_MAXLEN];
char savok[SAVE_MAX];

static void DOT_savegame (FILE *h) {
  int i, n;
  for (i = n = 0; i < MAXDOT; ++i) {
    if (dot[i].t) {
      ++n;
    }
  }
  myfwrite32(n, h);
  for (i = 0; i < MAXDOT; ++i) {
    if (dot[i].t) {
      myfwrite32(dot[i].o.x, h);
      myfwrite32(dot[i].o.y, h);
      myfwrite32(dot[i].o.xv, h);
      myfwrite32(dot[i].o.yv, h);
      myfwrite32(dot[i].o.vx, h);
      myfwrite32(dot[i].o.vy, h);
      myfwrite32(dot[i].o.r, h);
      myfwrite32(dot[i].o.h, h);
      myfwrite8(dot[i].c, h);
      myfwrite8(dot[i].t, h);
    }
  }
}

static void DOT_loadgame (FILE *h) {
  int i, n;
  n = myfread32(h);
  for (i = 0; i < n; i++) {
    dot[i].o.x = myfread32(h);
    dot[i].o.y = myfread32(h);
    dot[i].o.xv = myfread32(h);
    dot[i].o.yv = myfread32(h);
    dot[i].o.vx = myfread32(h);
    dot[i].o.vy = myfread32(h);
    dot[i].o.r = myfread32(h);
    dot[i].o.h = myfread32(h);
    dot[i].c = myfread8(h);
    dot[i].t = myfread8(h);
  }
}

static void FX_savegame (FILE *h) {
  int i, n;
  for (i = n = 0; i < MAXFX; ++i) {
    if (fx[i].t) {
      ++n;
    }
  }
  myfwrite32(n, h);
  for (i = 0; i < MAXFX; ++i) {
    if (fx[i].t) {
      myfwrite32(fx[i].x, h);
      myfwrite32(fx[i].y, h);
      myfwrite32(fx[i].xv, h);
      myfwrite32(fx[i].yv, h);
      myfwrite8(fx[i].t, h);
      myfwrite8(fx[i].s, h);
    }
  }
}

static void FX_loadgame (FILE *h) {
  int i, n;
  n = myfread32(h);
  for (i = 0; i < n; i++) {
    fx[i].x = myfread32(h);
    fx[i].y = myfread32(h);
    fx[i].xv = myfread32(h);
    fx[i].yv = myfread32(h);
    fx[i].t = myfread8(h);
    fx[i].s = myfread8(h);
  }
}

static void G_savegame (FILE *h) {
  myfwrite8(_2pl, h);
  myfwrite8(g_dm, h);
  myfwrite8(g_exit, h);
  myfwrite8(g_map, h);
  myfwrite32(g_time, h);
  myfwrite32(dm_pl1p, h);
  myfwrite32(dm_pl2p, h);
  myfwrite32(dm_pnum, h);
  int i = 0;
  while (i < dm_pnum) {
    myfwrite32(dm_pos[i].x, h);
    myfwrite32(dm_pos[i].y, h);
    myfwrite8(dm_pos[i].d, h);
    i += 1;
  }
  myfwrite8(cheat, h);
  myfwrite(g_music, 8, 1, h);
}

static void G_loadgame (FILE *h) {
  _2pl = myfread8(h);
  g_dm = myfread8(h);
  g_exit = myfread8(h);
  g_map = myfread8(h);
  g_time = myfread32(h);
  dm_pl1p = myfread32(h);
  dm_pl2p = myfread32(h);
  dm_pnum = myfread32(h);
  int i = 0;
  while (i < dm_pnum) {
    dm_pos[i].x = myfread32(h);
    dm_pos[i].y = myfread32(h);
    dm_pos[i].d = myfread8(h);
    i += 1;
  }
  cheat = myfread8(h);
  myfread(g_music, 8, 1, h);
  F_loadmus(g_music);
}

static void IT_savegame (FILE *h) {
  int i, n;
  for (n = MAXITEM - 1; n >= 0 && it[n].t == 0; n--) {
    // empty
  }
  n += 1;
  myfwrite32(n, h);
  for (i = 0; i < n; i++) {
    myfwrite32(it[i].o.x, h);
    myfwrite32(it[i].o.y, h);
    myfwrite32(it[i].o.xv, h);
    myfwrite32(it[i].o.yv, h);
    myfwrite32(it[i].o.vx, h);
    myfwrite32(it[i].o.vy, h);
    myfwrite32(it[i].o.r, h);
    myfwrite32(it[i].o.h, h);
    myfwrite32(it[i].t, h);
    myfwrite32(it[i].s, h);
  }
  myfwrite32(itm_rtime, h);
}

static void IT_loadgame (FILE *h) {
  int i, n;
  n = myfread32(h);
  for (i = 0; i < n; i++) {
    it[i].o.x = myfread32(h);
    it[i].o.y = myfread32(h);
    it[i].o.xv = myfread32(h);
    it[i].o.yv = myfread32(h);
    it[i].o.vx = myfread32(h);
    it[i].o.vy = myfread32(h);
    it[i].o.r = myfread32(h);
    it[i].o.h = myfread32(h);
    it[i].t = myfread32(h);
    it[i].s = myfread32(h);
  }
  itm_rtime = myfread32(h);
}

static void MN_savegame (FILE *h) {
  int i, n;
  for (n = MAXMN - 1; n >= 0 && mn[n].t == 0; n--) {
    // empty
  }
  n += 1;
  myfwrite32(n, h);
  for (i = 0; i < n; i++) {
    myfwrite32(mn[i].o.x, h);
    myfwrite32(mn[i].o.y, h);
    myfwrite32(mn[i].o.xv, h);
    myfwrite32(mn[i].o.yv, h);
    myfwrite32(mn[i].o.vx, h);
    myfwrite32(mn[i].o.vy, h);
    myfwrite32(mn[i].o.r, h);
    myfwrite32(mn[i].o.h, h);
    myfwrite8(mn[i].t, h);
    myfwrite8(mn[i].d, h);
    myfwrite8(mn[i].st, h);
    myfwrite8(mn[i].ftime, h);
    myfwrite32(mn[i].fobj, h);
    myfwrite32(mn[i].s, h);
    myfwrite32(0, h); // mn[i].ap useless, changed after load
    myfwrite32(mn[i].aim, h);
    myfwrite32(mn[i].life, h);
    myfwrite32(mn[i].pain, h);
    myfwrite32(mn[i].ac, h);
    myfwrite32(mn[i].tx, h);
    myfwrite32(mn[i].ty, h);
    myfwrite32(mn[i].ammo, h);
    myfwrite16(mn[i].atm, h);
  }
  myfwrite32(mnum, h);
  myfwrite32(gsndt, h);
}

static void MN_loadgame (FILE *h) {
  int i, n, c;
  n = myfread32(h);
  for (i = 0; i < n; i++) {
    mn[i].o.x = myfread32(h);
    mn[i].o.y = myfread32(h);
    mn[i].o.xv = myfread32(h);
    mn[i].o.yv = myfread32(h);
    mn[i].o.vx = myfread32(h);
    mn[i].o.vy = myfread32(h);
    mn[i].o.r = myfread32(h);
    mn[i].o.h = myfread32(h);
    mn[i].t = myfread8(h);
    mn[i].d = myfread8(h);
    mn[i].st = myfread8(h);
    mn[i].ftime = myfread8(h);
    mn[i].fobj = myfread32(h);
    mn[i].s = myfread32(h);
    mn[i].ap = NULL; myfread32(h); // useless, changed after loading
    mn[i].aim = myfread32(h);
    mn[i].life = myfread32(h);
    mn[i].pain = myfread32(h);
    mn[i].ac = myfread32(h);
    mn[i].tx = myfread32(h);
    mn[i].ty = myfread32(h);
    mn[i].ammo = myfread32(h);
    mn[i].atm = myfread16(h);
  }
  mnum = myfread32(h);
  gsndt = myfread32(h);
  for (n = 0; n < MAXMN; n++) {
    if (mn[n].t) {
      c = mn[n].ac;
      setst(n, mn[n].st);
      mn[n].ac = c;
    }
  }
}

static void PL_save_player (player_t *p, FILE *h) {
  myfwrite32(p->o.x, h);
  myfwrite32(p->o.y, h);
  myfwrite32(p->o.xv, h);
  myfwrite32(p->o.yv, h);
  myfwrite32(p->o.vx, h);
  myfwrite32(p->o.vy, h);
  myfwrite32(p->o.r, h);
  myfwrite32(p->o.h, h);
  myfwrite32(p->looky, h);
  myfwrite32(p->st, h);
  myfwrite32(p->s, h);
  myfwrite32(p->life, h);
  myfwrite32(p->armor, h);
  myfwrite32(p->hit, h);
  myfwrite32(p->hito, h);
  myfwrite32(p->pain, h);
  myfwrite32(p->air, h);
  myfwrite32(p->invl, h);
  myfwrite32(p->suit, h);
  myfwrite8(p->d, h);
  myfwrite32(p->frag, h);
  myfwrite32(p->ammo, h);
  myfwrite32(p->shel, h);
  myfwrite32(p->rock, h);
  myfwrite32(p->cell, h);
  myfwrite32(p->fuel, h);
  myfwrite32(p->kills, h);
  myfwrite32(p->secrets, h);
  myfwrite8(p->fire, h);
  myfwrite8(p->cwpn, h);
  myfwrite8(p->csnd, h);
  myfwrite8(p->amul, h);
  myfwrite16(p->wpns, h);
  myfwrite8(p->wpn, h);
  myfwrite8(p->f, h);
  myfwrite8(p->drawst, h);
  myfwrite8(p->color, h);
  myfwrite32(p->id, h);
  myfwrite8(p->keys, h);
  myfwrite8(p->lives, h);
  // k* not saved
}

static void PL_savegame (FILE *h) {
  PL_save_player(&pl1, h);
  if (_2pl) {
    PL_save_player(&pl2, h);
  }
  myfwrite32(PL_JUMP, h);
  myfwrite32(PL_RUN, h);
  myfwrite8(p_immortal, h);
}

static void PL_load_player (player_t *p, FILE *h) {
  p->o.x = myfread32(h);
  p->o.y = myfread32(h);
  p->o.xv = myfread32(h);
  p->o.yv = myfread32(h);
  p->o.vx = myfread32(h);
  p->o.vy = myfread32(h);
  p->o.r = myfread32(h);
  p->o.h = myfread32(h);
  p->looky = myfread32(h);
  p->st = myfread32(h);
  p->s = myfread32(h);
  p->life = myfread32(h);
  p->armor = myfread32(h);
  p->hit = myfread32(h);
  p->hito = myfread32(h);
  p->pain = myfread32(h);
  p->air = myfread32(h);
  p->invl = myfread32(h);
  p->suit = myfread32(h);
  p->d = myfread8(h);
  p->frag = myfread32(h);
  p->ammo = myfread32(h);
  p->shel = myfread32(h);
  p->rock = myfread32(h);
  p->cell = myfread32(h);
  p->fuel = myfread32(h);
  p->kills = myfread32(h);
  p->secrets = myfread32(h);
  p->fire = myfread8(h);
  p->cwpn = myfread8(h);
  p->csnd = myfread8(h);
  p->amul = myfread8(h);
  p->wpns = myfread16(h);
  p->wpn = myfread8(h);
  p->f = myfread8(h);
  p->drawst = myfread8(h);
  p->color = myfread8(h);
  p->id = myfread32(h);
  p->keys = myfread8(h);
  p->lives = myfread8(h);
  // k* not saved
}

static void PL_loadgame (FILE *h) {
  PL_load_player(&pl1, h);
  if (_2pl) {
    PL_load_player(&pl2, h);
  }
  PL_JUMP = myfread32(h);
  PL_RUN = myfread32(h);
  p_immortal = myfread8(h);
}

static void SMK_savegame (FILE *h) {
  int i, n;
  for (i = n = 0; i < MAXSMOK; ++i) {
    if (sm[i].t) {
      ++n;
    }
  }
  myfwrite32(n, h);
  for (i = 0; i < MAXSMOK; ++i) {
    if (sm[i].t) {
      myfwrite32(sm[i].x, h);
      myfwrite32(sm[i].y, h);
      myfwrite32(sm[i].xv, h);
      myfwrite32(sm[i].xv, h);
      myfwrite8(sm[i].t, h);
      myfwrite8(sm[i].s, h);
      myfwrite16(sm[i].o, h);
    }
  }
}

static void SMK_loadgame (FILE *h) {
  int i, n;
  n = myfread32(h);
  for (i = 0; i < n; ++i) {
    sm[i].x = myfread32(h);
    sm[i].y = myfread32(h);
    sm[i].xv = myfread32(h);
    sm[i].xv = myfread32(h);
    sm[i].t = myfread8(h);
    sm[i].s = myfread8(h);
    sm[i].o = myfread16(h);
  }
}

static void SW_savegame (FILE *h) {
  int i, n;
  for (n = MAXSW - 1; n >= 0 && sw[n].t == 0; n--) {
    // empty
  }
  n += 1;
  myfwrite32(n, h);
  for (i = 0; i < n; i++) {
    myfwrite8(sw[i].x, h);
    myfwrite8(sw[i].y, h);
    myfwrite8(sw[i].t, h);
    myfwrite8(sw[i].tm, h);
    myfwrite8(sw[i].a, h);
    myfwrite8(sw[i].b, h);
    myfwrite8(sw[i].c, h);
    myfwrite8(sw[i].d, h);
    myfwrite8(sw[i].f, h);
  }
  myfwrite32(sw_secrets, h);
}

static void SW_loadgame (FILE *h) {
  int i, n;
  n = myfread32(h);
  for (i = 0; i < n; i++) {
    sw[i].x = myfread8(h);
    sw[i].y = myfread8(h);
    sw[i].t = myfread8(h);
    sw[i].tm = myfread8(h);
    sw[i].a = myfread8(h);
    sw[i].b = myfread8(h);
    sw[i].c = myfread8(h);
    sw[i].d = myfread8(h);
    sw[i].f = myfread8(h);
  }
  sw_secrets = myfread32(h);
}

static void W_savegame (FILE* h) {
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
    myfwrite8(R_get_swp(i), h);
  }
  myfwrite(fldb, FLDW*FLDH, 1, h);
  myfwrite(fld, FLDW*FLDH, 1, h);
  myfwrite(fldf, FLDW*FLDH, 1, h);
}

static void W_loadgame (FILE* h) {
  int i;
  char s[8];
  sky_type = myfread32(h);
  R_loadsky(sky_type);
  R_begin_load();
  for (i = 1; i < 256; ++i) {
    myfread(s, 8, 1, h);
    if (s[0]) {
      R_load(s);
    }
  }
  R_end_load();
  for (i = 0; i < 256; i++) {
    myfread32(h); // useless
  }
  for (i = 0; i < 256; i++) {
    walf[i] = myfread8(h);
  }
  myfread(fldb, FLDW*FLDH, 1, h);
  myfread(fld, FLDW*FLDH, 1, h);
  myfread(fldf, FLDW*FLDH, 1, h);
}

static void WP_savegame (FILE *h) {
  int i, n;
  for (n = MAXWPN - 1; n >= 0 && wp[n].t == 0; n--) {
    // empty
  }
  n += 1;
  myfwrite32(n, h);
  for (i = 0; i < n; i++) {
    myfwrite32(wp[i].o.x, h);
    myfwrite32(wp[i].o.y, h);
    myfwrite32(wp[i].o.xv, h);
    myfwrite32(wp[i].o.yv, h);
    myfwrite32(wp[i].o.vx, h);
    myfwrite32(wp[i].o.vy, h);
    myfwrite32(wp[i].o.r, h);
    myfwrite32(wp[i].o.h, h);
    myfwrite8(wp[i].t, h);
    myfwrite8(wp[i].s, h);
    myfwrite32(wp[i].own, h);
    myfwrite16(wp[i].target, h);
  }
}

static void WP_loadgame (FILE *h) {
  int i, n;
  n = myfread32(h);
  for (i = 0; i < n; i++) {
    wp[i].o.x = myfread32(h);
    wp[i].o.y = myfread32(h);
    wp[i].o.xv = myfread32(h);
    wp[i].o.yv = myfread32(h);
    wp[i].o.vx = myfread32(h);
    wp[i].o.vy = myfread32(h);
    wp[i].o.r = myfread32(h);
    wp[i].o.h = myfread32(h);
    wp[i].t = myfread8(h);
    wp[i].s = myfread8(h);
    wp[i].own = myfread32(h);
    wp[i].target = myfread16(h);
  }
}

static char *getsavfpname (int n, int ro) {
  static char fn[] = "savgame0.dat";
  fn[7] = n + '0';
#ifndef WIN32
  static char p[100];
  char *e = getenv("HOME");
  strncpy(p, e, 60);
  strcat(p, "/.doom2d-rembo");
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

void F_getsavnames (void) {
  int i;
  FILE *h;
  short ver;
  char *p;
  for (i = 0; i < 7; ++i) {
    p = getsavfpname(i, 1);
    memset(savname[i], 0, 24);
    savok[i] = 0;
    h = fopen(p, "rb");
    if (h != NULL) {
      ver = -1;
      myfread(savname[i], 24, 1, h);
      ver = myfread16(h);
      savname[i][23] = 0;
      savok[i] = (ver == 3) ? 1 : 0;
      fclose(h);
    }
  }
}

void F_savegame (int n, char *s) {
  char *p = getsavfpname(n, 0);
  FILE *h = fopen(p, "wb");
  if (h != NULL) {
    myfwrite(s, 24, 1, h); // slot name
    myfwrite16(3, h); // version
    G_savegame(h);
    W_savegame(h);
    DOT_savegame(h);
    SMK_savegame(h);
    FX_savegame(h);
    IT_savegame(h);
    MN_savegame(h);
    PL_savegame(h);
    SW_savegame(h);
    WP_savegame(h);
    fclose(h);
  }
}

void F_loadgame (int n) {
  short ver;
  char *p = getsavfpname(n, 1);
  FILE *h = fopen(p, "rb");
  if (h != NULL) {
    fseek(h, 24, SEEK_SET); // skip name
    ver = myfread16(h); // version
    if (ver == 3) {
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
    fclose(h);
  }
}
