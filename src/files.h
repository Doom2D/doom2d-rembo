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

#ifndef FILES_H_INCLUDED
#define FILES_H_INCLUDED

#include "glob.h"
#include <stdio.h> // FILE
#include "map.h" // map_block_t

#define MAX_WADS 20
#define MAX_WAD 2000

typedef char wadname[8];

typedef struct {
  int o, l;
  char n[8];
} wad_t;

typedef struct {
  int o, l;
  char n[8];
  int f;
} mwad_t;

extern int d_start, d_end;
extern mwad_t wad[MAX_WAD];
extern char wads[MAX_WADS][__MAX_PATH];
extern FILE* wadh[MAX_WADS];

void F_startup (void);
void F_addwad (const char *fn);
void F_initwads (void);
void F_allocres (void);
void F_loadres (int r, void *p, dword o, dword l);
int F_findres (const char n[8]);
int F_getresid (const char n[8]);
void F_getresname (char n[8], int r);
int F_getsprid (const char n[4], int s, int d, char *dir);
int F_getreslen (int r);
void F_nextmus (char *s);
void F_randmus (char *s);
void F_readstr (FILE* h, char *s, int m);

#endif /* FILES_H_INCLUDED */
