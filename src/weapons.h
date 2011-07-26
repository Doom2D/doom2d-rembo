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

// Weapons

#define MAXWPN 300

void WP_init(void);
void WP_alloc(void);
void WP_act(void);
void WP_draw(void);
void WP_punch(int,int,int,int);
int WP_chainsaw(int,int,int,int);
void WP_gun(int,int,int,int,int,int);
void WP_pistol(int,int,int,int,int);
void WP_mgun(int,int,int,int,int);
void WP_rocket(int,int,int,int,int);
void WP_revf(int,int,int,int,int,int);
void WP_plasma(int,int,int,int,int);
void WP_ball1(int,int,int,int,int);
void WP_ball2(int,int,int,int,int);
void WP_ball7(int,int,int,int,int);
void WP_aplasma(int,int,int,int,int);
void WP_manfire(int,int,int,int,int);
void WP_bfgshot(int,int,int,int,int);
void WP_bfghit(int,int,int);
void WP_shotgun(int,int,int,int,int);
void WP_dshotgun(int,int,int,int,int);
void WP_ognemet(int x,int y,int xd,int yd,int xv,int yv,int o);

void WP_grenade(int x, int y, int xd, int yd, int o);
