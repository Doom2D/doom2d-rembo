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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "map.h"
#include "sound.h"
#include "music.h"
#include "view.h"
#include "monster.h"
#include "player.h"
#include "menu.h"
#include "files.h"
#include "render.h"
#include "error.h"
#include "input.h"
#include "my.h"

enum {NONE, BYTE, WORD, DWORD, STRING, SW_ON, SW_OFF, FILES, KEY};

typedef struct cfg_t {
  char *par, *cfg;
  void *p;
  byte t;
} cfg_t;

byte cheat;
byte shot_vga;

static FILE *f;
static int ch;

static cfg_t cfg[] = {
  {"file", NULL, NULL, FILES},
  {"cheat", NULL, &cheat, SW_ON},
  {"vga", "screenshot", &shot_vga, SW_ON},
  {"sndvol", "sound_volume", &snd_vol, WORD},
  {"musvol", "music_volume", &mus_vol, WORD},
//  {"fullscr", "fullscreen", &fullscreen, SW_ON},
//  {"window", NULL, &fullscreen, SW_OFF},
  {NULL, "sky", &w_horiz, SW_ON},
  {"mon", NULL, &nomon, SW_OFF},
//  {"gamma", "gamma", &gammaa, DWORD},
  {"warp", NULL, &_warp, BYTE},
  {"width", "screen_width", &SCRW, DWORD},
  {"height", "screen_height", &SCRH, DWORD},
  {NULL, "music_random", &music_random, SW_ON},
  {NULL, "music_time", &music_time, DWORD},
  {NULL, "music_fade", &music_fade, DWORD},
  {NULL, "pl1_left", &pl1.kl, KEY},
  {NULL, "pl1_right",&pl1.kr, KEY},
  {NULL, "pl1_up", &pl1.ku, KEY},
  {NULL, "pl1_down", &pl1.kd, KEY},
  {NULL, "pl1_jump", &pl1.kj, KEY},
  {NULL, "pl1_fire", &pl1.kf, KEY},
  {NULL, "pl1_next", &pl1.kwr, KEY},
  {NULL, "pl1_prev", &pl1.kwl, KEY},
  {NULL, "pl1_use", &pl1.kp, KEY},
  {NULL, "pl2_left", &pl2.kl, KEY},
  {NULL, "pl2_right",&pl2.kr, KEY},
  {NULL, "pl2_up",   &pl2.ku, KEY},
  {NULL, "pl2_down", &pl2.kd, KEY},
  {NULL, "pl2_jump", &pl2.kj, KEY},
  {NULL, "pl2_fire", &pl2.kf, KEY},
  {NULL, "pl2_next", &pl2.kwr, KEY},
  {NULL, "pl2_prev", &pl2.kwl, KEY},
  {NULL, "pl2_use",  &pl2.kp, KEY},
//  {"config", NULL, cfg_file, STRING},
  {NULL, NULL, NULL, NONE} // end
};

static int CFG_open_iterator (const char *name) {
  assert(f == NULL);
  f = fopen(name, "rb");
  if (f != NULL) {
    ch = fgetc(f);
  }
  return f != NULL;
}

static void CFG_skip_space (void) {
  while (feof(f) == 0 && isspace(ch)) {
    ch = fgetc(f);
  }
}

static void CFG_skip_line (void) {
  while (feof(f) == 0 && ch != '\n' && ch != '\r') {
    ch = fgetc(f);
  }
  while (feof(f) == 0 && ch == '\n' && ch == '\r') {
    ch = fgetc(f);
  }
}

static int CFG_scan_iterator (char *key, int keylen, char *value, int valuelen) {
  assert(key != NULL);
  assert(keylen > 0);
  assert(value != NULL);
  assert(valuelen > 0);
  int i;
  int found = 0;
  while (feof(f) == 0 && found == 0) {
    CFG_skip_space();
    if (ch == ';') {
      CFG_skip_line();
    } else if (feof(f) == 0) {
      found = 1;
      i = 0;
      while (feof(f) == 0 && isspace(ch) == 0 && ch != '=') {
        if (i < keylen - 1) {
          key[i] = ch;
          i += 1;
        }
        ch = fgetc(f);
      }
      key[i] = 0;
      CFG_skip_space();
      if (feof(f) == 0 && ch == '=') {
        ch = fgetc(f);
        CFG_skip_space();
      }
      i = 0;
      while (feof(f) == 0 && ch != '\n' && ch != '\r') {
        if (i < valuelen - 1) {
          value[i] = ch;
          i += 1;
        }
        ch = fgetc(f);
      }
      value[i] = 0;
      CFG_skip_line();
    }
  }
  return found;
}

static void CFG_close_iterator (void) {
  assert(f != NULL);
  fclose(f);
  f = NULL;
}

static int CFG_update_key (const char *key, const char *value, int iscfg) {
//  logo("CFG_update_key: [%s] [%s] %i\n", key, value, iscfg);
  int i = 0;
  while (cfg[i].t != NONE && ((iscfg ? cfg[i].cfg : cfg[i].par) == NULL || strcasecmp(key, iscfg ? cfg[i].cfg : cfg[i].par) != 0)) {
    i += 1;
  }
  switch (cfg[i].t) {
    case BYTE: *(byte*)cfg[i].p = atoi(value); break;
    case WORD: *(word*)cfg[i].p = atoi(value); break;
    case DWORD: *(dword*)cfg[i].p = atoi(value); break;
    case STRING: cfg[i].p = strcpy(malloc(strlen(value) + 1), value); break;
    case SW_ON: *(byte*)cfg[i].p = strcasecmp(value, "on") == 0 ? 1 : 0; break;
    case SW_OFF: *(byte*)cfg[i].p = strcasecmp(value, "off") == 0 ? 0 : 1; break;
    case FILES: F_addwad(value); break;
    case KEY: *(int*)cfg[i].p = I_string_to_key(value); break;
    case NONE: return 0;
    default: assert(0); // unknown type -> something broken
  }
  return 1;
}

static int CFG_read_config (const char *name) {
  char key[64];
  char value[64];
  assert(name != NULL);
  if (CFG_open_iterator(name)) {
    while (CFG_scan_iterator(key, 64, value, 64)) {
      CFG_update_key(key, value, 1);
    }
    CFG_close_iterator();
    return 1;
  } else {
    return 0;
  }
}

void CFG_args (int argc, const char **argv) {
  int i;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] != 0) {
      if (i + 1 >= argc) {
        ERR_failinit("CFG_args: not enough arguments for parameter %s\n", argv[i]);
      } else {
        if (CFG_update_key(&argv[i][1], argv[i + 1], 0) != 0) {
          ERR_failinit("CFG_args: unknown parameter %s\n", argv[i]);
        }
        i += 1;
      }
    } else {
      ERR_failinit("CFG_args: something wrong here: %s\n", argv[i]);
    }
  }
}

void CFG_load (void) {
  if (CFG_read_config("default.cfg") == 0) {
    // TODO alt config at $HOME and system directories
  }
  CFG_read_config("user.cfg");
}

void CFG_save (void) {
  // TODO
}
