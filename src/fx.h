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

#ifndef FX_H_INLUDED
#define FX_H_INLUDED

#define MAXFX 300

typedef struct {
  int x, y, xv, yv;
  char t, s;
} fx_t;

extern fx_t fx[MAXFX];

void FX_alloc (void);
void FX_init (void);
void FX_act (void);
void FX_tfog (int x, int y);
void FX_ifog (int x, int y);
void FX_bubble (int x, int y, int xv, int yv, int n);

#endif /* FX_H_INLUDED */
