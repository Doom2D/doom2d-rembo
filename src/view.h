/*
   Copyright (C) Prikol Software 1996-1997
   Copyright (C) Aleksey Volynskov 1996-1997
   Copyright (C) <ARembo@gmail.com> 2011

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

// View functions

#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

#include <stdio.h>


// object data structure
#pragma pack(1)
typedef struct{
  int x,y;		// coordinates
  int xv,yv;		// velocity
  int vx,vy;
  int r,h;		// radius, height
}obj_t;


typedef struct{
  int x,y;
  byte d;
}pos_t;
#pragma pack()

enum{HIT_SOME,HIT_ROCKET,HIT_BFG,HIT_TRAP,HIT_WATER,HIT_ELECTRO,HIT_FLAME};

enum{
  GS_TITLE,GS_GAME,GS_INTER,GS_DARKEN,GS_ENDANIM,GS_END2ANIM,GS_ENDSCR,
  GS_BVIDEO,GS_EVIDEO,GS_END3ANIM
};

#define FLDW 100
#define FLDH 100
#define CELW 8
#define CELH 8
#define MAXTXW 16
#define MAXTXH 8

#pragma pack(1)

typedef struct{
  char n[8];
  char t;
}wall_t;

#pragma pack()

void W_draw(void);
void W_act(void);

void W_init(void);

int W_load(FILE*);
int G_load(FILE*);

void G_init(void);
void G_start(void);
void G_act(void);
void G_draw(void);

extern byte walswp[256];
extern byte _2pl,g_dm,g_st,g_exit,w_horiz,g_map;
extern int g_sttm;
extern dword g_time;
extern int w_o,w_x,w_y;
extern byte fldb[FLDH][FLDW];
extern byte fldf[FLDH][FLDW];
extern byte fld[FLDH][FLDW];
extern pos_t dm_pos[];
extern int dm_pnum,dm_pl1p,dm_pl2p;

extern int WD; //
extern int HT; //

#endif /* VIEW_H_INCLUDED */