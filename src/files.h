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

extern int d_start, d_end;

void F_startup (void);
void F_addwad (const char *fn);
void F_initwads (void);
void F_allocres (void);
void F_loadres (int r, void *p);
int F_findres (const char n[8]);
int F_getresid (const char n[8]);
void F_getresname (char n[8], int r);
int F_getsprid (const char n[4], int s, int d, char *dir);
int F_getreslen (int r);
void F_nextmus (char *s);
void F_randmus (char *s);

void F_loadmap (char n[8]);

#endif /* FILES_H_INCLUDED */
