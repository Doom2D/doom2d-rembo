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
#include "system.h"
#include "files.h"
#include "input.h"

static FILE *f;
static int ch;

const cfg_t *CFG_find_entry (const char *key, const cfg_t *cfg) {
  assert(key != NULL);
  assert(cfg != NULL);
  int i = 0;
  while (cfg[i].cfg && strcasecmp(cfg[i].cfg, key) != 0) {
    i++;
  }
  return cfg[i].cfg ? &cfg[i] : NULL;
}

int CFG_update_key (const char *key, const char *value, const cfg_t *cfg) {
  const cfg_t *entry = CFG_find_entry(key, cfg);
  if (entry != NULL) {
    void *p = entry->p;
    switch (entry->t) {
      case Y_BYTE: *(byte*)p = atoi(value); break;
      case Y_WORD: *(word*)p = atoi(value); break;
      case Y_DWORD: *(dword*)p = atoi(value); break;
      case Y_STRING: strcpy(p, value); break; // TODO fix this security problem
      case Y_SW_ON: *(byte*)p = strcasecmp(value, "on") == 0 ? 1 : 0; break;
      case Y_SW_OFF: *(byte*)p = strcasecmp(value, "off") == 0 ? 0 : 1; break;
      case Y_FILES: F_addwad(value); break;
      case Y_KEY: *(int*)p = I_string_to_key(value); break;
      default: assert(0); // unknown type -> something broken
    }
    return 1;
  } else {
    return 0;
  }
}

/* --- parser --- */

int CFG_open_iterator (const char *name) {
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

int CFG_scan_iterator (char *key, int keylen, char *value, int valuelen) {
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

void CFG_close_iterator (void) {
  assert(f != NULL);
  fclose(f);
  f = NULL;
}

/* --- reader --- */

int CFG_read_config (const char *name, const cfg_t *cfg) {
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
      case Y_BYTE:
        snprintf(buf, 64, "%i", *(byte*)entry->p);
        CFG_write_key_value(f, key, buf);
        break;
      case Y_WORD:
        snprintf(buf, 64, "%i", *(word*)entry->p);
        CFG_write_key_value(f, key, buf);
        break;
      case Y_DWORD:
        snprintf(buf, 64, "%i", *(dword*)entry->p);
        CFG_write_key_value(f, key, buf);
        break;
      case Y_STRING:
        CFG_write_key_value(f, key, entry->p);
        break;
      case Y_SW_ON:
      case Y_SW_OFF:
        str = *(byte*)entry->p ? "on" : "off";
        CFG_write_key_value(f, key, str);
        break;
      case Y_KEY:
        str = I_key_to_string(*(int*)entry->p);
        CFG_write_key_value(f, key, str);
        break;
      case Y_FILES: return 1; // ignore
      case 0: return 0; // end
      default: assert(0); // unknown type -> something broken
    }
  }
  return entry->t == 0 ? 0 : 1;
}

int CFG_update_config (const char *old, const char *new, const cfg_t *cfg, const char *msg) {
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
