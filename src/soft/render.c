#include <string.h>
#include <stdarg.h>
#include <stdlib.h> // abs()
#include <assert.h>
#include "glob.h"
#include "render.h"
#include "view.h"
#include "player.h"
#include "switch.h"
#include "vga.h"
#include "menu.h"
#include "misc.h"
#include "dots.h"
#include "items.h"
#include "monster.h"
#include "weapons.h"
#include "smoke.h"
#include "fx.h"
#include "memory.h"
#include "files.h"
#include "error.h"
#include "game.h"
#include "sound.h"
#include "music.h"
#include "system.h"

#include "cp866.h"

// game
static vgaimg *scrnh[3]; // TITLEPIC INTERPIC ENDPIC
static vgaimg *ltn[2][2];
static void *cd_scr;
// smoke
static vgaimg *smk_spr[SMSN];
static vgaimg *smk_fspr[FLSN];
// fx
static vgaimg *fx_spr[15];
static char fx_sprd[15];
// weapons
static vgaimg *wp_spr[49*2];
static char wp_sprd[49*2];
// items
static vgaimg *item_spr[58];
static char item_sprd[58];
// player
static vgaimg *plr_spr[27*2];
static char plr_sprd[27*2];
static vgaimg *plr_wpn[11][6];
// monsters
static vgaimg *pl_spr[2];
static vgaimg *mn_spr[MN_TN][29*2];
static char mn_sprd[MN_TN][29*2];
static vgaimg *mn_fspr[8];
static vgaimg *mn_sgun[2];
// misc
#define MAXAIR 1091
static vgaimg *sth[22], *bfh[160 - '!'], *sfh[160 - '!'], *stone, *stone2, *keys[3];
static int prx = 0, pry = 0;
// menu
static vgaimg *msklh[2], *mbarl, *mbarm, *mbarr, *mbaro, *mslotl, *mslotm, *mslotr;
// low level
static int gammaa = 0;
static char main_pal[256][3];
static char std_pal[256][3];
static byte gamcor[5][64]={
  #include "gamma.dat"
};
// walls
#define ANIT 5
static int WD, HT;
static int w_o, w_x, w_y;
static vgaimg *walp[256];
static int walh[256];
static byte walani[256];
static byte walswp[256];
static int anih[ANIT][5];
static byte anic[ANIT];
static int max_textures;
static byte w_horiz = 1;
static vgaimg *horiz;

static int init_screen_width = 0;
static int init_screen_height = 0;
static byte init_screen_full = 0xFF;
static int init_screen_gammaa = -1;

/* --- misc --- */

static void *Z_getspr (char n[4], int s, int d, char *dir) {
  int h = F_getsprid(n, s, d, dir);
  return V_getvgaimg(h);
}

static vgaimg *Z_get_char_image (vgaimg **img, int ch) {
  ch = cp866_toupper(ch);
  return ch > 32 && ch < 160 ? img[ch - '!'] : NULL;
}

static int Z_get_char_width_generic (vgaimg **img, int off, int ch) {
  vgaimg *p = Z_get_char_image(img, ch);
  return p == NULL ? off : p->w - 1;
}

static int Z_putch_generic (vgaimg **img, int off, int ch) {
  vgaimg *p = Z_get_char_image(img, ch);
  int w = p == NULL ? off : p->w - 1;
  if (p != NULL) {
    V_spr(prx, pry, p);
  }
  prx += w;
  return w;
}

static int Z_get_string_width_generic (vgaimg **img, int off, const char *fmt, va_list ap) {
  int i, w, ww;
  char buf[80];
  vsprintf(buf, fmt, ap);
  for (i = w = ww = 0; buf[i]; ++i) {
    switch (buf[i]) {
      case '\n':
      case '\r':
        ww = max(w, ww);
        w = 0;
        break;
      default:
        w += Z_get_char_width_generic(img, off, (byte)buf[i]);
    }
  }
  return max(w, ww);
}

static int Z_printf_generic (vgaimg **img, int off, const char *fmt, va_list ap) {
  int i, w, ww;
  char buf[80];
  vsprintf(buf, fmt, ap);
  for (i = w = ww = 0; buf[i]; ++i) {
    switch (buf[i]) {
      case '\n':
        pry += off + 1;
      case '\r':
        w = max(w, ww);
        prx = 0;
        w = 0;
        break;
      default:
        w += Z_putch_generic(img, off, (byte)buf[i]);
    }
  }
  return w;
}

static void Z_gotoxy (int x, int y) {
  prx = x;
  pry = y;
}

static int Z_get_big_string_width (const char *fmt, ...) {
  va_list a;
  va_start(a, fmt);
  int w = Z_get_string_width_generic(bfh, 12, fmt, a);
  va_end(a);
  return w;
}

static int Z_printbf (const char *fmt, ...) {
  va_list a;
  va_start(a, fmt);
  int w = Z_printf_generic(bfh, 12, fmt, a);
  va_end(a);
  return w;
}

static int Z_get_small_string_width (const char *fmt, ...) {
  va_list a;
  va_start(a, fmt);
  int w = Z_get_string_width_generic(sfh, 7, fmt, a);
  va_end(a);
  return w;
}

static int Z_printsf (const char *fmt, ...) {
  va_list a;
  va_start(a, fmt);
  int w =Z_printf_generic(sfh, 7, fmt, a);
  va_end(a);
  return w;
}

static void Z_drawspr (int x, int y, void *p, char d) {
  if (d) {
    V_spr2(x - w_x + WD / 2, y - w_y + HT / 2 + 1 + w_o, p);
  } else {
    V_spr(x - w_x + WD / 2, y - w_y + HT / 2 + 1 + w_o, p);
  }
}

static void Z_clrst (void) {
  V_pic(SCRW - 120, w_o, stone);
  int y = ((vgaimg*)stone)->h;
  while (y < HT) {
    V_pic(SCRW - 120, w_o + y, stone2);
    y += ((vgaimg*)stone)->h;
  }
}

static void Z_drawstlives (char n) {
  V_setrect(SCRW - 40, 30, w_o, 40);
  V_spr(SCRW - 35, w_o + 17, sth[n]);
}

static void Z_drawstkeys (byte k) {
  int x, n;
  V_setrect(SCRW - 120, 70, w_o + 77, 23);
  for (k >>= 4, n = 0, x = SCRW - 75; n < 3; ++n, k >>= 1, x += 9) {
    if (k & 1) {
      V_spr(x, w_o + 91, keys[n]);
    }
  }
}

static void Z_drawstair (int a) {
  V_setrect(SCRW - 120, 120, w_o + 49, 2);
  if (a > 0) {
    if (a > MAXAIR) {
      a = MAXAIR;
    }
    a = a * 100 / MAXAIR;
    V_clr(SCRW - 110, a, w_o + 49, 2, 0xC8);
  }
}

static void Z_drawstprcnt (int y, int n) {
  char s[20];
  int l, i, x, c;
  V_setrect(SCRW - 120, 70, y * 19 + 7 + w_o, 19);
  sprintf(s, "%3d%%", n);
  l = strlen(s);
  x = SCRW - 110;
  for (i = 0; i < l; ++i, x += 14) {
    if (s[i] >='0' && s[i] <= '9') {
      c = s[i] - '0';
    } else if (s[i] == '-') {
      c = 10;
    } else if (s[i] == '%') {
      c = 11;
    } else {
      c = -1;
    }
    if (c >= 0) {
      V_spr(x, y * 19 + 7 + w_o, sth[c]);
    }
  }
}

static void Z_drawstnum (int n) {
  char s[20];
  int l, i, x, c;
  V_setrect(SCRW - 50, 50, w_o + 77, 23);
  if (g_dm) {
    sprintf(s, "%d", n);
    l = strlen(s);
    x = (115 - l * 14) + SCRW - 120;
    for (i = 0; i < l; ++i, x += 14) {
      if (s[i] >= '0' && s[i] <= '9') {
        c = s[i] - '0';
      } else if (s[i] == '-') {
        c = 10;
      } else if(s[i] == '%') {
        c = 11;
      } else {
        c =- 1;
      }
      if (c >= 0) {
        V_spr(x, w_o + 77 + 5, sth[c]);
      }
    }
  }
}

static void Z_drawstwpn (int n, int a) {
  char s[20];
  int l, i, x, c;
  i = n;
  V_setrect(SCRW - 120, 120, w_o + 58, 23);
  if (i >= 0) {
    V_spr(SCRW - 88, w_o + 58 + 19, sth[i + 12]);
  }
  if (n >= 2) {
    sprintf(s, "%d", a);
    l = strlen(s);
    x = SCRW - 10 - l * 14;
    for (i = 0; i < l; ++i, x += 14) {
      if (s[i] >= '0' && s[i] <= '9') {
        c = s[i] - '0';
      } else if (s[i] == '-') {
        c = 10;
      } else if (s[i] == '%') {
        c = 11;
      } else {
        c = -1;
      }
      if (c >= 0) {
        V_spr(x, w_o + 58 + 2, sth[c]);
      }
    }
  }
}

static void Z_drawmanspr (int x, int y, void *p, char d, byte color) {
  if (d) {
    V_manspr2(x - w_x + WD / 2, y - w_y + HT / 2 + 1 + w_o, p, color);
  } else {
    V_manspr(x - w_x + WD / 2, y - w_y + HT / 2 + 1 + w_o, p, color);
  }
}

static void Z_drawfld (byte *fld, int bg) {
    byte *p = fld;
    int x, y;
    for (y = 0; y < FLDH; y++) {
        for (x = 0; x < FLDW; x++) {
            int sx = x * CELW - w_x + WD / 2;
            int sy = y * CELH - w_y + HT / 2 + 1 + w_o;
            int id = *p;
            if (id) {
              //intptr_t spc = (intptr_t) walp[id];
              int spc = R_get_special_id(id);
              if (spc >= 0 && spc <= 3) {
                if (!bg) {
                  byte *cmap = clrmap + (spc + 7) * 256;
                  V_remap_rect(sx, sy, CELW, CELH, cmap);
                }
              } else {
                V_pic(sx, sy, walp[id]);
              }
            }
            p++;
        }
    }
}

/* --- menu --- */

static int gm_tm = 0; // ???

#define SCROLLER_MIDDLE 10
#define TEXTFIELD_MIDDLE 2

static void get_entry_size (const menu_t *m, int i, int *w, int *h) {
  assert(m != NULL);
  assert(i >= 0);
  assert(w != NULL);
  assert(h != NULL);
  int x = 0;
  int y = 0;
  int type = 0;
  menu_msg_t msg;
  msg.type = GM_GETENTRY;
  if (GM_send(m, i, &msg)) {
    type = msg.integer.i;
    switch (type) {
      case GM_BUTTON:
      case GM_SCROLLER:
      case GM_TEXTFIELD:
      case GM_TEXTFIELD_BUTTON:
        msg.type = GM_GETCAPTION;
        if (GM_send(m, i, &msg)) {
          x = Z_get_big_string_width("%.*s", msg.string.maxlen, msg.string.s);
        }
        break;
      case GM_SMALL_BUTTON:
        msg.type = GM_GETCAPTION;
        if (GM_send(m, i, &msg)) {
          x = Z_get_small_string_width("%.*s", msg.string.maxlen, msg.string.s);
        }
        break;
      default:
        assert(0);
    }
    switch (type) {
      case GM_BUTTON:
        msg.type = GM_GETSTR;
        if (GM_send(m, i, &msg)) {
          x += Z_get_big_string_width("%.*s", msg.string.maxlen, msg.string.s);
        }
        y = 16;
        break;
      case GM_SMALL_BUTTON:
        msg.type = GM_GETSTR;
        if (GM_send(m, i, &msg)) {
          x += Z_get_big_string_width("%.*s", msg.string.maxlen, msg.string.s);
        }
        y = 12;
        break;
      case GM_SCROLLER:
        x += (SCROLLER_MIDDLE + 2) * 8;
        y = 16;
        break;
      case GM_TEXTFIELD:
      case GM_TEXTFIELD_BUTTON:
        msg.type = GM_GETSTR;
        if (GM_send(m, i, &msg)) {
          x += (msg.string.maxlen + 2) * 8;
        } else {
          x += (TEXTFIELD_MIDDLE + 2) * 8;
        }
        y = 16;
        break;
      default:
        assert(0);
    }
  }
  *w = x;
  *h = y;
}

static void get_menu_size (const menu_t *m, int *w, int *h) {
  assert(m != NULL);
  assert(w != NULL);
  assert(h != NULL);
  int i, n, x, y, xx, yy, type;
  menu_msg_t msg;
  msg.type = GM_QUERY;
  if (GM_send_this(m, &msg)) {
    n = msg.integer.b;
    type = msg.integer.s;
    x = 0;
    y = 0;
    msg.type = GM_GETTITLE;
    if (GM_send_this(m, &msg)) {
      switch (type) {
        case GM_BIG: x = Z_get_big_string_width("%.*s", msg.string.maxlen, msg.string.s); break;
        case GM_SMALL: x = Z_get_small_string_width("%.*s", msg.string.maxlen, msg.string.s); break;
        default: assert(0);
      }
    }
    for (i = 0; i < n; i++) {
      get_entry_size(m, i, &xx, &yy);
      x = max(x, xx);
      y += yy;
    }
    *w = x;
    *h = y;
  } else {
    *w = 0;
    *h = 0;
  }
}

static int GM_draw (void) {
  int i, j, n, x, y, xoff, yoff, cur, w, type, recv;
  const menu_t *m = GM_get();
  menu_msg_t msg;
  if (m != NULL) {
    get_menu_size(m, &x, &y);
    x = SCRW / 2 - x / 2;
    y = SCRH / 2 - y / 2;
    // --- title ---
    msg.type = GM_QUERY;
    if (GM_send_this(m, &msg)) {
      cur = msg.integer.i;
      n = msg.integer.a;
      type = msg.integer.s;
      msg.type = GM_GETTITLE;
      if (GM_send_this(m, &msg)) {
        Z_gotoxy(x, y - 10);
        switch (type) {
          case GM_SMALL: yoff = 8; Z_printsf("%.*s", msg.string.maxlen, msg.string.s); break;
          case GM_BIG: yoff = 20; Z_printbf("%.*s", msg.string.maxlen, msg.string.s); break;
          default: assert(0);
        }
      } else {
        yoff = 0;
      }
      for (i = 0; i < n; i++) {
        msg.type = GM_GETENTRY;
        if (GM_send(m, i, &msg)) {
          type = msg.integer.i;
          if (i == cur) {
            if (type == GM_SMALL_BUTTON) {
              Z_gotoxy(x - 8, y + yoff);
              Z_printsf(">");
            } else {
              V_spr(x - 25, y + yoff - 8, msklh[(gm_tm / 6) & 1]);
            }
          }
          msg.type = GM_GETCAPTION;
          if (GM_send(m, i, &msg)) {
            Z_gotoxy(x, y + yoff);
            if (type == GM_SMALL_BUTTON) {
              xoff = Z_printsf("%.*s", msg.string.maxlen, msg.string.s);
            } else {
              xoff = Z_printbf("%.*s", msg.string.maxlen, msg.string.s);
            }
          } else {
            xoff = 0;
          }
          switch (type) {
            case GM_BUTTON:
            case GM_SMALL_BUTTON:
              msg.type = GM_GETSTR;
              if (GM_send(m, i, &msg)) {
                Z_gotoxy(x + xoff, y + yoff);
                if (type == GM_SMALL_BUTTON) {
                  Z_printsf("%.*s", msg.string.maxlen, msg.string.s);
                } else {
                  Z_printbf("%.*s", msg.string.maxlen, msg.string.s);
                }
              }
              yoff += type == GM_BUTTON ? 16 : 12;
              break;
            case GM_TEXTFIELD:
            case GM_TEXTFIELD_BUTTON:
              yoff += 9;
              msg.type = GM_GETSTR;
              recv = GM_send(m, i, &msg);
              w = recv ? msg.string.maxlen : TEXTFIELD_MIDDLE;
              V_spr(x + xoff, y + yoff, mslotl);
              for (j = 1; j <= w; j++) {
                V_spr(x + xoff + j * 8, y + yoff, mslotm);
              }
              V_spr(x + xoff + j * 8, y + yoff, mslotr);
              Z_gotoxy(x + xoff + 4, y + yoff - 7);
              if (input && i == cur) {
                Z_printsf("%.*s_", imax, ibuf);
              } else if (recv) {
                Z_printsf("%.*s", msg.string.maxlen, msg.string.s);
              }
              yoff += 7;
              break;
            case GM_SCROLLER:
              V_spr(x + xoff, y + yoff, mbarl);
              for (j = 1; j < SCROLLER_MIDDLE; j++) {
                V_spr(x + xoff + j * 8, y + yoff, mbarm);
              }
              V_spr(x + xoff + j * 8, y + yoff, mbarr);
              msg.type = GM_GETINT;
              if (GM_send(m, i, &msg)) {
                int lev = (msg.integer.i - msg.integer.a) * ((SCROLLER_MIDDLE - 2) * 8) / msg.integer.b;
                V_spr(x + xoff + lev + 8, y + yoff, mbaro);
              }
              yoff += 16;
              break;
            default:
              assert(0);
          }
        }
      }
    }
  }
  return m != NULL;
}

/* --- dots --- */

static void DOT_draw (void) {
  int i;
  for (i = 0; i < MAXDOT; i++) {
    if (dot[i].t) {
      V_dot(dot[i].o.x - w_x + WD / 2, dot[i].o.y - w_y + HT / 2 + 1 + w_o, dot[i].c);
    }
  }
}

/* --- items --- */

static void IT_draw (void) {
  int i, s;
  for (i = 0; i < MAXITEM; ++i) {
    s = -1;
    if (it[i].t && it[i].s >= 0) {
      switch(it[i].t & 0x7FFF) {
        case I_ARM1:
          s = it[i].s / 9 + 18;
          break;
        case I_ARM2:
          s = it[i].s / 9 + 20;
          break;
        case I_MEGA:
          s = it[i].s / 2 + 22;
          break;
        case I_INVL:
          s = it[i].s / 2 + 26;
          break;
        case I_SUPER:
        case I_RTORCH:
        case I_GTORCH:
        case I_BTORCH:
          s = it[i].s / 2 + (it[i].t - I_SUPER) * 4 + 35;
          break;
        case I_GOR1: case I_FCAN:
          s = it[i].s / 2 + (it[i].t - I_GOR1) * 3 + 51;
          break;
        case I_AQUA:
          s = 30;
          break;
        case I_SUIT:
          s = 34;
          break;
        case I_KEYR:
        case I_KEYG:
        case I_KEYB:
          s = (it[i].t & 0x7FFF) - I_KEYR + 31;
          break;
        case I_GUN2:
          s = 57;
          break;
        default:
          s = (it[i].t & 0x7FFF) - 1;
      }
    }
    if (s >= 0) {
      Z_drawspr(it[i].o.x, it[i].o.y, item_spr[s], item_sprd[s]);
    }
  }
}

/* --- player --- */

static int standspr (player_t *p) {
  if (p->f & PLF_UP) {
    return 'X';
  } else if (p->f & PLF_DOWN) {
    return 'Z';
  } else {
    return 'E';
  }
}

static int wpnspr (player_t *p) {
  if (p->f & PLF_UP) {
    return 'C';
  } else if(p->f & PLF_DOWN) {
    return 'E';
  } else {
    return 'A';
  }
}

static void PL_draw (player_t *p) {
  enum {STAND, GO, DIE, SLOP, DEAD, MESS, OUT, FALL}; // copypasted from player.c!
  static int wytab[] = {-1, -2, -1, 0};
  int s = 'A';
  int w = 0;
  int wx = 0;
  int wy = 0;
  switch (p->st) {
    case STAND:
      if (p->f & PLF_FIRE) {
        s = standspr(p) + 1;
        w = wpnspr(p) + 1;
      } else if (p->pain) {
        s = 'G';
        w = 'A';
        wx = p->d ? 2 : -2;
        wy = 1;
      } else {
        s = standspr(p);
        w = wpnspr(p);
      }
      break;
    case DEAD:
      s = 'N';
      break;
    case MESS:
      s = 'W';
      break;
    case GO:
      if (p->pain) {
        s = 'G';
        w = 'A';
        wx = p->d ? 2 : -2;
        wy = 1;
      } else {
        s = plr_goanim[p->s / 8];
        w = (p->f & PLF_FIRE) ? 'B' : 'A';
        wx = p->d ? 2 : -2;
        wy = 1 + wytab[s - 'A'];
      }
      break;
    case DIE:
      s = plr_dieanim[p->s];
      break;
    case SLOP:
      s = plr_slopanim[p->s];
      break;
    case OUT:
      s = 0;
      break;
  }
  if (p->wpn == 0) {
    w = 0;
  }
  if (w) {
    Z_drawspr(p->o.x + wx, p->o.y + wy, plr_wpn[p->wpn][w - 'A'], p->d);
  }
  if (s) {
    Z_drawmanspr(p->o.x, p->o.y, plr_spr[(s - 'A') * 2 + p->d], plr_sprd[(s - 'A') * 2 + p->d], p->color);
  }
}

static void PL_drawst (player_t *p) {
  int i;
  V_setrect(WD, 120, w_o, HT);
  Z_clrst();
  if (p->drawst & PL_DRAWAIR) {
      if (p->air < PL_AIR) {
        Z_drawstair(p->air);
      }
  }
  if (p->drawst & PL_DRAWLIFE) {
    Z_drawstprcnt(0, p->life);
  }
  if (p->drawst & PL_DRAWARMOR) {
    Z_drawstprcnt(1, p->armor);
  }
  if (p->drawst & PL_DRAWWPN) {
    switch(p->wpn) {
      case 2:
      case 5:
        i = p->ammo;
        break;
      case 3:
      case 4:
      case 9:
        i = p->shel;
        break;
      case 6:
        i = p->rock;
        break;
      case 10:
        i = p->fuel;
        break;
      case 7:
      case 8:
        i = p->cell;
        break;
    }
    Z_drawstwpn(p->wpn, i);
  }
  if (p->drawst & PL_DRAWFRAG) {
    Z_drawstnum(p->frag);
  }
  if (p->drawst & PL_DRAWKEYS) {
    Z_drawstkeys(p->keys);
  }
  if (!_2pl) {
    if (p->drawst & PL_DRAWLIVES) {
      Z_drawstlives(p->lives);
    }
  }
}

/* --- monster --- */

#define MANCOLOR 0xD0

static void MN_draw (void) {
  enum {SLEEP, GO, RUN, CLIMB, DIE, DEAD, ATTACK, SHOOT, PAIN, WAIT, REVIVE, RUNOUT}; // copypasted from monster.c!
  int i;
  for (i = 0; i < MAXMN; i++) {
    if (mn[i].t) {
      if (mn[i].t >= MN_PL_DEAD) {
        Z_drawmanspr(mn[i].o.x, mn[i].o.y, pl_spr[mn[i].t - MN_PL_DEAD], 0, mn[i].d);
        continue;
      }
      if ((mn[i].t != MN_SOUL && mn[i].t != MN_PAIN) || mn[i].st != DEAD) {
        if (mn[i].t != MN_MAN) {
          Z_drawspr(mn[i].o.x, mn[i].o.y, mn_spr[mn[i].t - 1][(mn[i].ap[mn[i].ac] - 'A') * 2 + mn[i].d], mn_sprd[mn[i].t - 1][(mn[i].ap[mn[i].ac] - 'A') * 2 + mn[i].d]);
        } else {
          if (mn[i].ap[mn[i].ac] == 'E' || mn[i].ap[mn[i].ac] == 'F') {
            Z_drawspr(mn[i].o.x, mn[i].o.y, mn_sgun[mn[i].ap[mn[i].ac] - 'E'], mn[i].d);
          }
          Z_drawmanspr(mn[i].o.x, mn[i].o.y, mn_spr[mn[i].t - 1][(mn[i].ap[mn[i].ac] - 'A') * 2 + mn[i].d], mn_sprd[mn[i].t - 1][(mn[i].ap[mn[i].ac] - 'A') * 2 + mn[i].d], MANCOLOR);
        }
      }
      if (mn[i].t == MN_VILE && mn[i].st == SHOOT) {
        Z_drawspr(mn[i].tx, mn[i].ty, mn_fspr[mn[i].ac / 3], 0);
      }
    }
  }
}

/* --- weapon --- */

static void WP_draw (void) {
  enum {NONE, ROCKET, PLASMA, APLASMA, BALL1, BALL2, BALL7, BFGBALL, BFGHIT, MANF, REVF, FIRE}; // copypasted from weapons.c!
  int i, s, d, x, y;
  for (i = 0; i < MAXWPN; ++i) {
    s = -1;
    d = 0;
    switch (wp[i].t) {
      case NONE:
      default:
        break;
      case REVF:
      case ROCKET:
        d = wp[i].s;
        if (d < 2) {
          d = wp[i].o.xv > 0 ? 1 : 0;
          x = abs(wp[i].o.xv);
          y = wp[i].o.yv;
          s = 0;
          if (y < 0) {
            if (-y >= x) {
              s = 30;
            }
          } else if (y > 0) {
            if (y >= x / 2) {
              s = 31;
            }
          }
        } else {
          s = (d - 2) / 2 + 1;
          d = 0;
        }
        break;
      case MANF:
        s=wp[i].s;
        if (s >= 2) {
          s /= 2;
          break;
        }
      case PLASMA:
      case APLASMA:
      case BALL1:
      case BALL7:
      case BALL2:
        s = wp[i].s;
        if (s >= 2) {
          s = s / 2 + 1;
        }
        switch (wp[i].t) {
          case PLASMA:
            s += 4;
            break;
          case APLASMA:
            s += 11;
            break;
          case BALL1:
            s += 32;
            break;
          case BALL2:
            s += 42;
            break;
          case BALL7:
            s += 37;
            d = wp[i].o.xv >= 0 ? 1 : 0;
            break;
          case MANF:
            s += 47;
            d= wp[i].o.xv>=0 ? 1 : 0;
            break;
        }
        break;
      case BFGBALL:
        s = wp[i].s;
        if (s >= 2) {
          s = s / 2 + 1;
        }
        s += 18;
        break;
      case BFGHIT:
        s = wp[i].s / 2 + 26;
        break;
    }
    if (s >= 0) {
      Z_drawspr(wp[i].o.x, wp[i].o.y, wp_spr[s * 2 + d], wp_sprd[s * 2 + d]);
    }
  }
}

/* --- smoke --- */

static void SMK_draw (void) {
  int i, s;
  for (i = 0; i < MAXSMOK; ++i) {
    if (sm[i].t) {
      switch (sm[i].s) {
        case 0:
          s = sm[i].t;
          if (s >= (SMSN - 1) * 3) {
            s = 0;
          } else {
            s = SMSN - 1 - s / 3;
          }
          V_sprf((sm[i].x >> 8) - w_x + WD / 2, (sm[i].y >> 8) - w_y + HT / 2 + 1 + w_o, smk_spr[s], &smoke_sprf);
          break;
        case 1:
          s = sm[i].t;
          if (s >= FLSN - 1) {
            s = 0;
          } else {
            s = FLSN - 1 - s;
          }
          V_sprf((sm[i].x >> 8) - w_x + WD / 2, (sm[i].y >> 8) - w_y + HT / 2 + 1 + w_o, smk_fspr[s], &flame_sprf);
          break;
      }
    }
  }
}

/* --- fx --- */

static void FX_draw (void) {
  enum {NONE, TFOG, IFOG, BUBL}; // copypasted from fx.c
  int i, s;
  for (i = 0; i < MAXFX; ++i) {
    s = -1;
    switch (fx[i].t) {
      case TFOG:
        s = fx[i].s / 2;
        break;
      case IFOG:
        s = fx[i].s / 2 + 10;
        break;
      case BUBL:
        V_dot((fx[i].x >> 8) - w_x + WD / 2, (fx[i].y >> 8) - w_y + HT / 2 + 1 + w_o, 0xC0 + fx[i].s);
        continue;
    }
    if (s >= 0) {
      Z_drawspr(fx[i].x, fx[i].y, fx_spr[s], fx_sprd[s]);
    }
  }
}

/* --- view --- */

static void W_adjust (void) {
  int MAXX = FLDW * CELW - WD / 2;
  int MAXY = FLDH * CELH - HT / 2;
  if (w_x < WD / 2) w_x = WD / 2;
  if (w_y < HT / 2) w_y = HT / 2;
  if (w_x > MAXX) w_x = MAXX;
  if (w_y > MAXY) w_y = MAXY;
}

static void W_draw(void) {
  W_adjust();
  V_setrect(0, WD, w_o + 1, HT);
  if (w_horiz) {
    vgaimg *img = (vgaimg*)horiz;
    int x = 0;
    int d = 0;
    do {
      int y = w_o;
      d &= ~2;
      do {
        V_rotspr(x, y, img, d);
        y += img->h;
        d ^= 2;
      } while (y < HT + w_o);
      x += img->w;
      d ^= 1;
    } while (x < WD);
    if (sky_type == 2) {
      if (lt_time < 0) {
        if (!lt_side) {
          V_spr(0, w_o + lt_ypos, ltn[lt_type][lt_time < -5 ? 0 : 1]);
        } else {
          V_spr2(WD - 1, w_o + lt_ypos, ltn[lt_type][lt_time < -5 ? 0 : 1]);
        }
      }
    }
  } else {
    V_clr(0, WD, w_o + 1, HT, 0x97);
  }
  Z_drawfld((byte*)fldb, 1);
  DOT_draw();
  IT_draw();
  PL_draw(&pl1);
  if (_2pl) {
    PL_draw(&pl2);
  }
  MN_draw();
  WP_draw();
  SMK_draw();
  FX_draw();
  Z_drawfld((byte*)fldf, 0);
  if (sky_type == 2) {
    if (lt_time == -4 || lt_time == -2) {
      V_remap_rect(0, WD, w_o + 1, HT, clrmap + 256 * 11);
    }
  }
}

/* --- game --- */

#define PL_FLASH 90

static void drawview (player_t *p) {
  if (p->looky < -SCRH / 4) {
    p->looky = -SCRH / 4;
  } else if (p->looky > SCRH / 4) {
    p->looky = SCRH / 4;
  }
  w_x = p->o.x;
  w_y = p->o.y - 12 + p->looky;
  W_draw();
  PL_drawst(p);
}

static int get_pu_st (int t) {
  if (t >= PL_FLASH) {
    return 1;
  } else if((t / 9) & 1) {
    return 0;
  } else {
    return 1;
  }
}

static void pl_info (player_t *p, int y) {
  dword t = p->kills * 10920 / g_time;
  Z_gotoxy(25, y); Z_printbf("KILLS");
  Z_gotoxy(25, y + 15); Z_printbf("KPM");
  Z_gotoxy(25, y + 30); Z_printbf("SECRETS %u / %u", p->secrets, sw_secrets);
  Z_gotoxy(255, y); Z_printbf("%u", p->kills);
  Z_gotoxy(255, y + 15); Z_printbf("%u.%u", t / 10, t % 10);
}

static void W_act (void) {
  int i, a;
  if (g_time % 3 == 0) {
    for (i = 1; i < 256; i++) {
      a = walani[i];
      if (a != 0) {
        anic[a]++;
        if (anih[a][anic[a]] == -1) {
          anic[a] = 0;
        }
        walp[i] = V_getvgaimg(anih[a][anic[a]]);
      }
    }
  }
}

void R_draw (void) {
  int h;
  word hr, mn, sc;
  W_act();
  switch (g_st) {
    case GS_ENDANIM:
    case GS_END2ANIM:
    case GS_DARKEN:
    case GS_BVIDEO:
    case GS_EVIDEO:
    case GS_END3ANIM:
      return;
    case GS_TITLE:
      V_center(1);
      V_pic(0, 0, scrnh[0]);
      V_center(0);
      break;
    case GS_ENDSCR:
      V_center(1);
      V_clr(0, SCRW, 0, SCRH, 0);
      V_pic(0, 0, scrnh[2]);
      V_center(0);
      break;
    case GS_INTER:
      V_center(1);
      V_clr(0, SCRW, 0, SCRH, 0);
      V_pic(0, 0, scrnh[1]);
      Z_gotoxy(60, 20);
      Z_printbf("LEVEL COMPLETE");
      Z_calc_time(g_time, &hr, &mn, &sc);
      Z_gotoxy(115, 40);
      Z_printbf("TIME %u:%02u:%02u", hr, mn, sc);
      h = 60;
      if (_2pl) {
        Z_gotoxy(80, h);
        Z_printbf("PLAYER ONE");
        Z_gotoxy(80, h + 70);
        Z_printbf("PLAYER TWO");
        h += SCRH / 10;
      }
      pl_info(&pl1, h);
      if (_2pl) {
        pl_info(&pl2, h + 70);
      }
      V_center(0);
      break;
  }
  V_center(0);
  if (g_st == GS_GAME) {
    if (_2pl) {
      w_o = 0;
      WD = SCRW - 120;
      HT = SCRH / 2 - 2;
      drawview(&pl1);
      w_o = SCRH / 2;
      WD = SCRW - 120;
      HT = SCRH / 2 - 2;
      drawview(&pl2);
    } else{
      w_o = 0;
      WD = SCRW - 120;
      HT = SCRH - 2;
      drawview(&pl1);
    }
    if (pl1.invl) {
      h = get_pu_st(pl1.invl) * 6;
    } else if (pl1.pain < 15) {
      h = 0;
    } else if (pl1.pain < 35) {
      h = 1;
    } else if (pl1.pain < 55) {
      h = 2;
    } else if (pl1.pain < 75) {
      h=3;
    } else if (pl1.pain < 95) {
      h=4;
    } else {
      h = 5;
    }
    if (h != 0) {
      V_maptoscr(0, SCRW - 120, 1, _2pl ? SCRH / 2 - 2 : SCRH - 2, clrmap + h * 256);
    }
    if (_2pl) {
      if (pl2.invl) {
        h = get_pu_st(pl2.invl) * 6;
      } else if (pl2.pain < 15) {
        h = 0;
      } else if (pl2.pain < 35) {
        h = 1;
      } else if (pl2.pain < 55) {
        h = 2;
      } else if (pl2.pain < 75) {
        h = 3;
      } else if (pl2.pain < 95) {
        h = 4;
      } else {
        h = 5;
      }
      if (h) {
        V_maptoscr(0, SCRW - 120, SCRH / 2 + 1, SCRH / 2 - 2, clrmap + h * 256);
      }
    }
  }
  V_center(0);
  V_setrect(0, SCRW, 0, SCRH);
  GM_draw();
  V_copytoscr(0, SCRW, 0, SCRH);
}

void R_alloc (void) {
  int i, j, n;
  char s[10];
  logo("R_alloc: загрузка графики\n");
  // game
  scrnh[0] = V_loadvgaimg("TITLEPIC");
  scrnh[1] = V_loadvgaimg("INTERPIC");
  scrnh[2] = V_loadvgaimg("ENDPIC");
  cd_scr = M_lock(F_getresid("CD1PIC"));
  for (i = 0; i < 2; ++i) {
    sprintf(s, "LTN%c", i + '1');
    for (j = 0; j < 2; ++j) {
      ltn[i][j] = Z_getspr(s, j, 0, NULL);
    }
  }
  // smoke
  for (i = 0; i < SMSN; ++i) {
    smk_spr[i] = Z_getspr("SMOK", i, 0, NULL);
  }
  for (i = 0; i < FLSN; ++i) {
    smk_fspr[i] = Z_getspr("FLAM", i, 0, NULL);
  }
  // fx
  for (i = 0; i < 10; ++i) {
    fx_spr[i] = Z_getspr("TFOG", i, 0, fx_sprd + i);
  }
  for (; i < 15; ++i) {
    fx_spr[i] = Z_getspr("IFOG", i - 10, 0, fx_sprd + i);
  }
  // weapons
  for (i = 0; i < 4; ++i) {
    wp_spr[i * 2] = Z_getspr("MISL", i, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("MISL", i, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 6; ++i) {
    wp_spr[i * 2] = Z_getspr("PLSS", i - 4, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("PLSS", i - 4, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 11; ++i) {
    wp_spr[i * 2] = Z_getspr("PLSE", i - 6, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("PLSE", i - 6, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 13; ++i) {
    wp_spr[i * 2] = Z_getspr("APLS", i - 11, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("APLS", i - 11, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 18; ++i) {
    wp_spr[i * 2] = Z_getspr("APBX", i - 13, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("APBX", i - 13, 2, wp_sprd + i * 2 + 1);
  }
  for(; i < 20; ++i) {
    wp_spr[i * 2] = Z_getspr("BFS1", i - 18, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BFS1", i - 18, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 26; ++i) {
    wp_spr[i * 2] = Z_getspr("BFE1", i - 20, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BFE1", i - 20, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 30; ++i) {
    wp_spr[i * 2] = Z_getspr("BFE2", i - 26, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BFE2", i - 26, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 32; ++i) {
    wp_spr[i * 2] = Z_getspr("MISL", i - 30 + 4, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("MISL", i - 30 + 4, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 37; ++i) {
    wp_spr[i * 2] = Z_getspr("BAL1", i - 32, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BAL1", i - 32, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 42; ++i) {
    wp_spr[i * 2] = Z_getspr("BAL7", i - 37, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BAL7", i - 37, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 47; ++i) {
    wp_spr[i * 2] = Z_getspr("BAL2", i - 42, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BAL2", i - 42, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 49; ++i) {
    wp_spr[i * 2] = Z_getspr("MANF", i - 47, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("MANF", i - 47, 2, wp_sprd + i * 2 + 1);
  }
  // items
  static char snm[18][4] = {
    "CLIP", "SHEL", "ROCK", "CELL", "AMMO", "SBOX", "BROK", "CELP",
    "STIM", "MEDI", "BPAK",
    "CSAW", "SHOT", "SGN2", "MGUN", "LAUN", "PLAS", "BFUG"
  };
  static char n4[4][4] = {
    "SOUL", "SMRT", "SMGT", "SMBT"
  };
  static char n3[2][4] = {
    "GOR1", "FCAN"
  };
  for (i = 0; i < 18; ++i) {
    item_spr[i] = Z_getspr(snm[i], 0, 0, item_sprd + i);
  }
  for (; i < 20; ++i) {
    item_spr[i] = Z_getspr("ARM1", i - 18, 0, item_sprd + i);
    item_spr[i + 2] = Z_getspr("ARM2", i - 18, 0, item_sprd + i);
  }
  i+=2;
  for (; i < 26; ++i) {
    item_spr[i] = Z_getspr("MEGA", i - 22, 0, item_sprd + i);
  }
  for (; i < 30; ++i) {
    item_spr[i] = Z_getspr("PINV", i - 26, 0, item_sprd + i);
  }
  item_spr[30] = Z_getspr("AQUA", 0, 0, item_sprd + 30);
  item_spr[31] = Z_getspr("KEYR", 0, 0, item_sprd + 31);
  item_spr[32] = Z_getspr("KEYG", 0, 0, item_sprd + 32);
  item_spr[33] = Z_getspr("KEYB", 0, 0, item_sprd + 33);
  item_spr[34] = Z_getspr("SUIT", 0, 0, item_sprd + 34);
  for (n = 35, j = 0; j < 4; ++j) {
    for (i = 0; i < 4; ++i, ++n) {
      item_spr[n] = Z_getspr(n4[j], i, 0, item_sprd + n);
    }
  }
  for (j = 0; j < 2; ++j) {
    for (i = 0; i < 3; ++i, ++n) {
      item_spr[n] = Z_getspr(n3[j], i, 0, item_sprd + n);
    }
  }
  item_spr[57] = Z_getspr("GUN2", 0, 0, item_sprd + 57);
  // player
  for (i = 0; i < 27; ++i) {
    plr_spr[i * 2] = Z_getspr("PLAY", i, 1, plr_sprd + i * 2);
    plr_spr[i * 2 + 1] = Z_getspr("PLAY", i, 2, plr_sprd + i * 2 + 1);
  }
  strncpy(s, "PWPx", 4);
  for (i = 1; i < 11; ++i) {
    s[3] = (i < 10 ? '0' : 'A' - 10) + i;
    for (j = 0; j < 6; ++j) {
      plr_wpn[i][j] = Z_getspr(s, j, 1, NULL);
    }
  }
  // monsters
  static char msn[MN_TN][4] = {
    "SARG", "TROO", "POSS", "SPOS", "CYBR", "CPOS", "BOSS", "BOS2", "HEAD", "SKUL",
    "PAIN", "SPID", "BSPI", "FATT", "SKEL", "VILE", "FISH", "BAR1", "ROBO", "PLAY"
  };
  static int mms[MN_TN] = {
    14*2, 21*2, 21*2, 21*2, 16*2, 20*2, 15*2, 15*2, 12*2, 11*2, 13*2, 19*2, 16*2,
    20*2, 17*2, 29*2, 6*2, 2*2, 17*2, 23*2
  };
  mn_sgun[0] = Z_getspr("PWP4", 0, 1, NULL);
  mn_sgun[1] = Z_getspr("PWP4", 1, 1, NULL);
  for (j = 0; j < MN_TN; ++j) {
    for (i = 0; i < mms[j]; ++i) {
      mn_spr[j][i] = Z_getspr(msn[j], i / 2, (i & 1) + 1, &mn_sprd[j][i]);
    }
    if (j == MN_BARREL - 1) {
      for (i = 4; i < 14; ++i) {
        mn_spr[j][i] = Z_getspr("BEXP", i / 2 - 2, (i & 1) + 1, &mn_sprd[j][i]);
      }
    }
  }
  for (i = 0; i < 8; ++i) {
    mn_fspr[i] = Z_getspr("FIRE", i, 0, NULL);
  }
  pl_spr[0] = Z_getspr("PLAY", 'N' - 'A', 0, NULL);
  pl_spr[1] = Z_getspr("PLAY", 'W' - 'A', 0, NULL);
  // misc
  static char mnm[22][8]={
	"STTNUM0","STTNUM1","STTNUM2","STTNUM3","STTNUM4",
	"STTNUM5","STTNUM6","STTNUM7","STTNUM8","STTNUM9",
	"STTMINUS","STTPRCNT",
	"FISTA0","CSAWA0","PISTA0","SHOTA0","SGN2A0","MGUNA0","LAUNA0",
	"PLASA0","BFUGA0","GUN2A0"
  };
  stone=V_loadvgaimg("STONE");
  stone2=V_loadvgaimg("STONE2");
  keys[0]=V_loadvgaimg("KEYRA0");
  keys[1]=V_loadvgaimg("KEYGA0");
  keys[2]=V_loadvgaimg("KEYBA0");
  for (i = 0; i < 22; ++i) {
    sth[i] = V_loadvgaimg(mnm[i]);
  }
  strcpy(s, "STBF_*");
  for (i = '!'; i < 160; ++i) {
    s[5] = i;
    bfh[i - '!'] = V_getvgaimg(F_findres(s));
  }
  for (i = '!'; i < 160; ++i) {
    sprintf(s, "STCFN%03d", i);
    sfh[i - '!'] = V_getvgaimg(F_findres(s));
  }
  strcpy(s, "WINUM*");
  for (i = '0'; i <= '9'; ++i) {
    s[5] = i;
    bfh[i - '!'] = V_loadvgaimg(s);
  }
  bfh[':' - '!'] = V_loadvgaimg("WICOLON");
  // menu
  msklh[0] = V_loadvgaimg("M_SKULL1");
  msklh[1] = V_loadvgaimg("M_SKULL2");
  mbarl = V_loadvgaimg("M_THERML");
  mbarm = V_loadvgaimg("M_THERMM");
  mbarr = V_loadvgaimg("M_THERMR");
  mbaro = V_loadvgaimg("M_THERMO");
  mslotl = V_loadvgaimg("M_LSLEFT");
  mslotm = V_loadvgaimg("M_LSCNTR");
  mslotr = V_loadvgaimg("M_LSRGHT");
  // walls
  static char *anm[ANIT - 1][5] = {
    {"WALL22_1", "WALL23_1", "WALL23_2", NULL,    NULL},
    {"WALL58_1", "WALL58_2", "WALL58_3", NULL,    NULL},
    {"W73A_1",   "W73A_2",   NULL,       NULL,    NULL},
    {"RP2_1",    "RP2_2",    "RP2_3",    "RP2_4", NULL}
  };
  for (i = 1; i < ANIT; i++) {
    for (j = 0; anm[i - 1][j]; j++) {
      anih[i][j] = F_getresid(anm[i - 1][j]);
    }
    for(; j < 5; j++) {
      anih[i][j] = -1;
    }
  }
}

void R_get_name (int n, char s[8]) {
  if (walh[n] == -1) {
    memset(s, 0, 8);
  } else if (walh[n] == -2) {
    memcpy(s, "_WATER_", 8);
    s[7] = (char)((intptr_t)walp[n] - 1 + '0');
  } else {
    F_getresname(s, walh[n] & 0x7FFF);
  }
}

static short getani (char n[8]) {
  if (strncasecmp(n, "WALL22_1", 8) == 0) {
    return 1;
  } else if (strncasecmp(n, "WALL58_1", 8) == 0) {
    return 2;
  } else if (strncasecmp(n, "W73A_1", 8) == 0) {
    return 3;
  } else if (strncasecmp(n, "RP2_1", 8) == 0) {
    return 4;
  } else {
    return 0;
  }
}

int R_get_special_id (int n) {
  assert(n >= 0 && n < 256);
  intptr_t x = (intptr_t)walp[n];
  return x >= 0 && x <= 3 ? x : -1;
}

void R_begin_load (void) {
  int i;
  for (i = 0; i < 256; i++) {
    if (walp[i] != NULL && walh[i] >= 0) {
      M_unlock(walp[i]);
    }
    walh[i] = -1;
    walp[i] = NULL;
    walswp[i] = i;
    walani[i] = 0;
  }
  memset(anic, 0, sizeof(anic));
  max_textures = 1;
}

void R_load (char s[8]) {
  assert(max_textures < 256);
  if (!s[0]) {
    walh[max_textures] = -1;
    walp[max_textures] = NULL;
  } else {
    if (strncasecmp(s, "_WATER_", 7) == 0) {
      walh[max_textures] = -2;
      walp[max_textures] = (void*)((intptr_t)s[7] - '0' + 1);
    } else {
      walh[max_textures] = F_getresid(s);
      walp[max_textures] = V_getvgaimg(walh[max_textures]);
      if (s[0] == 'S' && s[1] == 'W' && s[4] == '_') {
        walswp[max_textures] = 0;
      }
    }
    walani[max_textures] = getani(s);
  }
  max_textures++;
}

void R_end_load (void) {
  int i, j, k, g;
  char s[8];
  j = max_textures;
  for (i = 1; i < 256 && j < 256; i++) {
    if (walswp[i] == 0) {
      R_get_name(i, s);
      s[5] ^= 1;
      g = F_getresid(s);
      k = 1;
      while (k < 256 && walh[k] != g) {
        k += 1;
      }
      if (k >= 256) {
        k = j;
        j += 1;
        walh[k] = g;
        walp[k] = V_getvgaimg(g);
        walf[k] = walf[i];
      }
      walswp[i] = k;
      walswp[k] = i;
    }
  }
}

void R_loadsky (int sky) {
  char s[6];
  strcpy(s, "RSKY1");
  s[4] = '0' + sky;
  M_unlock(horiz);
  horiz = V_loadvgaimg(s);
}

void R_setgamma(int g) {
  int t;
  g = g < 0 ? 0 : (g > 4 ? 4 : g);
  gammaa = g;
  for (t = 0; t < 256; ++t) {
    std_pal[t][0]=gamcor[gammaa][main_pal[t][0]];
    std_pal[t][1]=gamcor[gammaa][main_pal[t][1]];
    std_pal[t][2]=gamcor[gammaa][main_pal[t][2]];
  }
  Y_set_vga_palette(std_pal);
}

int R_getgamma (void) {
  return gammaa;
}

void R_set_videomode (int w, int h, int fullscreen) {
  assert(w > 0);
  assert(h > 0);
  int was = Y_videomode_setted();
  int res = Y_set_videomode_software(w, h, fullscreen);
  if (res == 0) {
    if (was == 0) {
      ERR_failinit("Unable to set video mode");
    }
  } else {
    Y_get_videomode(&SCRW, &SCRH);
    V_update_buffer();
    R_setgamma(gammaa);
  }
}

void R_toggle_fullscreen (void) {
  Y_set_fullscreen(!Y_get_fullscreen());
  fullscreen = Y_get_fullscreen();
  Y_get_videomode(&SCRW, &SCRH);
  V_update_buffer();
  R_setgamma(gammaa);
}

static int video_menu_handler (menu_msg_t *msg, const menu_t *m, void *data, int i) {
  static int cur;
  static int w, h, fullscreen;
  static char buf[16];
  static int buflen;
  static int vmode;
  const videomode_t *v;
  enum { VIDEOMODE, FULLSCREEN, APPLY, __NUM__ };
  static const simple_menu_t sm = {
    GM_BIG, "Video", NULL,
    {
      { "Mode: ", NULL },
      { "Fullscreen: ", NULL },
      { "Apply ", NULL },
    }
  };
  if (msg->type == GM_ENTER) {
    Y_get_videomode(&w, &h);
    fullscreen = Y_get_fullscreen();
    v = Y_get_videomode_list_opengl(fullscreen);
    vmode = 0;
    while (vmode < v->n && v->modes[vmode].w != w && v->modes[vmode].h != h) {
      vmode += 1;
    }
    if (vmode < v->n) {
      w = v->modes[vmode].w;
      h = v->modes[vmode].h;
    }
    snprintf(buf, 16, "%ix%i", w, h);
    buflen = strlen(buf);
    return 1;
  }
  if (i == VIDEOMODE) {
    switch (msg->type) {
      case GM_GETSTR: return GM_init_str(msg, buf, buflen);
      case GM_SELECT:
        v = Y_get_videomode_list_opengl(fullscreen);
        vmode = vmode + 1 >= v->n ? 0 : vmode + 1;
        if (v->n > 0) {
          w = v->modes[vmode].w;
          h = v->modes[vmode].h;
        } else {
          Y_get_videomode(&w, &h);
        }
        snprintf(buf, 16, "%ix%i", w, h);
        buflen = strlen(buf);
        return 1;
    }
  } else if (i == FULLSCREEN) {
    switch (msg->type) {
      case GM_GETSTR: return GM_init_str(msg, fullscreen ? "Yes" : "No ", 3);
      case GM_SELECT: fullscreen = !fullscreen; return 1;
    }
  } else if (i == APPLY) {
    switch (msg->type) {
      case GM_SELECT: R_set_videomode(w, h, fullscreen); return 1;
    }
  }
  return simple_menu_handler(msg, i, __NUM__, &sm, &cur);
}

const menu_t *R_menu (void) {
  static const menu_t m = { video_menu_handler };
  return &m;
}

const cfg_t *R_args (void) {
  static const cfg_t args[] = {
    { "fullscr", &init_screen_full, Y_SW_ON },
    { "window", &init_screen_full, Y_SW_OFF },
    { "width", &init_screen_width, Y_DWORD },
    { "height", &init_screen_height, Y_DWORD },
    { "gamma", &init_screen_gammaa, Y_DWORD },
    { NULL, NULL, 0 } // end
  };
  return args;
}

const cfg_t *R_conf (void) {
  static const cfg_t conf[] = {
    { "sky", &w_horiz, Y_SW_ON },
    { "fullscreen", &fullscreen, Y_SW_ON },
    { "screen_width", &SCRW, Y_DWORD },
    { "screen_height", &SCRH, Y_DWORD },
    { "gamma", &gammaa, Y_DWORD },
    { NULL, NULL, 0 } // end
  };
  return conf;
}

void R_init () {
  int i;
  logo("R_init: initialize software render\n");
  F_loadres(F_getresid("PLAYPAL"), main_pal, 0, 768);
  for (i = 0; i < 256; ++i) {
    bright[i] = ((int) main_pal[i][0] + main_pal[i][1] + main_pal[i][2]) * 8 / (63 * 3);
  }
  F_loadres(F_getresid("MIXMAP"), mixmap, 0, 0x10000);
  F_loadres(F_getresid("COLORMAP"), clrmap, 0, 256*12);
  SCRW = init_screen_width > 0 ? init_screen_width : SCRW;
  SCRH = init_screen_height > 0 ? init_screen_height : SCRH;
  fullscreen = init_screen_full != 0xFF ? init_screen_full : fullscreen;
  gammaa = init_screen_gammaa >= 0 ? init_screen_gammaa : gammaa;
  R_set_videomode(SCRW, SCRH, fullscreen);
  V_setrect(0, SCRW, 0, SCRH);
  V_clr(0, SCRW, 0, SCRH, 0);
  R_alloc();
}

void R_done (void) {
  buffer = NULL;
  buf_w = 0;
  buf_h = 0;
  pitch = 0;
  Y_unset_videomode();
}

void R_switch_texture (int x, int y) {
  assert(x >= 0 && x < FLDW);
  assert(y >= 0 && y < FLDH);
  fldb[y][x] = walswp[fldb[y][x]];
}

int R_get_swp (int n) {
  assert(n >= 0 && n < 256);
  return walswp[n];
}
