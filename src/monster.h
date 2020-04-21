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

#ifndef MONSTER_H_INLUDED
#define MONSTER_H_INLUDED

#include "glob.h"
#include "view.h" // obj_t

#define MAXMN 200

enum {
  MN_NONE, MN_DEMON, MN_IMP, MN_ZOMBY, MN_SERG, MN_CYBER, MN_CGUN,
  MN_BARON, MN_KNIGHT, MN_CACO, MN_SOUL, MN_PAIN, MN_SPIDER, MN_BSP,
  MN_MANCUB, MN_SKEL, MN_VILE, MN_FISH, MN_BARREL, MN_ROBO, MN_MAN,
  MN__LAST,
  MN_PL_DEAD = 100,
  MN_PL_MESS
};

#define MN_TN (MN__LAST-MN_DEMON)

typedef struct {
  obj_t o;
  byte t, d, st, ftime;
  int fobj;
  int s;
  char *ap;
  int aim, life, pain, ac, tx, ty, ammo;
  short atm;
} mn_t;

extern byte nomon;
extern int hit_xv, hit_yv;
extern mn_t mn[MAXMN];
extern int mnum, gsndt;

void setst (int i, int st);

void MN_alloc (void);
void MN_init (void);
int MN_spawn (int x, int y, byte d, int t);
int MN_spawn_deadpl (obj_t *o, byte c, int t);
int Z_getobjpos (int i, obj_t *o);
void MN_act (void);
void MN_mark (void);
int MN_hit (int n, int d, int o, int t);
int Z_gunhit (int x, int y, int o, int xv, int yv);
int Z_hit (obj_t *o, int d, int own, int t);
void MN_killedp (void);
void Z_explode (int x, int y, int rad, int own);
void Z_bfg9000 (int x, int y, int own);
int Z_chktrap (int t, int d, int o, int ht);
void Z_teleobj (int o, int x, int y);
void MN_warning (int l, int t, int r, int b);

#endif /* MONSTER_H_INLUDED */
