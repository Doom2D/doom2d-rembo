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

#include "glob.h"
#include <stdlib.h>
#include "error.h"
#include "view.h"
#include "items.h"
#include "fx.h"
#include "player.h"
#include "monster.h"
#include "things.h"
#include "misc.h"
#include "map.h"
#include "files.h"
#include "game.h"

item_t it[MAXITEM];

static void *snd[4];
static int tsndtm, rsndtm;

int itm_rtime = 1092;

void IT_alloc (void) {
  int i, j, n;
  static char nm[][6] = {
    "ITEMUP", "WPNUP", "GETPOW", "ITMBK"
  };
  for (i = 0; i < 4; ++i) {
    snd[i] = Z_getsnd(nm[i]);
  }
  for (i = 0; i < MAXITEM; ++i) {
    it[i].o.r = 10;
    it[i].o.h = 8;
  }
}

void IT_init (void) {
  int i;
  for (i = 0; i < MAXITEM; ++i) {
    it[i].t = I_NONE;
    it[i].o.xv = 0;
    it[i].o.yv = 0;
    it[i].o.vx = 0;
    it[i].o.vy = 0;
  }
  tsndtm = 0;
  rsndtm = 0;
}

static void takesnd (int t) {
  if(tsndtm) return;
  t&=0x7FFF;
  if(t<=I_CELP || (t>=I_BPACK && t<=I_BFG) || t==I_GUN2)
    {tsndtm=Z_sound(snd[1],128);return;}
  if(t==I_MEGA || t==I_INVL || t==I_SUPER)
    {tsndtm=Z_sound(snd[2],192);return;}
  tsndtm=Z_sound(snd[0], 255);
}

void IT_act (void) {
  int i,j;

  if(tsndtm) --tsndtm;
  if(rsndtm) --rsndtm;
  for(i=0;i<MAXITEM;++i) if(it[i].t)
    if(it[i].s<0) {
      if(++it[i].s==-8) {
		FX_ifog(it[i].o.x,it[i].o.y);
		if(!rsndtm) rsndtm=Z_sound(snd[3],128);
	  }
	}else{
	  switch(it[i].t) {
		case I_ARM1: case I_ARM2:
		  if(++it[i].s>=18) it[i].s=0; break;
		case I_MEGA: case I_INVL:
		case I_SUPER: case I_RTORCH: case I_GTORCH: case I_BTORCH:
		  if(++it[i].s>=8) it[i].s=0; break;
		case I_GOR1: case I_FCAN:
		  if(++it[i].s>=6) it[i].s=0; break;
      }
	  if(it[i].t&0x8000) {
		if((j=Z_moveobj(&it[i].o))&Z_FALLOUT) {it[i].t=0;continue;}
		else if(j&Z_HITWATER) Z_splash(&it[i].o,it[i].o.r+it[i].o.h);
	  }
      if(Z_overlap(&it[i].o,&pl1.o))
		if(PL_give(&pl1,it[i].t&0x7FFF)) {
		  takesnd(it[i].t);
		  if(_2pl) if((it[i].t&0x7FFF)>=I_KEYR && (it[i].t&0x7FFF)<=I_KEYB) continue;
		  if(!(it[i].s=-itm_rtime) || (it[i].t&0x8000)) it[i].t=0;
		  continue;
		}
	  if(_2pl) if(Z_overlap(&it[i].o,&pl2.o))
		if(PL_give(&pl2,it[i].t&0x7FFF)) {
		  takesnd(it[i].t);
		  if((it[i].t&0x7FFF)>=I_KEYR && (it[i].t&0x7FFF)<=I_KEYB) continue;
		  if(!(it[i].s=-itm_rtime) || (it[i].t&0x8000)) it[i].t=0;
		  continue;
		}
	}
}

void IT_spawn (int x,int y,int t) {
  int i;

  for(i=0;i<MAXITEM;++i) if(!it[i].t) {
	it[i].t=t|0x8000;it[i].s=0;
    it[i].o.x=x;it[i].o.y=y;
    it[i].o.xv=it[i].o.yv=it[i].o.vx=it[i].o.vy=0;
    it[i].o.r=10;it[i].o.h=8;
    return;
  }
}

void IT_drop_ammo (int t, int n, int x, int y) {
  static int an[8]={10,4,1,40,50,25,5,100};
  int a;

again:;
  for(a=an[t-I_CLIP];n>=a;n-=a)
	IT_spawn(x+myrand(3*2+1)-3,y-myrand(7),t);
  if(t>=I_AMMO) {t-=4;goto again;}
}
