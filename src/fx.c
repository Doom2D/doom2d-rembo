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

#include "glob.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vga.h"
#include "view.h"
#include "fx.h"
#include "misc.h"

enum{NONE,TFOG,IFOG,BUBL};

#pragma pack(1)
typedef struct{
  int x,y,xv,yv;
  char t,s;
}fx_t;
#pragma pack()

static void *spr[15],*bsnd[2];
static char sprd[15];
static fx_t fx[MAXFX];
static char bubsn;
static int last;

unsigned char fx_scr1[64000],fx_scr2[64000];

#define SINP 256
#define SINPM (SINP-1)
#define W (SINP/100)

static int stdsin[SINP]={
#include "fx1sin.dat"
};
static int sintab[SINP];

static unsigned char dmap[32*32];

static int isin(int a) {
  return sintab[a&SINPM];
}

static void setamp(int a) {
  int i;

  for(i=0;i<SINP;++i) sintab[i]=stdsin[i]*a;
}

void FX_trans1(int t) {

  int x,y,u,v;
  static unsigned char k;
  unsigned p;

  setamp(t);
  k=t;
  t*=W;
  for(y=0,p=0;y<200;++y)
    for(x=0;x<320;++x,++p) {
      if(dmap[(y&31)*32+(x&31)]>=k) {
        u=x+((isin(y+t))>>16);
        v=y+((isin(u+t))>>16);
        if(u<0 || u>=320 || v<0 || v>=200) scra[p]=0;
        else scra[p]=fx_scr1[v*320+u];
      }else scra[p]=fx_scr2[p];
    }

}

static void init_fx1sin(void) {
  int j,r,l,rr;
  unsigned i;

  memset(dmap,0,32*32);
  for(i=1,rr=32*32;i<64;++i) {
    for(l=32*32/64;l;--l,--rr) {
      r=rand()%rr;
      for(j=0;r;--r,++j) {
        for(;dmap[j];++j);
      }
      for(;dmap[j];++j);
      dmap[j]=i;
    }
  }
}

void FX_savegame(FILE* h) {
  int i,n;

  for(i=n=0;i<MAXFX;++i) if(fx[i].t) ++n;
  myfwrite(&n,1,4,h);
  for(i=0;i<MAXFX;++i) if(fx[i].t) myfwrite(&fx[i],1,sizeof(fx_t),h);
}

void FX_loadgame(FILE* h) {
  int n;

  myfread(&n,1,4,h);
  myfread(fx,1,n*sizeof(fx_t),h);
}

void FX_alloc(void) {
  int i;

  for(i=0;i<10;++i) spr[i]=Z_getspr("TFOG",i,0,sprd+i);
  for(;i<15;++i) spr[i]=Z_getspr("IFOG",i-10,0,sprd+i);
  bsnd[0]=Z_getsnd("BUBL1");
  bsnd[1]=Z_getsnd("BUBL2");
  init_fx1sin();
}

void FX_init(void) {
  int i;

  for(i=0;i<MAXFX;++i) fx[i].t=0;
  bubsn=0;
  last=0;
}

void FX_act(void) {
  int i;
  byte b;

  bubsn=0;
  for(i=0;i<MAXFX;++i) switch(fx[i].t) {
    case TFOG:
      if(++fx[i].s>=20) fx[i].t=0;
      break;
    case IFOG:
      if(++fx[i].s>=10) fx[i].t=0;
      break;
    case BUBL:
      fx[i].yv-=5;
      fx[i].xv=Z_dec(fx[i].xv,20);
      fx[i].x+=fx[i].xv;
      fx[i].y+=fx[i].yv;
      if((b=fld[fx[i].y>>11][fx[i].x>>11]) < 5 || b>7) fx[i].t=0;
      break;
  }
}

void FX_draw(void) {
  int i,s;

  for(i=0;i<MAXFX;++i) {
    s=-1;
    switch(fx[i].t) {
      case TFOG: s=fx[i].s/2;break;
	  case IFOG: s=fx[i].s/2+10;break;
	  case BUBL:
		V_dot((fx[i].x>>8)-w_x+WD/2,(fx[i].y>>8)-w_y+HT/2+1+w_o,0xC0+fx[i].s);//V_dot((fx[i].x>>8)-w_x+100,(fx[i].y>>8)-w_y+50+w_o,0xC0+fx[i].s);
		continue;
    }
    if(s>=0) Z_drawspr(fx[i].x,fx[i].y,spr[s],sprd[s]);
  }
}

static int findfree(void) {
  int i;

  for(i=0;i<MAXFX;++i) if(!fx[i].t) return i;
  for(i=0;i<MAXFX;++i) if(fx[i].t==IFOG) return i;
  if(++last>=MAXFX) last=0;
  return last;
}

void FX_tfog(int x,int y) {
  int i;

  i=findfree();
	fx[i].t=TFOG;fx[i].s=0;
	fx[i].x=x;fx[i].y=y;
}

void FX_ifog(int x,int y) {
  int i;

  i=findfree();
    fx[i].t=IFOG;fx[i].s=0;
    fx[i].x=x;fx[i].y=y;
}

void FX_bubble(int x,int y,int xv,int yv,int n) {
  int i;

  if(!bubsn) {Z_sound(bsnd[rand()&1],128);bubsn=1;}
  for(;n>0;--n) {
	i=findfree();
	fx[i].t=BUBL;fx[i].s=rand()&3;
	fx[i].x=(x<<8)+myrand(513)-256;fx[i].y=(y<<8)+myrand(513)-256;
	fx[i].xv=xv;fx[i].yv=yv-myrand(256)-768;
  }
}

