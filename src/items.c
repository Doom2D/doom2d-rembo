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

extern map_block_t blk;

#pragma pack(1)
typedef struct{
  obj_t o;
  int t;
  int s;
}item_t;
#pragma pack()

static void *snd[4],*spr[58];
static char sprd[58];
static int tsndtm,rsndtm;
static item_t it[MAXITEM];

int itm_rtime=1092;

void IT_savegame(FILE* h) {
  int n;

  for(n=MAXITEM;--n;) if(it[n].t) break;
  ++n;myfwrite(&n,1,4,h);
  myfwrite(it,1,n*sizeof(it[0]),h);
  myfwrite(&itm_rtime,1,4,h);
}

void IT_loadgame(FILE* h) {
  int n;

  myfread(&n,1,4,h);
  myfread(it,1,n*sizeof(it[0]),h);
  myfread(&itm_rtime,1,4,h);
}

void IT_alloc(void) {
  int i,j,n;
  static char nm[][6]={
    "ITEMUP","WPNUP","GETPOW","ITMBK"
  },snm[][4]={
	"CLIP","SHEL","ROCK","CELL","AMMO","SBOX","BROK","CELP",
	"STIM","MEDI","BPAK",
	"CSAW","SHOT","SGN2","MGUN","LAUN","PLAS","BFUG"
  },n4[][4]={
    "SOUL","SMRT","SMGT","SMBT"
  },n3[][4]={
    "GOR1","FCAN"
  };

//  logo("  items");
  for(i=0;i<18;++i) spr[i]=Z_getspr(snm[i],0,0,sprd+i);
  for(;i<20;++i) {
	spr[i]=Z_getspr("ARM1",i-18,0,sprd+i);
	spr[i+2]=Z_getspr("ARM2",i-18,0,sprd+i);
  }i+=2;
  for(;i<26;++i) spr[i]=Z_getspr("MEGA",i-22,0,sprd+i);
  for(;i<30;++i) spr[i]=Z_getspr("PINV",i-26,0,sprd+i);
  spr[30]=Z_getspr("AQUA",0,0,sprd+30);
  spr[31]=Z_getspr("KEYR",0,0,sprd+31);
  spr[32]=Z_getspr("KEYG",0,0,sprd+32);
  spr[33]=Z_getspr("KEYB",0,0,sprd+33);
  spr[34]=Z_getspr("SUIT",0,0,sprd+34);
  for(n=35,j=0;j<4;++j)
    for(i=0;i<4;++i,++n) spr[n]=Z_getspr(n4[j],i,0,sprd+n);
  for(j=0;j<2;++j)
    for(i=0;i<3;++i,++n) spr[n]=Z_getspr(n3[j],i,0,sprd+n);
  spr[57]=Z_getspr("GUN2",0,0,sprd+57);
  for(i=0;i<4;++i) snd[i]=Z_getsnd(nm[i]);
  for(i=0;i<MAXITEM;++i) {it[i].o.r=10;it[i].o.h=8;}
}

void IT_init(void) {
  int i;

  for(i=0;i<MAXITEM;++i) {
	it[i].t=I_NONE;
	it[i].o.xv=it[i].o.yv=it[i].o.vx=it[i].o.vy=0;
  }
  tsndtm=rsndtm=0;
}

int IT_load(FILE* h) {
  int m,i,j;
  old_thing_t t;

  switch(blk.t) {
	case MB_THING:
	  for(i=0;blk.sz>0;++i,blk.sz-=8) {
		myfread(&t,1,sizeof(t),h);
		it[i].o.x=t.x;it[i].o.y=t.y;
		it[i].t=t.t;it[i].s=t.f;
		if(!it[i].t) break;
		if((it[i].s&THF_DM) && !g_dm) it[i].t=0;
	  }m=i;
	  for(i=0,j=-1;i<m;++i) if(it[i].t==TH_PLR1) {j=i;it[i].t=0;}
	  if(!g_dm) {
		if(j==-1) ERR_fatal("Предмет игрок_1 не найден");
		dm_pos[0].x=it[j].o.x;dm_pos[0].y=it[j].o.y;dm_pos[0].d=it[j].s&THF_DIR;
	  }
	  for(i=0,j=-1;i<m;++i) if(it[i].t==TH_PLR2) {j=i;it[i].t=0;}
	  if(!g_dm && _2pl) {
		if(j==-1) ERR_fatal("Предмет игрок_2 не найден");
		dm_pos[1].x=it[j].o.x;dm_pos[1].y=it[j].o.y;dm_pos[1].d=it[j].s&THF_DIR;
	  }
	  for(i=0,j=0;i<m;++i) if(it[i].t==TH_DMSTART) {
		if(g_dm)
		  {dm_pos[j].x=it[i].o.x;dm_pos[j].y=it[i].o.y;dm_pos[j].d=it[i].s&THF_DIR;}
		it[i].t=0;++j;
	  }
	  if(g_dm && j<2) ERR_fatal("Меньше 2-ух точек DM");
	  if(g_dm) {
	    dm_pnum=j;
	    dm_pl1p=myrand(dm_pnum);
	    do{ dm_pl2p=myrand(dm_pnum); }while(dm_pl2p==dm_pl1p);
	  }else {dm_pl1p=0;dm_pl2p=1;dm_pnum=2;}
	  PL_spawn(&pl1,dm_pos[dm_pl1p].x,dm_pos[dm_pl1p].y,dm_pos[dm_pl1p].d);
	  if(_2pl) PL_spawn(&pl2,dm_pos[dm_pl2p].x,dm_pos[dm_pl2p].y,dm_pos[dm_pl2p].d);
	  for(i=0;i<m;++i)
		if(it[i].t>=TH_CLIP && it[i].t<TH_DEMON) {
		  it[i].s=0;it[i].t=it[i].t-TH_CLIP+I_CLIP;
		  if(it[i].t>=I_KEYR && it[i].t<=I_KEYB) it[i].t|=0x8000;
		}else if(it[i].t>=TH_DEMON) {
		  MN_spawn(it[i].o.x,it[i].o.y,it[i].s&THF_DIR,it[i].t-TH_DEMON+MN_DEMON);
		  it[i].t=0;
		}
	  return 1;
  }return 0;
}

static void takesnd(int t) {
  if(tsndtm) return;
  t&=0x7FFF;
  if(t<=I_CELP || (t>=I_BPACK && t<=I_BFG) || t==I_GUN2)
    {tsndtm=Z_sound(snd[1],128);return;}
  if(t==I_MEGA || t==I_INVL || t==I_SUPER)
    {tsndtm=Z_sound(snd[2],192);return;}
  tsndtm=Z_sound(snd[0],256);
}

void IT_act(void) {
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

void IT_draw(void) {
  int i,s;

  for(i=0;i<MAXITEM;++i) {
    s=-1;
    if(it[i].t && it[i].s>=0) switch(it[i].t&0x7FFF) {
          case I_ARM1:
		s=it[i].s/9+18;break;
	  case I_ARM2:
		s=it[i].s/9+20;break;
	  case I_MEGA:
		s=it[i].s/2+22;break;
	  case I_INVL:
		s=it[i].s/2+26;break;
	  case I_SUPER: case I_RTORCH: case I_GTORCH: case I_BTORCH:
		s=it[i].s/2+(it[i].t-I_SUPER)*4+35;break;
	  case I_GOR1: case I_FCAN:
		s=it[i].s/2+(it[i].t-I_GOR1)*3+51;break;
	  case I_AQUA: s=30;break;
	  case I_SUIT: s=34;break;
	  case I_KEYR: case I_KEYG: case I_KEYB:
	    s=(it[i].t&0x7FFF)-I_KEYR+31;break;
	  case I_GUN2: s=57;break;
	  default:
		s=(it[i].t&0x7FFF)-1;
    }
	if(s>=0) Z_drawspr(it[i].o.x,it[i].o.y,spr[s],sprd[s]);
  }
}

void IT_spawn(int x,int y,int t) {
  int i;

  for(i=0;i<MAXITEM;++i) if(!it[i].t) {
	it[i].t=t|0x8000;it[i].s=0;
    it[i].o.x=x;it[i].o.y=y;
    it[i].o.xv=it[i].o.yv=it[i].o.vx=it[i].o.vy=0;
    it[i].o.r=10;it[i].o.h=8;
    return;
  }
}

void IT_drop_ammo(int t,int n,int x,int y) {
  static int an[8]={10,4,1,40,50,25,5,100};
  int a;

again:;
  for(a=an[t-I_CLIP];n>=a;n-=a)
	IT_spawn(x+myrand(3*2+1)-3,y-myrand(7),t);
  if(t>=I_AMMO) {t-=4;goto again;}
}
