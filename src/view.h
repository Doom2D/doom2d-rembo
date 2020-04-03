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

#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

#include "glob.h"
#include <stdio.h> // FILE

#define FLDW 100
#define FLDH 100
#define CELW 8
#define CELH 8
#define MAXTXW 16
#define MAXTXH 8

enum {
  HIT_SOME, HIT_ROCKET, HIT_BFG, HIT_TRAP, HIT_WATER, HIT_ELECTRO, HIT_FLAME
};

enum {
  GS_TITLE, GS_GAME, GS_INTER, GS_DARKEN, GS_ENDANIM, GS_END2ANIM, GS_ENDSCR,
  GS_BVIDEO, GS_EVIDEO, GS_END3ANIM
};

typedef struct {
  int x, y;		// coordinates
  int xv, yv;		// velocity
  int vx, vy;
  int r, h;		// radius, height
} obj_t;

typedef struct {
  int x, y;
  byte d;
} pos_t;

typedef struct {
  char n[8];
  char t;
} wall_t;

extern byte w_horiz;
extern int sky_type;
extern dword walf[256];
extern byte walswp[256];
extern byte fldb[FLDH][FLDW];
extern byte fldf[FLDH][FLDW];
extern byte fld[FLDH][FLDW];

void W_savegame (FILE *h);
void W_loadgame (FILE *h);
void W_init (void);
int W_load (FILE *h);

#endif /* VIEW_H_INCLUDED */
