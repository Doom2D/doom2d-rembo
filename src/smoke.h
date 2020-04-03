/*
   Copyright (C) Prikol Software 1996-1997
   Copyright (C) Aleksey Volynskov 1996-1997

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

#ifndef SMOKE_H_INLUDED
#define SMOKE_H_INLUDED

#include "glob.h"
#include <stdio.h> // FILE

#define MAXSMOK 500

#define SMSN 10
#define FLSN 8

typedef struct {
  int x, y, xv, yv;
  byte t, s;
  short o;
} smoke_t;

extern smoke_t sm[MAXSMOK];

void SMK_savegame (FILE *h);
void SMK_loadgame (FILE *h);
void SMK_init (void);
void SMK_alloc (void);
void SMK_act (void);
void SMK_gas (int x0, int y0, int xr, int yr, int xv, int yv, int k);
void SMK_flame (int x0, int y0, int ox, int oy, int xr, int yr, int xv, int yv, int k, int o);

#endif /* SMOKE_H_INLUDED */
