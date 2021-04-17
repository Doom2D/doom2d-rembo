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

#define SAVE_MAX 7
#define SAVE_MAXLEN 24

extern char savname[SAVE_MAX][SAVE_MAXLEN];
extern char savok[SAVE_MAX];
extern int d_start, d_end;

void F_addwad (const char *fn);
void F_initwads (void);
int F_findres (const char n[8]);
int F_getresid (const char n[8]);
void F_getresname (char n[8], int r);
int F_getreslen (int r);

// Get sprite resource id.
// Sprite name has following format:
//  (nnnn)('A'+s)('0'+d)[('A'+s)('0'+d)]
//  Letter means animation frame
//    A for first, B for second...
//  Number means direction
//    0 = front
//    1 = left
//    2 = right
//  Optional part means that this file can be used for differnt frame/direction.
//  Note that if found FRONT direction for this frame than it UNCONDITIONALLY used.
//  Note that search performed between markers S_START and S_END in order as paced in wad.
//  int n[4]  -- sprite name
//  int s     -- sprite frame
//  int d     -- sprite direction
//  char *dir -- out flag "alternative used"
int F_getsprid (const char n[4], int s, int d, char *dir);

// void F_nextmus (char *s);
// void F_randmus (char *s);

void F_loadmap (char n[8]);

void F_getsavnames (void);
void F_savegame (int n, char *s);
void F_loadgame (int n);

#endif /* FILES_H_INCLUDED */
