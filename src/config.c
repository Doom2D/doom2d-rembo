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
  const char *cfg;
  void *p;
  byte t;
} cfg_t;

byte cheat;
byte shot_vga;

static FILE *f;
static int ch;

static const cfg_t arg[] = {
  {"file", NULL, FILES},
  {"cheat", &cheat, SW_ON},
  {"vga", &shot_vga, SW_ON},
  {"sndvol", &snd_vol, WORD},
  {"musvol",&mus_vol, WORD},
//  {"fullscr", &fullscreen, SW_ON},
//  {"window", &fullscreen, SW_OFF},
  {"mon", &nomon, SW_OFF},
//  {"gamma", &gammaa, DWORD},
  {"warp", &_warp, BYTE},
  {"width", &SCRW, DWORD},
  {"height", &SCRH, DWORD},
//  {"config", NULL, cfg_file, STRING},
  {NULL, NULL, NONE} // end
};

static const cfg_t cfg[] = {
  {"screenshot", &shot_vga, SW_ON},
  {"sound_volume", &snd_vol, WORD},
  {"music_volume", &mus_vol, WORD},
//  {"fullscreen", &fullscreen, SW_ON},
  {"sky", &w_horiz, SW_ON},
//  {"gamma", &gammaa, DWORD},
  {"screen_width", &SCRW, DWORD},
  {"screen_height", &SCRH, DWORD},
  {"music_random", &music_random, SW_ON},
  {"music_time", &music_time, DWORD},
  {"music_fade", &music_fade, DWORD},
  {"pl1_left", &pl1.kl, KEY},
  {"pl1_right",&pl1.kr, KEY},
  {"pl1_up", &pl1.ku, KEY},
  {"pl1_down", &pl1.kd, KEY},
  {"pl1_jump", &pl1.kj, KEY},
  {"pl1_fire", &pl1.kf, KEY},
  {"pl1_next", &pl1.kwr, KEY},
  {"pl1_prev", &pl1.kwl, KEY},
  {"pl1_use", &pl1.kp, KEY},
  {"pl2_left", &pl2.kl, KEY},
  {"pl2_right",&pl2.kr, KEY},
  {"pl2_up",   &pl2.ku, KEY},
  {"pl2_down", &pl2.kd, KEY},
  {"pl2_jump", &pl2.kj, KEY},
  {"pl2_fire", &pl2.kf, KEY},
  {"pl2_next", &pl2.kwr, KEY},
  {"pl2_prev", &pl2.kwl, KEY},
  {"pl2_use",  &pl2.kp, KEY},
  {NULL, NULL, NONE} // end
};

static const cfg_t *CFG_find_entry (const char *key, const cfg_t *cfg) {
  assert(key != NULL);
  assert(cfg != NULL);
  int i = 0;
  while (cfg[i].cfg && strcasecmp(cfg[i].cfg, key) != 0) {
    i++;
  }
  return cfg[i].cfg ? &cfg[i] : NULL;
}

static int CFG_update_key (const char *key, const char *value, const cfg_t *cfg) {
  const cfg_t *entry = CFG_find_entry(key, cfg);
  if (entry != NULL) {
    void *p = entry->p;
    switch (entry->t) {
      case BYTE: *(byte*)p = atoi(value); break;
      case WORD: *(word*)p = atoi(value); break;
      case DWORD: *(dword*)p = atoi(value); break;
      case STRING: strcpy(p, value); break; // TODO fix this security problem
      case SW_ON: *(byte*)p = strcasecmp(value, "on") == 0 ? 1 : 0; break;
      case SW_OFF: *(byte*)p = strcasecmp(value, "off") == 0 ? 0 : 1; break;
      case FILES: F_addwad(value); break;
      case KEY: *(int*)p = I_string_to_key(value); break;
      default: assert(0); // unknown type -> something broken
    }
    return 1;
  } else {
    return 0;
  }
}

/* --- parser --- */

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

/* --- reader --- */

static int CFG_read_config (const char *name) {
  char key[64];
  char value[64];
  assert(name != NULL);
  if (CFG_open_iterator(name)) {
    while (CFG_scan_iterator(key, 64, value, 64)) {
      CFG_update_key(key, value, cfg);
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
        if (CFG_update_key(&argv[i][1], argv[i + 1], arg) != 0) {
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
  CFG_read_config("default.cfg");
  CFG_read_config("doom2d.cfg");
}

/* --- writer --- */

static void CFG_write_key_value (FILE *f, const char *key, const char *value) {
  assert(f != NULL);
  assert(key != NULL);
  assert(value != NULL);
  fwrite(key, strlen(key), 1, f);
  fwrite("=", 1, 1, f);
  fwrite(value, strlen(value), 1, f);
  fwrite("\n", 1, 1, f);
}

static int CFG_write_entry (FILE *f, const cfg_t *entry) {
  assert(f != NULL);
  assert(entry != NULL);
  char buf[64];
  const char *str;
  const char *key = entry->cfg;
  if (key != NULL) {
    switch (entry->t) {
      case BYTE:
        snprintf(buf, 64, "%i", *(byte*)entry->p);
        CFG_write_key_value(f, key, buf);
        break;
      case WORD:
        snprintf(buf, 64, "%i", *(word*)entry->p);
        CFG_write_key_value(f, key, buf);
        break;
      case DWORD:
        snprintf(buf, 64, "%i", *(dword*)entry->p);
        CFG_write_key_value(f, key, buf);
        break;
      case STRING:
        CFG_write_key_value(f, key, entry->p);
        break;
      case SW_ON:
      case SW_OFF:
        str = *(byte*)entry->p ? "on" : "off";
        CFG_write_key_value(f, key, str);
        break;
      case KEY:
        str = I_key_to_string(*(int*)entry->p);
        CFG_write_key_value(f, key, str);
        break;
      case FILES: return 1; // ignore
      case NONE: return 0; // end
      default: assert(0); // unknown type -> something broken
    }
  }
  return entry->t == NONE ? 0 : 1;
}

static int CFG_update_config (const char *old, const char *new, const cfg_t *cfg, const char *msg) {
  assert(old != NULL);
  assert(new != NULL);
  assert(cfg != NULL);
  char key[64];
  char value[64];
  FILE *nf = fopen(new, "wb");
  if (nf != NULL) {
    if (msg != NULL) {
      fwrite("; ", 2, 1, nf);
      fwrite(msg, strlen(msg), 1, nf);
      fwrite("\n", 1, 1, nf);
    }
    if (CFG_open_iterator(old)) {
      while (CFG_scan_iterator(key, 64, value, 64)) {
        if (CFG_find_entry(key, cfg) == NULL) {
          CFG_write_key_value(nf, key, value);
        }
      }
      CFG_close_iterator();
    }
    int i = 0;
    while (CFG_write_entry(nf, &cfg[i])) {
      i++;
    }
    fclose(nf);
  }
  return nf != NULL;
}

void CFG_save (void) {
  CFG_update_config("doom2d.cfg", "doom2d.cfg", cfg, "generated by doom2d, do not modify");
  //CFG_update_config("doom2d.cfg", "doom2d.tmp", cfg, "temporary file");
  //CFG_update_config("doom2d.tmp", "doom2d.cfg", cfg, "generated by doom2d, do not modify");
  //remove("doom2d.tmp");
}
