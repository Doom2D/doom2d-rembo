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
#include "files.h"
#include "memory.h"
#include "error.h"
#include "sound.h"
#include "view.h"
#include "player.h"
#include "switch.h"
#include "menu.h"
#include "misc.h"
#include "render.h"
#include "config.h"
#include "game.h"
#include "player.h"
#include "sound.h"
#include "music.h"
#include "input.h"
#include "system.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define PCOLORN 10
static byte pcolortab[PCOLORN] = {
  0x18, 0x20, 0x40, 0x58, 0x60, 0x70, 0x80, 0xB0, 0xC0, 0xD0
};
static int p1color = 5;
static int p2color = 4;

byte _warp;

#define MAX_STACK 8
static struct {
  int n;
  const new_menu_t *m;
} stack[MAX_STACK];
static int stack_p = -1;

#define GM_MAX_INPUT 24
char ibuf[GM_MAX_INPUT];
byte input;
int icur;
int imax;
static byte cbuf[32];
short lastkey;

#define QSND_NUM 14
static int qsnd[QSND_NUM];
static snd_t *csnd1, *csnd2, *msnd1, *msnd2, *msnd3, *msnd4, *msnd5, *msnd6;
static snd_t *voc;
static int voc_ch;

static void GM_stop (void) {
  if (voc != NULL) {
    if (voc_ch) {
      S_stop(voc_ch);
      voc_ch = 0;
    }
    S_free(voc);
    voc = NULL;
  }
}

static void GM_say (const char nm[8]) {
  snd_t *snd = S_load(nm);
  if (snd) {
    GM_stop();
    voc = S_load(nm);
    voc_ch = S_play(voc, 0, 255);
  }
}

static int GM_init_int (new_msg_t *msg, int i, int a, int b, int s) {
  assert(msg != NULL);
  assert(a <= b);
  assert(s >= 0);
  msg->integer.i = min(max(i, a), b);
  msg->integer.a = a;
  msg->integer.b = b;
  msg->integer.s = s;
  return 1;
}

static int GM_init_str (new_msg_t *msg, char *str, int maxlen) {
  assert(msg != NULL);
  assert(str != NULL);
  assert(maxlen >= 0);
  msg->string.s = str;
  msg->string.maxlen = maxlen;
  return 1;
}

static int GM_newgame_handler (new_msg_t *msg, const new_menu_t *m, void *data) {
  assert(msg != NULL);
  intptr_t i = (intptr_t)data;
  switch (msg->type) {
    case GM_ENTER:
      GM_say("_NEWGAME");
      return 1;
    case GM_SELECT:
      _2pl = 0;
      g_dm = 0;
      switch (i) {
        case 0: GM_say("_1PLAYER"); break;
        case 1: GM_say("_2PLAYER"); break;
        case 2: GM_say("_DM"); break;
        // GM_say("_COOP");
      }
      switch (i) {
        case 2: // DEATHMATCH
          g_dm = 1;
        case 1: // COOPERATIVE
          _2pl = 1;
        case 0: // SINGLEPLAYER
          g_map = _warp ? _warp : 1;
          PL_reset();
          pl1.color = pcolortab[p1color];
          pl2.color = pcolortab[p2color];
          G_start();
          GM_popall();
          return 1;
      }
      break;
  }
  return 0;
}

static int GM_var_handler (new_msg_t *msg, const new_menu_t *m, void *data) {
  assert(msg != NULL);
  if (data == &snd_vol) {
    switch (msg->type) {
      case GM_GETINT: return GM_init_int(msg, snd_vol, 0, 128, 8);
      case GM_SETINT: S_volume(msg->integer.i); return 1;
    }
  } else if (data == &mus_vol) {
    switch (msg->type) {
      case GM_GETINT: return GM_init_int(msg, mus_vol, 0, 128, 8);
      case GM_SETINT: S_volumemusic(msg->integer.i); return 1;
    }
  } else if (data == g_music) {
    switch (msg->type) {
      case GM_GETSTR:
        return GM_init_str(msg, g_music, 8);
      case GM_SELECT:
        F_freemus();
        F_nextmus(g_music);
        F_loadmus(g_music);
        S_startmusic(music_time * 2); // ???
        return 1;
    }
  }
  return 0;
}

static int GM_load_handler (new_msg_t *msg, const new_menu_t *m, void *data) {
  assert(msg != NULL);
  intptr_t i = (intptr_t)data;
  switch (msg->type) {
    case GM_ENTER:
      F_getsavnames();
      return 1;
    case GM_GETSTR:
      return GM_init_str(msg, (char*)savname[i], 24);
    case GM_SELECT:
      if (savok[i]) {
        load_game(i);
        GM_popall();
        return 1;
      }
      break;
  }
  return 0;
}

static int GM_save_handler (new_msg_t *msg, const new_menu_t *m, void *data) {
  assert(msg != NULL);
  intptr_t i = (intptr_t)data;
  switch (msg->type) {
    case GM_ENTER:
      if (g_st == GS_GAME) {
        F_getsavnames();
      } else {
        GM_pop();
      }
      return 1;
    case GM_GETSTR:
      F_getsavnames();
      return GM_init_str(msg, (char*)savname[i], 24);
    case GM_END:
      if (g_st == GS_GAME) {
        F_savegame(i, msg->string.s); // TODO check size
        GM_popall();
        return 1;
      }
      break;
  }
  return 0;
}

static int GM_exit_handler (new_msg_t *msg, const new_menu_t *m, void *data) {
  switch (msg->type) {
    case GM_ENTER:
      GM_say(rand() & 1 ? "_EXIT1" : "_EXIT2");
      return 1;
    case GM_SELECT:
      if (data != NULL) {
        F_freemus();
        GM_stop();
        Z_sound(S_get(qsnd[myrand(QSND_NUM)]), 255);
        S_wait();
        ERR_quit();
      } else {
        GM_pop();
      }
      return 1;
  }
  return 0;
}

static const new_menu_t newgame_menu = {
  GM_BIG, "New game", NULL, NULL,
  {
    { GM_BUTTON, "One player", (void*)0, &GM_newgame_handler, NULL },
    { GM_BUTTON, "Two players", (void*)1, &GM_newgame_handler, NULL },
    { GM_BUTTON, "Deathmatch", (void*)2, &GM_newgame_handler, NULL },
    { 0, NULL, NULL, NULL, NULL } // end
  }
}, loadgame_menu = {
  GM_BIG, "Load game", NULL, &GM_load_handler,
  {
    { GM_TEXTFIELD_BUTTON, "", (void*)0, &GM_load_handler, NULL },
    { GM_TEXTFIELD_BUTTON, "", (void*)1, &GM_load_handler, NULL },
    { GM_TEXTFIELD_BUTTON, "", (void*)2, &GM_load_handler, NULL },
    { GM_TEXTFIELD_BUTTON, "", (void*)3, &GM_load_handler, NULL },
    { GM_TEXTFIELD_BUTTON, "", (void*)4, &GM_load_handler, NULL },
    { GM_TEXTFIELD_BUTTON, "", (void*)5, &GM_load_handler, NULL },
    { GM_TEXTFIELD_BUTTON, "", (void*)6, &GM_load_handler, NULL },
    { 0, NULL, NULL, NULL, NULL } // end
  }
}, savegame_menu = {
  GM_BIG, "Save game", NULL, &GM_save_handler,
  {
    { GM_TEXTFIELD, "", (void*)0, &GM_save_handler, NULL },
    { GM_TEXTFIELD, "", (void*)1, &GM_save_handler, NULL },
    { GM_TEXTFIELD, "", (void*)2, &GM_save_handler, NULL },
    { GM_TEXTFIELD, "", (void*)3, &GM_save_handler, NULL },
    { GM_TEXTFIELD, "", (void*)4, &GM_save_handler, NULL },
    { GM_TEXTFIELD, "", (void*)5, &GM_save_handler, NULL },
    { GM_TEXTFIELD, "", (void*)6, &GM_save_handler, NULL },
    { 0, NULL, NULL, NULL, NULL } // end
  }
}, sound_menu = {
  GM_BIG, "Sound", NULL, NULL,
  {
    { GM_SCROLLER, "Volume", &snd_vol, &GM_var_handler, NULL },
    { 0, NULL, NULL, NULL, NULL } // end
  }
}, music_menu = {
  GM_BIG, "Music", NULL, NULL,
  {
    { GM_SCROLLER, "Volume", &mus_vol, &GM_var_handler, NULL },
    { GM_BUTTON, "Music: ", g_music, &GM_var_handler, NULL },
    { 0, NULL, NULL, NULL, NULL } // end
  }
}, options_menu = {
  GM_BIG, "Options", NULL, NULL,
  {
    //{ GM_BUTTON, "Video", NULL, NULL, NULL },
    { GM_BUTTON, "Sound", NULL, NULL, &sound_menu },
    { GM_BUTTON, "Music", NULL, NULL, &music_menu },
    { 0, NULL, NULL, NULL, NULL } // end
  }
}, exit_menu = {
  GM_SMALL, "You are sure?", NULL, &GM_exit_handler,
  {
    { GM_SMALL_BUTTON, "Yes", (void*)1, &GM_exit_handler, NULL },
    { GM_SMALL_BUTTON, "No", (void*)0, &GM_exit_handler, NULL },
    { 0, NULL, NULL, NULL, NULL } // end
  }
}, main_menu = {
  GM_BIG, "Menu", NULL, NULL,
  {
    { GM_BUTTON, "New game", NULL, NULL, &newgame_menu },
    { GM_BUTTON, "Load game", NULL, NULL, &loadgame_menu },
    { GM_BUTTON, "Save game", NULL, NULL, &savegame_menu },
    { GM_BUTTON, "Options", NULL, NULL, &options_menu },
    { GM_BUTTON, "Exit", NULL, NULL, &exit_menu },
    { 0, NULL, NULL, NULL, NULL } // end
  }
};

void GM_push (const new_menu_t *m) {
  assert(m != NULL);
  assert(stack_p >= -1);
  assert(stack_p < MAX_STACK - 1);
  new_msg_t msg;
  stack_p += 1;
  if (stack[stack_p].m != m) {
    stack[stack_p].n = 0;
    stack[stack_p].m = m;
  }
  msg.type = GM_ENTER;
  GM_send_this(m, &msg);
}

void GM_pop (void) {
  assert(stack_p >= 0);
  new_msg_t msg;
  stack_p -= 1;
  msg.type = GM_LEAVE;
  GM_send_this(stack[stack_p + 1].m, &msg);
}

void GM_popall (void) {
  int i;
  for (i = 0; i >= -1; i--) {
    GM_pop();
  }
}

const new_menu_t *GM_get (void) {
  if (stack_p >= 0) {
    return stack[stack_p].m;
  } else {
    return NULL;
  }
}

int GM_geti (void) {
  if (stack_p >= 0) {
    return stack[stack_p].n;
  } else {
    return 0;
  }
}

static void GM_normalize_message (new_msg_t *msg) {
  switch (msg->type) {
    case GM_SETINT:
      msg->integer.i = min(max(msg->integer.i, msg->integer.a), msg->integer.b);
      break;
    case GM_SETSTR:
      assert(msg->string.maxlen >= 0);
      break;
  }
}

int GM_send_this (const new_menu_t *m, new_msg_t *msg) {
  assert(m != NULL);
  assert(msg != NULL);
  if (m->handler != NULL) {
    GM_normalize_message(msg);
    return m->handler(msg, m, m->data);
  }
  return 0;
}

int GM_send (const new_menu_t *m, int i, new_msg_t *msg) {
  assert(m != NULL);
  assert(i >= 0);
  assert(msg != NULL);
  const new_var_t *v = &m->entries[i];
  if (v->handler != NULL) {
    GM_normalize_message(msg);
    return v->handler(msg, m, v->data);
  }
  return 0;
}

void G_code (void) {
  void *s;
  s=csnd2;
  if(memcmp(cbuf+32-5,"IDDQD",5)==0) {
    PL_hit(&pl1,400,0,HIT_SOME);
    if(_2pl) PL_hit(&pl2,400,0,HIT_SOME);
    s=csnd1;
  }else if(memcmp(cbuf+32-4,"TANK",4)==0) {
    pl1.life=pl1.armor=200;pl1.drawst|=PL_DRAWARMOR|PL_DRAWLIFE;
    if(_2pl) {pl2.life=pl2.armor=200;pl2.drawst|=PL_DRAWARMOR|PL_DRAWLIFE;}
  }else if(memcmp(cbuf+32-8,"BULLFROG",8)==0) {
    PL_JUMP=(PL_JUMP==10)?20:10;
  }else if(memcmp(cbuf+32-8,"FORMULA1",8)==0) {
    PL_RUN=(PL_RUN==8)?24:8;
  }else if(memcmp(cbuf+32-5,"RAMBO",5)==0) {
    pl1.ammo=pl1.shel=pl1.rock=pl1.cell=pl1.fuel=30000;
    pl1.wpns=0x7FF;pl1.drawst|=PL_DRAWWPN|PL_DRAWKEYS;
    pl1.keys=0x70;
    if(_2pl) {
      pl2.ammo=pl2.shel=pl2.rock=pl2.cell=pl1.fuel=30000;
      pl2.wpns=0x7FF;pl2.drawst|=PL_DRAWWPN|PL_DRAWKEYS;
      pl2.keys=0x70;
    }
  }else if(memcmp(cbuf+32-5,"UJHTW",5)==0) {
    p_immortal=!p_immortal;
  }else if(memcmp(cbuf+32-9,",TKSQJHTK",9)==0) {
    p_fly=!p_fly;
  }else if(memcmp(cbuf+32-6,"CBVCBV",6)==0) {
    SW_cheat_open();
  }else if(memcmp(cbuf+32-7,"GOODBYE",7)==0) {
    g_exit=1;
  }else if(memcmp(cbuf+32-9,"GJITKYF",7)==0) {
    if(cbuf[30]>='0' && cbuf[30]<='9' && cbuf[31]>='0' && cbuf[31]<='9') {
      g_map=(cbuf[30]=='0')?0:(cbuf[30]-'0')*10;
      g_map+=(cbuf[31]=='0')?0:(cbuf[31]-'0');
      G_start();
    }
  }else return;
  memset(cbuf,0,32);
  Z_sound(s,128);
}

static int count_menu_entries (const new_menu_t *m) {
  assert(m != NULL);
  int i = 0;
  while (m->entries[i].type != 0) {
    i += 1;
  }
  return i;
}

static int strnlen (const char *s, int len) {
  int i = 0;
  while (i < len && s[i] != 0) {
    i++;
  }
  return i;
}

static int state_for_anykey (int x) {
  return x == GS_TITLE || x == GS_ENDSCR;
}

int GM_act (void) {
  int n, cur;
  new_msg_t msg;
  const new_var_t *v;
  const new_menu_t *m = GM_get ();
  if (m == NULL) {
    if (lastkey == KEY_ESCAPE || (state_for_anykey(g_st) && lastkey != KEY_UNKNOWN)) {
      GM_push(&main_menu);
      Z_sound(msnd3, 128);
    }
  } else {
    n = count_menu_entries(m);
    cur = stack[stack_p].n;
    v = &m->entries[cur];
    switch (lastkey) {
      case KEY_ESCAPE:
        if (v->type == GM_TEXTFIELD && input) {
          input = 0;
          Y_disable_text_input();
          msg.type = GM_CANCEL;
          GM_send(m, cur, &msg);
        } else {
          GM_pop();
          Z_sound(msnd4, 128);
        }
        break;
      case KEY_UP:
        stack[stack_p].n = stack[stack_p].n - 1 < 0 ? n - 1 : stack[stack_p].n - 1;
        Z_sound(msnd1, 128);
        break;
      case KEY_DOWN:
        stack[stack_p].n = stack[stack_p].n + 1 >= n ? 0 : stack[stack_p].n + 1;
        Z_sound(msnd1, 128);
        break;
      case KEY_LEFT:
      case KEY_RIGHT:
        if (v->type == GM_SCROLLER) {
          msg.integer.type = GM_GETINT;
          if (GM_send(m, cur, &msg)) {
            msg.integer.type = GM_SETINT;
            msg.integer.i += lastkey == KEY_LEFT ? -msg.integer.s : msg.integer.s;
            msg.integer.i = min(max(msg.integer.i, msg.integer.a), msg.integer.b);
            if (GM_send(m, cur, &msg)) {
              Z_sound(lastkey == KEY_LEFT ? msnd5 : msnd6, 255);
            }
          }
        } else if (v->type == GM_TEXTFIELD && input) {
          //icur += lastkey == KEY_LEFT ? -1 : +1;
          //icur = min(max(icur, 0), strnlen(ibuf, imax));
        }
        break;
      case KEY_BACKSPACE:
        if (v->type == GM_TEXTFIELD && input) {
          if (icur > 0) {
            // FIXIT buffers in strncpy must not overlap
            strncpy(&ibuf[icur - 1], &ibuf[icur], imax - icur);
            ibuf[imax - 1] = 0;
            icur -= 1;
          }
        }
        break;
      case KEY_RETURN:
        if (v->submenu != NULL) {
          GM_push(v->submenu);
          Z_sound(msnd2, 128);
        } else if (v->type == GM_TEXTFIELD) {
          if (input) {
            input = 0;
            Y_disable_text_input();
            msg.type = GM_END;
            msg.string.s = ibuf;
            msg.string.maxlen = imax;
            GM_send(m, cur, &msg);
          } else {
            msg.type = GM_GETSTR;
            if (GM_send(m, cur, &msg)) {
              imax = min(msg.string.maxlen, GM_MAX_INPUT);
              strncpy(ibuf, msg.string.s, imax);
              icur = strnlen(ibuf, imax);
            }
            input = 1;
            Y_enable_text_input();
            msg.type = GM_BEGIN;
            GM_send(m, cur, &msg);
          }
        } else {
          msg.type = GM_SELECT;
          GM_send(m, cur, &msg);
        }
        break;
    }
  }
  lastkey = KEY_UNKNOWN;
  return m != NULL;
}

void GM_input (int ch) {
  if (ch != 0 && input) {
    if (icur < imax) {
      ibuf[icur] = ch;
      icur += 1;
      if (icur < imax) {
        ibuf[icur] = 0;
      }
    }
  }
}

void GM_key (int key, int down) {
  int i;
  if (down) {
    lastkey = key;
    if (!_2pl || cheat) {
      for (i = 0; i < 31; i++) {
        cbuf[i] = cbuf[i + 1];
      }
      if (key >= KEY_0 && key <= KEY_9) {
        cbuf[31] = key - KEY_0 + '0';
      } else if (key >= KEY_A && key <= KEY_Z) {
        cbuf[31] = key - KEY_A + 'A';
      } else {
        cbuf[31] = 0;
      }
    }
  }
}

void GM_init (void) {
  int i;
  char s[8];
  static const char nm[QSND_NUM][6] = {
    "CYBSIT", "KNTDTH", "MNPAIN", "PEPAIN", "SLOP", "MANSIT", "BOSPN", "VILACT",
    "PLFALL", "BGACT", "BGDTH2", "POPAIN", "SGTATK", "VILDTH"
  };
  s[0] = 'D';
  s[1] = 'S';
  for (i = 0; i < QSND_NUM; ++i) {
    memcpy(s + 2, nm[i], 6);
    qsnd[i] = F_getresid(s);
  }
  csnd1 = Z_getsnd("HAHA1");
  csnd2 = Z_getsnd("RADIO");
  msnd1 = Z_getsnd("PSTOP");
  msnd2 = Z_getsnd("PISTOL");
  msnd3 = Z_getsnd("SWTCHN");
  msnd4 = Z_getsnd("SWTCHX");
  msnd5 = Z_getsnd("SUDI");
  msnd6 = Z_getsnd("TUDI");
  F_loadmus("MENU");
  S_startmusic(0);
}
