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

#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include "glob.h"

typedef struct {
  byte type;
  int n, cur, x;
  char *ttl;
  char **m;
  byte *t;
} menu_t;

extern menu_t save_mnu;
extern byte *panimp;
extern byte _warp;

#define PCOLORN 10
extern byte pcolortab[PCOLORN];
extern int p1color;
extern int p2color;

extern char ibuf[24];
extern byte input;

extern menu_t *mnu;
extern byte gm_redraw;
extern short lastkey;

void GMV_say (const char nm[8]);
void G_keyf (int key, int down);
void G_code (void);
int GM_act (void);
void GM_init (void);

#endif /* MENU_H_INCLUDED */
