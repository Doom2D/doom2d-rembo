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

#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

#include "glob.h"

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

extern int sky_type;
extern dword walf[256];
extern byte fldb[FLDH][FLDW];
extern byte fldf[FLDH][FLDW];
extern byte fld[FLDH][FLDW];

void W_init (void);

#endif /* VIEW_H_INCLUDED */
