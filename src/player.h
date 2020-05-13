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

#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "glob.h"
#include "view.h" // obj_t

#define PL_DRAWLIFE  1
#define PL_DRAWARMOR 2
#define PL_DRAWWPN   4
#define PL_DRAWFRAG  8
#define PL_DRAWAIR   16
#define PL_DRAWKEYS  32
#define PL_DRAWLIVES 64

#define PLK_U  1
#define PLK_D  2
#define PLK_L  4
#define PLK_R  8
#define PLK_F  16
#define PLK_J  32
#define PLK_P  64
#define PLK_W  128
#define PLK_WL 1
#define PLK_WR 2

#define PLF_FIRE  1
#define PLF_PNSND 2
#define PLF_UP    4
#define PLF_DOWN  8

#define PL_AIR 360
#define PL_POWERUP_TIME 546

typedef struct {
  obj_t o;
  int looky;
  int st, s;
  int life, armor, hit, hito;
  int pain, air;
  int invl, suit;
  char d;
  int frag, ammo, shel, rock, cell, fuel, kills, secrets;
  byte fire, cwpn, csnd;
  byte amul;
  word wpns;
  char wpn;
  byte f;
  byte drawst;
  byte color;
  int id;
  byte keys;
  char lives;
  int ku, kd, kl, kr, kf, kj, kwl, kwr, kp;
} player_t;

extern byte p_immortal;
extern byte p_fly;
extern int PL_JUMP;
extern int PL_RUN;

extern player_t pl1;
extern player_t pl2;

extern byte plr_goanim[];
extern byte plr_dieanim[];
extern byte plr_slopanim[];

int PL_isdead (player_t *p);
void PL_init (void);
void PL_alloc (void);
void PL_reset (void);
void PL_spawn (player_t *p, int x, int y, char d);
int PL_hit (player_t *p, int d, int o, int t);
void PL_damage (player_t *p);
void PL_cry (player_t *p);
int PL_give (player_t *p, int t);
void PL_act (player_t *p);
void bfg_fly (int x, int y, int o);

#endif /* PLAYER_H_INCLUDED */
