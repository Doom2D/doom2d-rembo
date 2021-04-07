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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "view.h"
#include "fx.h"
#include "misc.h"

enum{NONE,TFOG,IFOG,BUBL};

fx_t fx[MAXFX];

static void *bsnd[2];
static char bubsn;
static int last;

//unsigned char fx_scr1[64000],fx_scr2[64000];

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

/*
void FX_trans1 (int t) {
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
*/

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

void FX_alloc (void) {
  bsnd[0]=Z_getsnd("BUBL1");
  bsnd[1]=Z_getsnd("BUBL2");
  init_fx1sin();
}

void FX_init (void) {
  int i;

  for(i=0;i<MAXFX;++i) fx[i].t=0;
  bubsn=0;
  last=0;
}

void FX_act (void) {
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

static int findfree (void) {
  int i;

  for(i=0;i<MAXFX;++i) if(!fx[i].t) return i;
  for(i=0;i<MAXFX;++i) if(fx[i].t==IFOG) return i;
  if(++last>=MAXFX) last=0;
  return last;
}

void FX_tfog (int x, int y) {
  int i;

  i=findfree();
	fx[i].t=TFOG;fx[i].s=0;
	fx[i].x=x;fx[i].y=y;
}

void FX_ifog (int x, int y) {
  int i;

  i=findfree();
    fx[i].t=IFOG;fx[i].s=0;
    fx[i].x=x;fx[i].y=y;
}

void FX_bubble (int x, int y, int xv, int yv, int n) {
  int i;

  if(!bubsn) {Z_sound(bsnd[rand()&1],128);bubsn=1;}
  for(;n>0;--n) {
	i=findfree();
	fx[i].t=BUBL;fx[i].s=rand()&3;
	fx[i].x=(x<<8)+myrand(513)-256;fx[i].y=(y<<8)+myrand(513)-256;
	fx[i].xv=xv;fx[i].yv=yv-myrand(256)-768;
  }
}
