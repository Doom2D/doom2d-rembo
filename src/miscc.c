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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "files.h"
#include "memory.h"
#include "sound.h"
#include "view.h"
#include "bmap.h"
#include "dots.h"
#include "monster.h"
#include "misc.h"
#include "render.h"

//#define WD 200
//#define HT 98

#define MAX_YV 30

extern dword walf[256];

byte z_dot=0;

extern void *walp[256];

static void *bulsnd[2];

int Z_sign(int a) {
  if(a>0) return 1;
  if(a<0) return -1;
  return 0;
}

int Z_dec(int a,int b) {
  if(abs(a)<=b) return 0;
  if(a>0) return a-b;
  if(a<0) return a+b;
  return 0;
}

void *Z_getsnd(char n[6]) {
  char s[8];

  //if(snd_type==-1) return NULL;
  strncpy(s+2,n,6);s[0]='D';
  s[1]='S';

  int id = F_getresid(s);
  int loaded = M_was_locked(id);
  snd_t *snd = M_lock(id);
  if (snd != NULL && !loaded) {
    snd->len = int2host(snd->len);
    snd->rate = int2host(snd->rate);
    snd->lstart = int2host(snd->lstart);
    snd->llen = int2host(snd->llen);
  }
  return snd;
}

int Z_sound(void *s,int v) {
  //if(snd_type==-1) return 0;
  if(!s) return 0;
  S_play(s,-1,1024,v);
  return F_getreslen(((int*)s)[-1])/605;
}

#define GAS_START (MN__LAST-MN_DEMON+5)
#define GAS_TOTAL (MN__LAST-MN_DEMON+16+10)

void Z_initst(void) {
  bulsnd[0]=Z_getsnd("BUL1");
  bulsnd[1]=Z_getsnd("BUL2");
}

int Z_canstand(int x,int y,int r) {
  int i;

  i=(x-r)/CELW;
  x=(x+r)/CELW;
  y=(y+1)/CELH;
  if(y>=FLDH || y<0) return 0;
  if(i<0) i=0;
  if(x>=FLDW) x=FLDW-1;
  for(;i<=x;++i)
    if(fld[y][i]==1 || fld[y][i]==2 || fld[y][i]==4)
      if(!z_dot) return 1;
      else if(!((walf[fldf[y][i]]|walf[fldb[y][i]])&2)) return 1;
  return 0;
}

int Z_hitceil(int x,int y,int r,int h) {
  int i;

  i=(x-r)/CELW;
  x=(x+r)/CELW;
  y=(y-h+1)/CELH;
  if(y>=FLDH || y<0) return 0;
  if(i<0) i=0;
  if(x>=FLDW) x=FLDW-1;
  for(;i<=x;++i)
    if(fld[y][i]==1 || fld[y][i]==2)
      if(!z_dot) return 1;
      else if(!((walf[fldf[y][i]]|walf[fldb[y][i]])&2)) return 1;
  return 0;
}

int Z_canfit(int x,int y,int r,int h) {
  int i,j,sx,sy;

  sx=(x-r)/CELW;
  sy=(y-h+1)/CELH;
  if(sx<0) sx=0;
  if(sy<0) sy=0;
  x=(x+r)/CELW;
  y=(y-0)/CELH;
  if(x>=FLDW) x=FLDW-1;
  if(y>=FLDH) y=FLDH-1;
  for(i=sx;i<=x;++i)
    for(j=sy;j<=y;++j)
      if(fld[j][i]==1 || fld[j][i]==2)
        if(!z_dot) return 0;
        else if(!((walf[fldf[j][i]]|walf[fldb[j][i]])&2)) return 0;
  return 1;
}

int Z_inlift(int x,int y,int r,int h) {
  int i,j,sx,sy;

  sx=(x-r)/CELW;
  sy=(y-h+1)/CELH;
  if(sx<0) sx=0;
  if(sy<0) sy=0;
  x=(x+r)/CELW;
  y=(y-1)/CELH;
  if(x>=FLDW) x=FLDW-1;
  if(y>=FLDH) y=FLDH-1;
  for(i=sx;i<=x;++i)
	for(j=sy;j<=y;++j)
	  if(fld[j][i]==9 || fld[j][i]==10) return fld[j][i]-8;
  return 0;
}

int Z_isblocked(int x,int y,int r,int h,int xv) {
  int i,j,sx,sy;

  sx=(x-r)/CELW;
  sy=(y-h+1)/CELH;
  if(sx<0) sx=0;
  if(sy<0) sy=0;
  x=(x+r)/CELW;
  y=(y-1)/CELH;
  if(xv<0) x=sx;
  else if(xv>0) sx=x;
  if(x>=FLDW) x=FLDW-1;
  if(y>=FLDH) y=FLDH-1;
  for(i=sx;i<=x;++i)
	for(j=sy;j<=y;++j)
	  if(fld[j][i]==8) return 1;
  return 0;
}

int Z_istrapped(int x,int y,int r,int h) {
  int i,j,sx,sy;

  sx=(x-r)/CELW;
  sy=(y-h+1)/CELH;
  if(sx<0) sx=0;
  if(sy<0) sy=0;
  x=(x+r)/CELW;
  y=(y-1)/CELH;
  if(x>=FLDW) x=FLDW-1;
  if(y>=FLDH) y=FLDH-1;
  for(i=sx;i<=x;++i)
    for(j=sy;j<=y;++j)
	  if(fld[j][i]==255) return 1;
  return 0;
}

void Z_set_speed(obj_t *o,int s) {
  int m;

  if(!(m=max(abs(o->xv),abs(o->yv)))) m=1;
  o->xv=o->xv*s/m;o->yv=o->yv*s/m;
}

static byte wfront;

int Z_inwater(int x,int y,int r,int h) {
  int i,j,sx,sy;

  sx=(x-r)/CELW;
  sy=(y-h+1)/CELH;
  if(sx<0) sx=0;
  if(sy<0) sy=0;
  x=(x+r)/CELW;
  y=(y-h/2)/CELH;
  if(x>=FLDW) x=FLDW-1;
  if(y>=FLDH) y=FLDH-1;
  for(i=sx;i<=x;++i)
	for(j=sy;j<=y;++j)
	  if(fld[j][i]>=5 && fld[j][i]<=7) {wfront=fldf[j][i];return 1;}
  return 0;
}

int Z_getacid(int x,int y,int r,int h) {
  int i,j,sx,sy,a;
  static byte tab[4]={0,5,10,20};

  a=0;
  sx=(x-r)/CELW;
  sy=(y-h+1)/CELH;
  if(sx<0) sx=0;
  if(sy<0) sy=0;
  x=(x+r)/CELW;
  y=y/CELH;
  if(x>=FLDW) x=FLDW-1;
  if(y>=FLDH) y=FLDH-1;
  for(i=sx;i<=x;++i)
	for(j=sy;j<=y;++j)
	  if(fld[j][i]==6) a|=1;
	  else if(fld[j][i]==7) a|=2;
  return tab[a];
}

int Z_canbreathe(int x,int y,int r,int h) {
  int i,j,sx,sy;

  sx=(x-r)/CELW;
  sy=(y-h+1)/CELH;
  if(sx<0) sx=0;
  if(sy<0) sy=0;
  x=(x+r)/CELW;
  y=(y-h/2)/CELH;
  if(x>=FLDW) x=FLDW-1;
  if(y>=FLDH) y=FLDH-1;
  if(sx>x || sy>y) return 1;
  for(i=sx;i<=x;++i)
    for(j=sy;j<=y;++j)
      if(fld[j][i]==0 || fld[j][i]==3 || fld[j][i]==9 || fld[j][i]==10) return 1;
  return 0;
}

int Z_overlap(obj_t *a,obj_t *b) {
  if(a->x - a->r > b->x + b->r) return 0;
  if(a->x + a->r < b->x - b->r) return 0;
  if(a->y <= b->y - b->h) return 0;
  if(a->y - a->h >= b->y) return 0;
  return 1;
}

void Z_kickobj(obj_t *o,int x,int y,int pwr) {
  int dx,dy,m;

  dx=o->x-x;dy=o->y-o->h/2-y;
  if(!(m=max(abs(dx),abs(dy)))) m=1;
  o->vx+=(long)dx*pwr/m;
  o->vy+=(long)dy*pwr/m;
}

int Z_cansee(int x,int y,int xd,int yd) {
  register dword d,m;
  int sx,sy;
  dword xe,ye,s,i;

  if((xd-=x)>0) sx=1;
  else if(xd<0) sx=-1;
  else sx=0;
  if((yd-=y)>0) sy=1;
  else if(yd<0) sy=-1;
  else sy=0;
  if(!xd && !yd) return 1;
  if((xd=abs(xd)) > (yd=abs(yd))) d=xd; else d=yd;
  xe=ye=0;
  for(i=0;i<=d;) {
	if(x<0 || x>=FLDW*8 || y<0 || y>=FLDH*8) return 0;
	if((bmap[y>>5][x>>5]&BM_WALL)) {
	  if(fld[y>>3][x>>3]==1 || fld[y>>3][x>>3]==2) return 0;
	  if((xe+=(xd<<3))>=d) {
		x+=xe/d*sx;xe=xe%d;
	  }
	  if((ye+=(yd<<3))>=d) {
		y+=ye/d*sy;ye=ye%d;
	  }
	  i+=8;
	}else{
	  if(sx==0) m=0;
	  else{m=x&31;if(sx>0) m^=31; ++m;}
	  if(sy==0) s=0;
	  else{s=y&31;if(sy>0) s^=31; ++s;}
	  if((s<m && s!=0) || m==0) m=s;
	  i+=m;
		x+=(xd*m+xe)/d*sx;xe=(xd*m+xe)%d;
		y+=(yd*m+ye)/d*sy;ye=(yd*m+ye)%d;
	}
  }
  return 1;
}

int Z_look(obj_t *a,obj_t *b,int d) {
  if(Z_sign(b->x-a->x)!=d*2-1) return 0;
  return Z_cansee(a->x,a->y-a->h/2,b->x,b->y-b->h/2);
}

#define wvel(v) if((xv=abs(v)+1)>5) v=Z_dec(v,xv/2-2)

byte z_mon=0;

int Z_moveobj(obj_t *p) {
  static int x,y,xv,yv,r,h,lx,ly,st;
  static byte inw;

  st=0;
  switch(Z_inlift(x=p->x,y=p->y,r=p->r,h=p->h)) {
    case 0:
      if(++p->yv>MAX_YV) --p->yv;
      break;
    case 1:
      if(--p->yv < -5) ++p->yv;
      break;
    case 2:
      if(p->yv > 5) {--p->yv;break;}
      ++p->yv;break;
  }
  if((inw=Z_inwater(x,y,r,h))!=0) {
	st|=Z_INWATER;
	wvel(p->xv);
	wvel(p->yv);
	wvel(p->vx);
	wvel(p->vy);
  }
  p->vx=Z_dec(p->vx,1);
  p->vy=Z_dec(p->vy,1);
  xv=p->xv+p->vx;yv=p->yv+p->vy;
  while(xv || yv) {
	if(x<-100 || x>=FLDW*8+100 || y<-100 || y>=FLDH*8+100)
	  {st|=Z_FALLOUT;}

	lx=x;
	x+=(abs(xv)<=7)?xv:((xv>0)?7:-7);
	if(z_mon) if(Z_isblocked(x,y,r,h,xv)) st|=Z_BLOCK;
	if(!Z_canfit(x,y,r,h)) {
	  if(xv==0) x=lx;
	  else if(xv<0) x=((lx-r)&0xFFF8)+r;
          else x=((lx+r)&0xFFF8)-r+7;
	  xv=p->xv=p->vx=0;st|=Z_HITWALL;
	}
	xv-=(abs(xv)<=7)?xv:((xv>0)?7:-7);

	ly=y;
	y+=(abs(yv)<=7)?yv:((yv>0)?7:-7);
	if(yv>=8) --y;
	if(yv<0 && Z_hitceil(x,y,r,h)) {
	  y=((ly-h+1)&0xFFF8)+h-1;
	  yv=p->vy=1;p->yv=0;st|=Z_HITCEIL;
	}
	if(yv>0 && Z_canstand(x,y,r)) {
	  y=((y+1)&0xFFF8)-1;
	  yv=p->yv=p->vy=0;st|=Z_HITLAND;
	}
	yv-=(abs(yv)<=7)?yv:((yv>0)?7:-7);
  }
  p->x=x;p->y=y;
  if(Z_inwater(x,y,r,h)) {
	st|=Z_INWATER;
	if(!inw) st|=Z_HITWATER;
  }else if(inw) st|=Z_HITAIR;
  return st;
}

void Z_splash (obj_t *p, int n) {
  Z_sound(bulsnd[0], 128);
  DOT_water(p->x, p->y-p->h / 2, p->xv + p->vx, p->yv + p->vy, n, R_get_special_id(wfront) - 1);
}

void Z_calc_time(dword t,word *h,word *m,word *s)
{
    t = t * DELAY;
    t = t / 1000;
    *s = t % 60;
    t = t - *s;
    t = t / 60;
    *m = t % 60;
    t = t - *m;
    t = t / 60;
    *h = t;
}

#define SWAP_VAR(a, b) do { unsigned char t = a; a = b; b = t; } while(0)

int16_t short2swap (int16_t x) {
  union {
    uint8_t a[2];
    int16_t x;
  } y;
  y.x = x;
  SWAP_VAR(y.a[0], y.a[1]);
  return y.x;
}

int32_t int2swap (int32_t x) {
  union {
    uint8_t a[4];
    int32_t x;
  } y;
  y.x = x;
  SWAP_VAR(y.a[0], y.a[3]);
  SWAP_VAR(y.a[1], y.a[2]);
  return y.x;
}

#undef SWAP_VAR

int16_t short2host (int16_t x) {
#if __BIG_ENDIAN__
  return short2swap(x);
#else
  return x;
#endif
}

int32_t int2host (int32_t x) {
#if __BIG_ENDIAN__
  return int2swap(x);
#else
  return x;
#endif
}
