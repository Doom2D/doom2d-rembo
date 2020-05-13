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

#ifndef ITEMS_H_INCLUDED
#define ITEMS_H_INCLUDED

#include "view.h" // obj_t

#define MAXITEM 300

enum {
  I_NONE, I_CLIP, I_SHEL, I_ROCKET, I_CELL, I_AMMO, I_SBOX, I_RBOX, I_CELP,
  I_STIM, I_MEDI, I_BPACK, I_CSAW, I_SGUN, I_SGUN2, I_MGUN, I_LAUN, I_PLAS, I_BFG,
  I_ARM1, I_ARM2, I_MEGA, I_INVL, I_AQUA, I_KEYR, I_KEYG, I_KEYB, I_SUIT, I_SUPER,
  I_RTORCH, I_GTORCH, I_BTORCH, I_GOR1, I_FCAN, I_GUN2
};

typedef struct {
  obj_t o;
  int t;
  int s;
} item_t;

extern item_t it[MAXITEM];
extern int itm_rtime;

void IT_alloc (void);
void IT_init (void);
void IT_act (void);
void IT_spawn (int x, int y, int t);
void IT_drop_ammo (int t, int n, int x, int y);

#endif /* ITEMS_H_INCLUDED */
