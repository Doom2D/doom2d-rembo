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

#ifndef DOTS_H_INCLUDED
#define DOTS_H_INCLUDED

#include "glob.h"
#include "view.h" // obj_t

#define MAXDOT 400

typedef struct {
  obj_t o;
  byte c, t;
} dot_t;

extern dot_t dot[MAXDOT];

void DOT_init (void);
void DOT_alloc (void);
void DOT_act (void);
void DOT_add (int x, int y, char xv, char yv, byte c, byte t);
void DOT_blood (int x, int y, int xv, int yv, int n);
void DOT_spark (int x, int y, int xv, int yv, int n);
void DOT_water (int x, int y, int xv, int yv, int n, int c);

#endif /* DOTS_H_INCLUDED */
