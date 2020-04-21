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

#include "save.h"

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
  const menu_t *m;
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

static int GM_say (const char nm[8]) {
  snd_t *snd = S_load(nm);
  if (snd) {
    GM_stop();
    voc = S_load(nm);
    voc_ch = S_play(voc, 0, 255);
  }
  return 1;
}

int GM_init_int0 (menu_msg_t *msg, int i, int a, int b, int s) {
  assert(msg != NULL);
  msg->integer.i = i;
  msg->integer.a = a;
  msg->integer.b = b;
  msg->integer.s = s;
  return 1;
}

int GM_init_int (menu_msg_t *msg, int i, int a, int b, int s) {
  assert(msg != NULL);
  assert(a <= b);
  assert(s >= 0);
  return GM_init_int0(msg, min(max(i, a), b), a, b, s);
}

int GM_init_str (menu_msg_t *msg, char *str, int maxlen) {
  assert(msg != NULL);
  assert(str != NULL);
  assert(maxlen >= 0);
  msg->string.s = str;
  msg->string.maxlen = maxlen;
  return 1;
}

int basic_menu_handler (menu_msg_t *msg, byte type, char *title, char *say, int n, int *cur) {
  assert(msg != NULL);
  assert(type == GM_BIG || type == GM_SMALL);
  assert(title != NULL);
  assert(n >= 0);
  assert(cur != NULL);
  switch (msg->type) {
    case GM_QUERY: return GM_init_int0(msg, *cur, n, n, type);
    case GM_GETTITLE: return GM_init_str(msg, title, strlen(title));
    case GM_ENTER: return say ? GM_say(say) : 1;
    case GM_UP: *cur = GM_CYCLE(*cur - 1, 0, n - 1); return 1;
    case GM_DOWN: *cur = GM_CYCLE(*cur + 1, 0, n - 1); return 1;
  }
  return 0;
}

int simple_menu_handler (menu_msg_t *msg, int i, int n, const simple_menu_t *m, int *cur) {
  assert(msg != NULL);
  assert(n >= 0);
  assert(i >= 0 && i < n);
  assert(m != NULL);
  assert(cur != NULL);
  switch (msg->type) {
    case GM_GETENTRY: return GM_init_int0(msg, m->type == GM_SMALL ? GM_SMALL_BUTTON : GM_BUTTON, 0, 0, 0);
    case GM_GETCAPTION: return GM_init_str(msg, m->entries[i].caption, strlen(m->entries[i].caption));
    case GM_SELECT: return m->entries[i].submenu ? GM_push(m->entries[i].submenu) : 1;
  }
  return basic_menu_handler(msg, m->type, m->title, m->say, n, cur);
}

static int start_game (int twoplayers, int dm, int level) {
  _2pl = twoplayers;
  g_dm = dm;
  g_map = level ? level : 1;
  PL_reset();
  pl1.color = pcolortab[p1color];
  pl2.color = pcolortab[p2color];
  G_start();
  return GM_popall();
}

static int new_game_menu_handler (menu_msg_t *msg, const menu_t *m, void *data, int i) {
  static int cur;
  enum { ONEPLAYER, TWOPLAYERS, DEATHMATCH, __NUM__ };
  static const simple_menu_t sm = {
    GM_BIG, "New Game", "_NEWGAME",
    {
      { "One Player", NULL },
      { "Two Players", NULL },
      { "Deathmatch", NULL },
    }
  };
  if (msg->type == GM_SELECT) {
    switch (i) {
      case ONEPLAYER: GM_say("_1PLAYER"); return start_game(0, 0, _warp);
      case TWOPLAYERS: GM_say("_2PLAYER"); return start_game(1, 0, _warp);
      case DEATHMATCH: GM_say("_DM"); return start_game(1, 1, _warp);
      // GM_say("_COOP");
    }
  }
  return simple_menu_handler(msg, i, __NUM__, &sm, &cur);
}

static const menu_t new_game_menu = {
  NULL, &new_game_menu_handler
};

static int load_game_menu_handler (menu_msg_t *msg, const menu_t *m, void *data, int i) {
  static int cur;
  const int max_slots = 7;
  assert(i >= 0 && i < max_slots);
  switch (msg->type) {
    case GM_ENTER: F_getsavnames(); break;
    case GM_GETENTRY: return GM_init_int0(msg, GM_TEXTFIELD_BUTTON, 0, 0, 0);
    case GM_GETSTR: return GM_init_str(msg, (char*)savname[i], 24);
    case GM_SELECT:
      if (savok[i]) {
        load_game(i);
        GM_popall();
      }
      return 1;
  }
  return basic_menu_handler(msg, GM_BIG, "Load game", "_OLDGAME", max_slots, &cur);
}

static const menu_t load_game_menu = {
  NULL, &load_game_menu_handler
};

static int save_game_menu_handler (menu_msg_t *msg, const menu_t *m, void *data, int i) {
  static int cur;
  const int max_slots = 7;
  assert(i >= 0 && i < max_slots);
  switch (msg->type) {
    case GM_ENTER:
      if (g_st == GS_GAME) {
        F_getsavnames();
        break;
      } else {
        return GM_pop();
      }
    case GM_GETENTRY: return GM_init_int0(msg, GM_TEXTFIELD, 0, 0, 0);
    case GM_GETSTR: return GM_init_str(msg, (char*)savname[i], 24);
    case GM_END:
      if (g_st == GS_GAME) {
        assert(msg->string.maxlen >= 24);
        F_savegame(i, msg->string.s);
      }
      return GM_popall();
  }
  return basic_menu_handler(msg, GM_BIG, "Save game", "_SAVGAME", max_slots, &cur);
}

static const menu_t save_game_menu = {
  NULL, &save_game_menu_handler
};

static int sound_menu_handler (menu_msg_t *msg, const menu_t *m, void *data, int i) {
  static int cur;
  enum { VOLUME, __NUM__ };
  static const simple_menu_t sm = {
    GM_BIG, "Sound", NULL,
    {
      { "Volume", NULL },
    }
  };
  if (i == VOLUME) {
    switch (msg->type) {
      case GM_GETENTRY: return GM_init_int0(msg, GM_SCROLLER, 0, 0, 0);
      case GM_GETINT: return GM_init_int(msg, snd_vol, 0, 128, 8);
      case GM_SETINT: S_volume(msg->integer.i); return 1;
    }
  }
  return simple_menu_handler(msg, i, __NUM__, &sm, &cur);
}

static const menu_t sound_menu = {
  NULL, &sound_menu_handler
};

static int music_menu_handler (menu_msg_t *msg, const menu_t *m, void *data, int i) {
  static int cur;
  enum { VOLUME, MUSIC, __NUM__ };
  static const simple_menu_t sm = {
    GM_BIG, "Music", NULL,
    {
      { "Volume", NULL },
      { "Music: ", NULL },
    }
  };
  if (i == VOLUME) {
    switch (msg->type) {
      case GM_GETENTRY: return GM_init_int0(msg, GM_SCROLLER, 0, 0, 0);
      case GM_GETINT: return GM_init_int(msg, mus_vol, 0, 128, 8);
      case GM_SETINT: S_volumemusic(msg->integer.i); return 1;
    }
  } else if (i == MUSIC) {
    switch (msg->type) {
      case GM_GETSTR: return GM_init_str(msg, g_music, strlen(g_music));
      case GM_SELECT:
        F_freemus();
        F_nextmus(g_music);
        F_loadmus(g_music);
        S_startmusic(music_time * 2); // ???
        return 1;
    }
  }
  return simple_menu_handler(msg, i, __NUM__, &sm, &cur);
}

static const menu_t music_menu = {
  NULL, &music_menu_handler
};

static int options_menu_handler (menu_msg_t *msg, const menu_t *m, void *data, int i) {
  static int cur;
  enum { VIDEO, SOUND, MUSIC, __NUM__ };
  static const simple_menu_t sm = {
    GM_BIG, "Options", NULL,
    {
      { "Video", NULL },
      { "Sound", &sound_menu },
      { "Music", &music_menu },
    }
  };
  if (msg->type == GM_SELECT) {
    if (i == VIDEO) {
      const menu_t *mm = R_menu();
      return mm ? GM_push(mm) : 1;
    }
  }
  return simple_menu_handler(msg, i, __NUM__, &sm, &cur);
}

static const menu_t options_menu = {
  NULL, &options_menu_handler
};

static int exit_menu_handler (menu_msg_t *msg, const menu_t *m, void *data, int i) {
  static int cur;
  enum { YES, NO, __NUM__ };
  static const simple_menu_t sm = {
    GM_SMALL, "You are sure?", NULL,
    {
      { "Yes", NULL },
      { "No", NULL },
    }
  };
  if (msg->type == GM_ENTER) {
    return GM_say(rand() & 1 ? "_EXIT1" : "_EXIT2");
  } else if (msg->type == GM_SELECT) {
    switch (i) {
      case YES:
        F_freemus();
        GM_stop();
        Z_sound(S_get(qsnd[myrand(QSND_NUM)]), 255);
        S_wait();
        ERR_quit();
        return 1;
      case NO:
        return GM_pop();
    }
  }
  return simple_menu_handler(msg, i, __NUM__, &sm, &cur);
}

static const menu_t exit_menu = {
  NULL, &exit_menu_handler
};

static int main_menu_handler (menu_msg_t *msg, const menu_t *m, void *data, int i) {
  static int cur;
  enum { NEWGAME, OLDGAME, SAVEGAME, OPTIONS, EXIT, __NUM__ };
  static const simple_menu_t sm = {
    GM_BIG, "Menu", NULL,
    {
      { "New Game", &new_game_menu },
      { "Load Game", &load_game_menu },
      { "Save Game", &save_game_menu },
      { "Options", &options_menu },
      { "Exit", &exit_menu },
    }
  };
  return simple_menu_handler(msg, i, __NUM__, &sm, &cur);
}

static const menu_t main_menu = {
  NULL, &main_menu_handler
};

int GM_push (const menu_t *m) {
  assert(m != NULL);
  assert(stack_p >= -1);
  assert(stack_p < MAX_STACK - 1);
  menu_msg_t msg;
  stack_p += 1;
  stack[stack_p].m = m;
  msg.type = GM_ENTER;
  GM_send_this(m, &msg);
  return 1;
}

int GM_pop (void) {
  assert(stack_p >= 0);
  menu_msg_t msg;
  stack_p -= 1;
  msg.type = GM_LEAVE;
  GM_send_this(stack[stack_p + 1].m, &msg);
  return 1;
}

int GM_popall (void) {
  int i;
  for (i = 0; i >= -1; i--) {
    GM_pop();
  }
  return 1;
}

const menu_t *GM_get (void) {
  if (stack_p >= 0) {
    return stack[stack_p].m;
  } else {
    return NULL;
  }
}

static void GM_normalize_message (menu_msg_t *msg) {
  switch (msg->type) {
    case GM_SETINT:
      msg->integer.i = min(max(msg->integer.i, msg->integer.a), msg->integer.b);
      break;
    case GM_SETSTR:
      assert(msg->string.maxlen >= 0);
      break;
  }
}

int GM_send_this (const menu_t *m, menu_msg_t *msg) {
  assert(m != NULL);
  assert(msg != NULL);
  if (m->handler != NULL) {
    GM_normalize_message(msg);
    return m->handler(msg, m, m->data, 0);
  }
  return 0;
}

int GM_send (const menu_t *m, int i, menu_msg_t *msg) {
  assert(m != NULL);
  assert(i >= 0);
  assert(msg != NULL);
  if (m->handler != NULL) {
    GM_normalize_message(msg);
    return m->handler(msg, m, m->data, i);
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
  menu_msg_t msg;
  int n, cur, type;
  const menu_t *m = GM_get ();
  if (m == NULL) {
    if (lastkey == KEY_ESCAPE || (state_for_anykey(g_st) && lastkey != KEY_UNKNOWN)) {
      GM_push(&main_menu);
      Z_sound(msnd3, 128);
    }
  } else {
    msg.type = GM_QUERY;
    assert(GM_send_this(m, &msg));
    cur = msg.integer.i;
    n = msg.integer.a;
    msg.type = GM_GETENTRY;
    assert(GM_send(m, cur, &msg));
    type = msg.integer.i;
    switch (lastkey) {
      case KEY_ESCAPE:
        if (type == GM_TEXTFIELD && input) {
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
      case KEY_DOWN:
        msg.type = lastkey == KEY_UP ? GM_UP : GM_DOWN;
        if (GM_send(m, cur, &msg)) {
          Z_sound(msnd1, 128);
        }
        break;
      case KEY_LEFT:
      case KEY_RIGHT:
        if (type == GM_SCROLLER) {
          msg.integer.type = GM_GETINT;
          if (GM_send(m, cur, &msg)) {
            msg.integer.type = GM_SETINT;
            msg.integer.i += lastkey == KEY_LEFT ? -msg.integer.s : msg.integer.s;
            msg.integer.i = min(max(msg.integer.i, msg.integer.a), msg.integer.b);
            if (GM_send(m, cur, &msg)) {
              Z_sound(lastkey == KEY_LEFT ? msnd5 : msnd6, 255);
            }
          }
        } else if (type == GM_TEXTFIELD) {
          //if (input) {
          //  icur += lastkey == KEY_LEFT ? -1 : +1;
          //  icur = min(max(icur, 0), strnlen(ibuf, imax));
          //}
        }
        break;
      case KEY_BACKSPACE:
        if (type == GM_TEXTFIELD) {
          if (input && icur > 0) {
            // FIXIT buffers in strncpy must not overlap
            strncpy(&ibuf[icur - 1], &ibuf[icur], imax - icur);
            ibuf[imax - 1] = 0;
            icur -= 1;
          }
        }
        break;
      case KEY_RETURN:
        if (type == GM_TEXTFIELD) {
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
            } else {
              memset(ibuf, 0, GM_MAX_INPUT);
              imax = GM_MAX_INPUT;
              icur = 0;
            }
            input = 1;
            Y_enable_text_input();
            msg.type = GM_BEGIN;
            GM_send(m, cur, &msg);
          }
          Z_sound(msnd2, 128);
        } else {
          msg.type = GM_SELECT;
          if (GM_send(m, cur, &msg)) {
            Z_sound(msnd2, 128);
          }
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
