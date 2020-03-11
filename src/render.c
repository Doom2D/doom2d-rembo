#include "glob.h"
#include "render.h"
#include "view.h"
#include "player.h"
#include "switch.h"
#include "vga.h"
#include "menu.h"
#include "misc.h"

extern byte clrmap[256*12]; // main.c
extern void *scrnh[3]; // game.c
extern int g_trans; // game.c
extern byte transdraw; // game.c
extern void *horiz; // view.c
extern int sky_type; // view.c
extern int lt_time, lt_type, lt_side, lt_ypos, lt_force; // game.c
extern void *ltn[2][2]; // game.c

/* --- menu --- */

/*
static int GM_draw (void) {
  int i, j, k, y;
  ++gm_tm;
  V_setrect(0, SCRW, 0, SCRH);
  if (!mnu && !gm_redraw) {
    return 0;
  }
  gm_redraw = 0;
  if (!mnu) {
    return 1;
  }
  if (mnu->type == MENU) {
    y = (200 - mnu -> n * 16 - 20) / 2;
    Z_gotoxy(mnu->x, y - 10); Z_printbf(mnu->ttl);
    for (i = 0; i < mnu->n; ++i) {
      if (mnu->t[i] == LOAD || mnu->t[i] == SAVE) {
        j = y + i * 16 + 29;
        V_spr(mnu->x, j, mslotl);
        for (k = 8; k < 184; k += 8) {
          V_spr(mnu->x + k, j, mslotm);
        }
        V_spr(mnu->x+184,j,mslotr);
        Z_gotoxy(mnu->x+4,j-8);
        if (input && i == save_mnu.cur) {
          Z_printsf("%s_", ibuf);
        } else {
          Z_printsf("%s", savname[i]);
        }
      } else {
        Z_gotoxy(mnu->x + (mnu->t[i] >= SVOLM ? (mnu->t[i] >= PL1CM ? 50 : 152) : 0), y + i * 16 + 20);
        Z_printbf(mnu->m[i]);
      }
      if (mnu->t[i] == MUSIC) {
        Z_printbf(" '%.8s'",g_music);
      } else if(mnu->t[i] == INTERP) {
        Z_printbf("%s", fullscreen ? "ON" : "OFF");
      } else if(mnu->t[i] >= PL1CM) {
        V_manspr(mnu->x + (mnu->t[i] == PL1CM ? 15 : 35), y + i * 16 + 20 + 14, PL_getspr(*panimp, 0), pcolortab[(mnu->t[i] == PL1CM) ? p1color : p2color]);
      } else if(mnu->t[i]>=SVOLM) {
        j = y + i * 16 + 20'
        V_spr(mnu->x, j, mbarl);
        for (k = 8; k < 144; k += 8) {
          V_spr(mnu->x + k, j, mbarm);
        }
        V_spr(mnu->x + 144, j, mbarr);
        switch(mnu->t[i]) {
          case SVOLM: k = snd_vol; break;
          case MVOLM: k = mus_vol; break;
          case GAMMAM: k = gammaa << 5; break;
        }
        V_spr(mnu->x+8+k,j,mbaro);
      }
    }
    V_spr(mnu->x - 25, y + mnu->cur * 16 + 20 - 8, msklh[(gm_tm / 6) & 1]);
  } else {
    Z_gotoxy((320 - strlen(mnu->ttl) * 7) / 2, 90); Z_printsf(mnu->ttl);
    Z_gotoxy(136, 100); Z_printsf("(Y/N)");
  }
  return 1;
}
*/

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
  if(_2pl) {
    PL_draw(&pl2);
  }
  MN_draw();
  WP_draw();
  SMK_draw();
  FX_draw();
  Z_drawfld((byte*)fldf, 0);
  if(sky_type == 2) {
    if(lt_time == -4 || lt_time == -2) {
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

void G_draw (void) {
  int h;
  word hr, mn, sc;
  if (g_trans && !transdraw) {
    return;
  }
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
  V_center(1);
  if (g_st != GS_GAME) {
    if (g_trans) {
      return;
    }
    GM_draw();
    V_copytoscr(0, SCRW, 0, SCRH);
    return;
  }
  V_center(0);
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
  if (g_trans) {
    return;
  }
  V_center(1);
  if (GM_draw()) {
    pl1.drawst = 0xFF;
    pl2.drawst = 0xFF;
    V_copytoscr(0,SCRW,0,SCRH);
    return;
  }
  V_center(0);
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
  } else {
    V_copytoscr(0, SCRW - 120, 1, _2pl ? SCRH / 2 - 2 : SCRH - 2);
  }
  if (pl1.drawst) {
    V_copytoscr(SCRW - 120, 120, 0, _2pl ? SCRH / 2 : SCRH);
  }
  pl1.drawst = 0xFF;
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
    } else {
      V_copytoscr(0, SCRW - 120, SCRH / 2 + 1, SCRH / 2 - 2);
    }
    if (pl2.drawst) {
      V_copytoscr(SCRW - 120, 120, SCRH / 2, SCRH / 2);
    }
    pl2.drawst = 0xFF;
  }
}

