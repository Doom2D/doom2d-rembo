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
#include "vga.h"
#include "sound.h"
#include "view.h"
#include "bmap.h"
#include "dots.h"
#include "monster.h"
#include "misc.h"

//#define WD 200
//#define HT 98

#define MAX_YV 30

#define MAXAIR 1091

extern dword walf[256];

byte z_dot=0;

extern void *walp[256];

static void *sth[22],*bfh[160-'!'],*sfh[160-'!'],*bulsnd[2],*stone, *stone2, *keys[3];
static int prx=0,pry=0;

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

void *Z_getspr(char n[4],int s,int d,char *dir) {
  int h;

  h=F_getsprid(n,s,d);
  if(dir) *dir=(h&0x8000)?1:0;
  //return M_lock(h);
  return V_getvgaimg(h);
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
  int i;
  char s[10];
  static char nm[22][8]={
	"STTNUM0","STTNUM1","STTNUM2","STTNUM3","STTNUM4",
	"STTNUM5","STTNUM6","STTNUM7","STTNUM8","STTNUM9",
	"STTMINUS","STTPRCNT",
	"FISTA0","CSAWA0","PISTA0","SHOTA0","SGN2A0","MGUNA0","LAUNA0",
	"PLASA0","BFUGA0","GUN2A0"
  };

  stone=V_loadvgaimg("STONE");
  stone2=V_loadvgaimg("STONE2");
  keys[0]=V_loadvgaimg("KEYRA0");
  keys[1]=V_loadvgaimg("KEYGA0");
  keys[2]=V_loadvgaimg("KEYBA0");
  for(i=0;i<22;++i)
    sth[i]=V_loadvgaimg(nm[i]);
  strcpy(s,"STBF_*");
  for(i='!';i<160;++i) {
	s[5]=i;
	bfh[i-'!']=V_getvgaimg(F_findres(s));
	if(!(i&15)) logo_gas(GAS_START+((i-'!')>>4),GAS_TOTAL);
  }
  for(i='!';i<160;++i) {
	sprintf(s,"STCFN%03d",i);
	sfh[i-'!']=V_getvgaimg(F_findres(s));
	if(!(i&15)) logo_gas(GAS_START+8+((i-'!')>>4),GAS_TOTAL);
  }
  strcpy(s,"WINUM*");
  for(i='0';i<='9';++i) {
	s[5]=i;
	bfh[i-'!']=V_loadvgaimg(s);
  }
  bfh[':'-'!']=V_loadvgaimg("WICOLON");
  bulsnd[0]=Z_getsnd("BUL1");
  bulsnd[1]=Z_getsnd("BUL2");
}

void Z_putbfch(int c) {
  vgaimg *p;

  if(c>32 && c<160) p=bfh[c-'!']; else p=NULL;
  if(p) {
    V_spr(prx,pry,p);
    prx+=p->w-1;
  }else prx+=12;
}

void Z_putsfch(int c) {
  vgaimg *p;

  if(c>32 && c<160) p=sfh[c-'!']; else p=NULL;
  if(p) {
    V_spr(prx,pry,p);
    prx+=p->w-1;
  }else prx+=7;
}

void Z_gotoxy(int x,int y) {prx=x;pry=y;}

void Z_printbf(char *s,...) {
  int i;
  va_list ap;
  char buf[80];

  va_start(ap,s);
  vsprintf(buf,s,ap);
  va_end(ap);
  for(i=0;buf[i];++i) switch(buf[i]) {
	case '\n':
	  pry+=13;
	case '\r':
	  prx=0;break;
	default:
	  Z_putbfch((byte)buf[i]);
  }
}

void Z_printsf(char *s,...) {
  int i;
  va_list ap;
  char buf[80];

  va_start(ap,s);
  vsprintf(buf,s,ap);
  va_end(ap);
  for(i=0;buf[i];++i) switch(buf[i]) {
	case '\n':
	  pry+=8;
	case '\r':
	  prx=0;break;
	default:
	  Z_putsfch((byte)buf[i]);
  }
}

void Z_drawspr(int x,int y,void *p,char d) {
  if(d) V_spr2(x-w_x+WD/2,y-w_y+HT/2+1+w_o,p);//if(d) V_spr2(x-w_x+100,y-w_y+HT/2+1+w_o,p);
    else V_spr(x-w_x+WD/2,y-w_y+HT/2+1+w_o,p);//else V_spr(x-w_x+100,y-w_y+HT/2+1+w_o,p);
}

void Z_clrst(void) {
  V_pic(SCRW-120,w_o,stone);//V_pic(200,w_o,stone);
  int y = ((vgaimg*)stone)->h;
  while (y<HT) {
    V_pic(SCRW-120,w_o+y,stone2);
    y+=((vgaimg*)stone)->h;
  }
}


void Z_drawstlives(char n) {
  V_setrect(SCRW-40,30,w_o,40);Z_clrst();//V_setrect(280,30,w_o,40);Z_clrst();
  V_spr(SCRW-35,w_o+17,sth[n]);//V_spr(285,w_o+17,sth[n]);
}

void Z_drawstkeys(byte k) {
  int x,n;

  V_setrect(SCRW-120,70,w_o+77,23);Z_clrst();//V_setrect(200,70,w_o+77,23);Z_clrst();
  for(k>>=4,n=0,x=SCRW-75;n<3;++n,k>>=1,x+=9)//for(k>>=4,n=0,x=245;n<3;++n,k>>=1,x+=9)
    if(k&1) V_spr(x,w_o+91,keys[n]);
}

void Z_drawstair(int a) {
  V_setrect(SCRW-120,120,w_o+49,2);Z_clrst();//V_setrect(200,120,w_o+49,2);Z_clrst();
  if(a<=0) return;
  if(a>MAXAIR) a=MAXAIR;
  a=a*100/MAXAIR;
  if(!a) return;
  V_clr(SCRW-110,a,w_o+49,2,0xC8);//V_clr(210,a,w_o+49,2,0xC8);
}

void Z_drawstprcnt(int y,int n) {
  char s[20];
  int l,i,x,c;

  V_setrect(SCRW-120,70,y*19+7+w_o,19);Z_clrst();//V_setrect(200,70,y*19+7+w_o,19);Z_clrst();
  sprintf(s,"%3d%%",n);
  l=strlen(s);x=SCRW-110;//l=strlen(s);x=210;
  for(i=0;i<l;++i,x+=14) {
    if(s[i]>='0' && s[i]<='9') c=s[i]-'0';
    else if(s[i]=='-') c=10;
    else if(s[i]=='%') c=11;
    else c=-1;
    if(c>=0)
      V_spr(x,y*19+7+w_o,sth[c]);
  }
}

void Z_drawstnum(int n) {
  char s[20];
  int l,i,x,c;

  V_setrect(SCRW-50,50,w_o+77,23);Z_clrst();//V_setrect(270,50,w_o+77,23);Z_clrst();
  if(!g_dm) return;
  sprintf(s,"%d",n);
  l=strlen(s);x=(115-l*14)+SCRW-120;//l=strlen(s);x=(115-l*14)+200;
  for(i=0;i<l;++i,x+=14) {
    if(s[i]>='0' && s[i]<='9') c=s[i]-'0';
    else if(s[i]=='-') c=10;
    else if(s[i]=='%') c=11;
    else c=-1;
    if(c>=0)
      V_spr(x,w_o+77+5,sth[c]);
  }
}

void Z_drawstwpn(int n,int a) {
  char s[20];
  int l,i,x,c;

  i=n;
  V_setrect(SCRW-120,120,w_o+58,23);Z_clrst();//V_setrect(200,120,w_o+58,23);Z_clrst();
  if(i>=0) V_spr(SCRW-88,w_o+58+19,sth[i+12]);//if(i>=0) V_spr(232,w_o+58+19,sth[i+12]);
  if(n>=2) {
	sprintf(s,"%d",a);
	l=strlen(s);x=SCRW-10-l*14;//l=strlen(s);x=310-l*14;
    for(i=0;i<l;++i,x+=14) {
      if(s[i]>='0' && s[i]<='9') c=s[i]-'0';
      else if(s[i]=='-') c=10;
      else if(s[i]=='%') c=11;
      else c=-1;
      if(c>=0)
	V_spr(x,w_o+58+2,sth[c]);
    }
  }
}



void Z_drawmanspr(int x,int y,void *p,char d,byte color) {
  if(d) V_manspr2(x-w_x+WD/2,y-w_y+HT/2+1+w_o,p,color);//if(d) V_manspr2(x-w_x+100,y-w_y+HT/2+1+w_o,p,color);
    else V_manspr(x-w_x+WD/2,y-w_y+HT/2+1+w_o,p,color);//else V_manspr(x-w_x+100,y-w_y+HT/2+1+w_o,p,color);
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

void Z_splash(obj_t *p,int n) {
  Z_sound(bulsnd[0],128);
  DOT_water(p->x,p->y-p->h/2,p->xv+p->vx,p->yv+p->vy,n,
	(int)walp[wfront]-1);
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