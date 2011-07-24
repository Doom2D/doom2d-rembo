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
//#include <stdio.h>
#include <stdlib.h>
#include "files.h"
#include "memory.h"
#include "vga.h"
#include "error.h"
#include "keyb.h"
#include "sound.h"
#include "view.h"
#include "dots.h"
#include "misc.h"

#define MAXINI 50
#define MAXSR 20

#define BL_XV 4
#define BL_YV 4
#define BL_MINT 10
#define BL_MAXT 14

#define SP_V 2
#define SP_MINT 5
#define SP_MAXT 7

extern byte z_dot;

#pragma pack(1)
typedef struct{
  obj_t o;
  byte c,t;
}dot_t;
#pragma pack()

typedef struct{
  int xv,yv;
  byte c,t;
}init_t;

static dot_t dot[MAXDOT];
static init_t bl_ini[MAXINI],sp_ini[MAXINI];
static int bl_r,sp_r,sr_r,sxr[MAXSR],syr[MAXSR];
static int ldot;

void DOT_savegame(FILE* h) {
  int i,n;

  for(i=n=0;i<MAXDOT;++i) if(dot[i].t) ++n;
  myfwrite(&n,1,4,h);
  for(i=0;i<MAXDOT;++i) if(dot[i].t) myfwrite(&dot[i],1,sizeof(dot_t),h);
}

void DOT_loadgame(FILE* h) {
  int n;
  
  myfread(&n,1,4,h);
  myfread(dot,1,n*sizeof(dot_t),h);
}

void DOT_init(void) {
  int i;

  for(i=0;i<MAXDOT;++i) {dot[i].t=0;dot[i].o.r=0;dot[i].o.h=1;}
  ldot=0;
}

static void incldot(void) {
  if(++ldot>=MAXDOT) ldot=0;
}

void DOT_alloc(void) {
  int i;

  for(i=0;i<MAXINI;++i) {
	bl_ini[i].xv=myrand(BL_XV*2+1)-BL_XV;
	bl_ini[i].yv=-myrand(BL_YV);
	bl_ini[i].c=0xB0+myrand(16);
	bl_ini[i].t=myrand(BL_MAXT-BL_MINT+1)+BL_MINT;
	sp_ini[i].xv=myrand(SP_V*2+1)-SP_V;
	sp_ini[i].yv=myrand(SP_V*2+1)-SP_V;
	sp_ini[i].c=0xA0+myrand(6);
	sp_ini[i].t=myrand(SP_MAXT-SP_MINT+1)+SP_MINT;
  }
  for(i=0;i<MAXSR;++i) {
	sxr[i]=myrand(2*2+1)-2;
	syr[i]=myrand(2*2+1)-2;
  }
  bl_r=sp_r=sr_r=0;
}

void DOT_act(void) {
  int i,s,xv,yv;

  z_dot=1;
  for(i=0;i<MAXDOT;++i) if(dot[i].t) {
    xv=dot[i].o.xv+dot[i].o.vx;
    yv=dot[i].o.yv+dot[i].o.vy;
    s=Z_moveobj(&dot[i].o);
    if(dot[i].t<254) --dot[i].t;
    if(s&(Z_HITWATER|Z_FALLOUT)) {dot[i].t=0;continue;}
    if(s&Z_HITLAND) {
      if(!dot[i].o.xv) {
        if(yv>2) {
          if(!xv) dot[i].o.vx=(rand()&1)?-1:1;
          else dot[i].o.vx=Z_sign(dot[i].o.vx);
          if(rand()%yv==0) dot[i].o.vx*=2;
          dot[i].o.yv=yv-2;
        }
      }
      dot[i].o.xv=0;
      if(dot[i].t>4 && dot[i].t!=255) dot[i].t=4;
    }
    if(s&Z_HITWALL) {
      dot[i].o.vx=Z_sign(xv)*2;
      dot[i].o.yv=Z_sign(dot[i].o.yv);
      if(dot[i].o.yv>=0) if(rand()&3) --dot[i].o.yv;
      if(dot[i].o.yv>=0) if(rand()&1) --dot[i].o.yv;
    }
    if(s&Z_HITCEIL) {dot[i].o.xv=0;dot[i].o.yv=(myrand(100))?-2:0;}
  }
  z_dot=0;
}

void DOT_draw(void) {
  int i;

  for(i=0;i<MAXDOT;++i)
    if(dot[i].t) V_dot(dot[i].o.x-w_x+WD/2,dot[i].o.y-w_y+HT/2+1+w_o,dot[i].c);//if(dot[i].t) V_dot(dot[i].o.x-w_x+100,dot[i].o.y-w_y+50+w_o,dot[i].c);
}

void DOT_add(int x,int y,char xv,char yv,byte c,byte t) {
  int i;

  if(!Z_canfit(x,y,0,1)) return;
  i=ldot;
  dot[i].o.x=x;dot[i].o.y=y;
  dot[i].o.xv=xv;dot[i].o.yv=yv;
  dot[i].c=c;dot[i].t=t;
  dot[i].o.vx=dot[i].o.vy=0;
  incldot();
}

void DOT_blood(int x,int y,int xv,int yv,int n) {
  int i,k,dx,dy;

  for(k=n;k;--k) {
    dx=x+sxr[sr_r];dy=y+syr[sr_r];
    if(!Z_canfit(x,y,0,1)) continue;
    i=ldot;
	dot[i].o.x=dx;dot[i].o.y=dy;
	dot[i].o.xv=bl_ini[bl_r].xv+Z_dec(xv,3);
	dot[i].o.yv=bl_ini[bl_r].yv+Z_dec(yv,3)-3;
	dot[i].c=bl_ini[bl_r].c;
	dot[i].t=255;
	dot[i].o.vx=dot[i].o.vy=0;
	if(++bl_r>=MAXINI) bl_r=0;
	if(++sr_r>=MAXSR) sr_r=0;
    incldot();
  }
}

void DOT_spark(int x,int y,int xv,int yv,int n) {
  int i,k,dx,dy;

  for(k=n;k;--k) {
    dx=x+sxr[sr_r];dy=y+syr[sr_r];
    if(!Z_canfit(x,y,0,1)) continue;
    i=ldot;
	dot[i].o.x=dx;dot[i].o.y=dy;
	dot[i].o.xv=sp_ini[sp_r].xv-xv/4;
	dot[i].o.yv=sp_ini[sp_r].yv-yv/4;
	dot[i].c=sp_ini[sp_r].c;
	dot[i].t=sp_ini[sp_r].t;
	dot[i].o.vx=dot[i].o.vy=0;
	if(++sp_r>=MAXINI) sp_r=0;
	if(++sr_r>=MAXSR) sr_r=0;
    incldot();
  }
}

void DOT_water(int x,int y,int xv,int yv,int n,int c) {
  int i,k,dx,dy;
  static byte ct[3]={0xC0,0x70,0xB0};

  if(c<0 || c>=3) return;
  c=ct[c];
  for(k=n;k;--k) {
    dx=x+sxr[sr_r];dy=y+syr[sr_r];
    if(!Z_canfit(x,y,0,1)) continue;
    i=ldot;
	dot[i].o.x=dx;dot[i].o.y=dy;
	dot[i].o.xv=bl_ini[bl_r].xv-Z_dec(xv,3);
	dot[i].o.yv=bl_ini[bl_r].yv-abs(yv);
	dot[i].c=bl_ini[bl_r].c-0xB0+c;
	dot[i].t=254;
	dot[i].o.vx=dot[i].o.vy=0;
	if(++bl_r>=MAXINI) bl_r=0;
	if(++sr_r>=MAXSR) sr_r=0;
    incldot();
  }
}
