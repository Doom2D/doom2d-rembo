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

#ifndef SAVE_H_INCLUDED
#define SAVE_H_INCLUDED

#include "common/streams.h"

#define SAVE_MAX 7
#define SAVE_MAXLEN 24

extern char savname[SAVE_MAX][SAVE_MAXLEN];
extern char savok[SAVE_MAX];

void F_getsavnames (void);
void F_savegame (int n, char *s);
void F_loadgame (int n);

void SAVE_save (Stream *w, const char name[24]);
void SAVE_load (Stream *r);

#endif /* SAVE_H_INCLUDED */
