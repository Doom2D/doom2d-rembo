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

#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include "glob.h"
#include "view.h" // obj_t
#include <stdint.h> // int16_t int32_t

#define MAXDIST 2000000L

enum {
  Z_HITWALL = 1,
  Z_HITCEIL = 2,
  Z_HITLAND = 4,
  Z_FALLOUT = 8,
  Z_INWATER = 16,
  Z_HITWATER = 32,
  Z_HITAIR = 64,
  Z_BLOCK = 128
};

extern byte z_dot;
extern byte z_mon;

int Z_sign (int a);
int Z_dec (int a, int b);
void *Z_getsnd (char n[6]);
int Z_sound (void *s, int v);
void Z_initst (void);
int Z_canstand (int x, int y, int r);
int Z_canfit (int x, int y, int r, int h);
int Z_istrapped (int x, int y, int r, int h);
void Z_set_speed (obj_t *o, int s);
int Z_inwater (int x, int y, int r, int h);
int Z_getacid (int x, int y, int r, int h);
int Z_canbreathe (int x, int y, int r, int h);
int Z_overlap (obj_t *a, obj_t *b);
int Z_cansee (int x, int y, int xd, int yd);
int Z_look (obj_t *a, obj_t *b, int d);
int Z_moveobj (obj_t *p);
void Z_splash (obj_t *p, int n);
void Z_calc_time(dword t, word *h, word *m, word *s);

#endif /* MISC_H_INCLUDED */
