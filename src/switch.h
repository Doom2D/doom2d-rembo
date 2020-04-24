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

#ifndef SWITCH_H_INCLUDED
#define SWITCH_H_INCLUDED

#include "view.h" // obj_t

#define MAXSW 100

enum {
  SW_NONE, SW_EXIT, SW_EXITS, SW_OPENDOOR, SW_SHUTDOOR, SW_SHUTTRAP,
  SW_DOOR, SW_DOOR5, SW_PRESS, SW_TELE, SW_SECRET, SW_LIFTUP, SW_LIFTDOWN,
  SW_TRAP, SW_LIFT
};

typedef struct {
  byte x, y;
  byte t, tm;
  byte a, b, c, d;
  byte f;
} sw_t;

extern int sw_secrets;
extern sw_t sw[MAXSW];

void SW_alloc (void);
void SW_init (void);
void Z_water_trap (obj_t *o);
void Z_untrap (byte t);
void SW_act (void);
void SW_cheat_open (void);
int SW_press (int x, int y, int r, int h, byte t, int o);

#endif /* SWITCH_H_INCLUDED */
