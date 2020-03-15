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
#include <string.h>
#include "view.h"
#include "bmap.h"
#include "switch.h"
#include "player.h"
#include "misc.h"
#include "map.h"
#include "files.h"
#include "game.h"
#include "my.h"
#include "monster.h"

#define MAXSW 100

#pragma pack(1)
typedef struct {
  byte x, y;
  byte t, tm;
  byte a, b, c, d;
  byte f;
} sw_t;
#pragma pack()

int sw_secrets;

static sw_t sw[MAXSW];
static void *sndswn, *sndswx, *sndnoway, *sndbdo, *sndbdc, *sndnotele;
static int swsnd;
static byte cht, chto, chf, f_ch;

void SW_savegame (FILE *h) {
  int i, n;
  for (n = MAXSW - 1; n >= 0 && sw[n].t == 0; n--) {
    // empty
  }
  n += 1;
  myfwrite32(n, h);
  for (i = 0; i < n; i++) {
    myfwrite8(sw[i].x, h);
    myfwrite8(sw[i].y, h);
    myfwrite8(sw[i].t, h);
    myfwrite8(sw[i].tm, h);
    myfwrite8(sw[i].a, h);
    myfwrite8(sw[i].b, h);
    myfwrite8(sw[i].c, h);
    myfwrite8(sw[i].d, h);
    myfwrite8(sw[i].f, h);
  }
  myfwrite32(sw_secrets, h);
}

void SW_loadgame (FILE *h) {
  int i, n;
  n = myfread32(h);
  for (i = 0; i < n; i++) {
    sw[i].x = myfread8(h);
    sw[i].y = myfread8(h);
    sw[i].t = myfread8(h);
    sw[i].tm = myfread8(h);
    sw[i].a = myfread8(h);
    sw[i].b = myfread8(h);
    sw[i].c = myfread8(h);
    sw[i].d = myfread8(h);
    sw[i].f = myfread8(h);
  }
  sw_secrets = myfread32(h);
}

int SW_load (FILE *h) {
  int i;
  switch(blk.t) {
    case MB_SWITCH2:
      sw_secrets = 0;
      for (i = 0; i < MAXSW && blk.sz > 0; ++i, blk.sz -= 9) {
        sw[i].x = myfread8(h);
        sw[i].y = myfread8(h);
        sw[i].t = myfread8(h);
        sw[i].tm = myfread8(h); // unused
        sw[i].a = myfread8(h);
        sw[i].b = myfread8(h);
        sw[i].c = myfread8(h);
        sw[i].d = myfread8(h); // unused
        sw[i].f = myfread8(h);
        sw[i].tm = 0;
        sw[i].d = 0;
        sw[i].f |= 0x80;
        if (sw[i].t == SW_SECRET) {
          ++sw_secrets;
        }
      }
      return 1;
  }
  return 0;
}

void SW_alloc (void) {
  sndswn=Z_getsnd("SWTCHN");
  sndswx=Z_getsnd("SWTCHX");
  sndnoway=Z_getsnd("NOWAY");
  sndbdo=Z_getsnd("BDOPN");
  sndbdc=Z_getsnd("BDCLS");
  sndnotele=Z_getsnd("NOTELE");
}

void SW_init (void) {
  int i;
  for (i = 0; i < MAXSW; i++) {
    sw[i].t = 0;
  }
  swsnd = 0;
}

static void door(byte x,byte y) {
  byte ex;

  if(x>=FLDW || y>=FLDH) return;
  if(fld[y][x]!=cht) return;
  ex=x+1;
  for(;x && fld[y][x-1]==cht;--x);
  for(;ex<FLDW && fld[y][ex]==cht;++ex);
  memset(fld[y]+x,chto,ex-x);
  if(f_ch) memset(fldf[y]+x,chf,ex-x);
  for(;x<ex;++x) {
	door(x,y-1);
	door(x,y+1);
  }
}

void Z_water_trap (obj_t *o) {
  int i,j,sx,sy,x,y;

  if((y=o->y)>=FLDH*CELH+o->h) return;
  if((x=o->x)<0 || o->x>FLDW*CELW) return;
  sx=(x-o->r)/CELW;
  sy=(y-o->h+1)/CELH;
  x=(x+o->r)/CELW;
  y=(y-o->h/2)/CELH;
  for(i=sx;i<=x;++i)
	for(j=sy;j<=y;++j)
	  if(fld[j][i]==5) {
		cht=5;chto=255;f_ch=0;
		door(i,j);
	  }
}

void Z_untrap (byte t) {
  byte *p;
  word n;

  for(p=(byte*)fld,n=FLDW*FLDH;n;--n,++p)
	if(*p==255) *p=t;
}

static void opendoor(int i) {
  int j;

  swsnd=Z_sound(sndbdo,128);
  j=fldf[sw[i].b][sw[i].a];
  cht=2;chto=3;chf=0;f_ch=1;
  door(sw[i].a,sw[i].b);
  fldf[sw[i].b][sw[i].a]=j;
  fld_need_remap=1;
}

static int shutdoor(int i) {
  int j;

  cht=3;chto=255;chf=fldf[sw[i].b][sw[i].a];f_ch=1;
  door(sw[i].a,sw[i].b);
  cht=255;
  if(Z_chktrap(0,0,-3,HIT_SOME)) {
	j=fldf[sw[i].b][sw[i].a];
	chto=3;chf=0;f_ch=1;
	door(sw[i].a,sw[i].b);
	fldf[sw[i].b][sw[i].a]=j;
	return 0;
  }
  chto=2;
  door(sw[i].a,sw[i].b);
  fld_need_remap=1;
  swsnd=Z_sound(sndbdc,128);
  return 1;
}

void SW_act (void) {
  int i;

  if(swsnd) --swsnd;
  for(i=0;i<MAXSW;++i) if(sw[i].t) {
    if(sw[i].tm) --sw[i].tm;
    switch(sw[i].t) {
      case SW_DOOR5: case SW_DOOR: case SW_SHUTDOOR:
        if(!sw[i].d) break;
        if(fld[sw[i].b][sw[i].a]!=3) {sw[i].d=0;break;}
        if(--sw[i].d==0) if(!shutdoor(i)) sw[i].d=9;
        break;
      case SW_TRAP:
        if(!sw[i].d) break;
        if(fld[sw[i].b][sw[i].a]!=2) {sw[i].d=0;break;}
        if(--sw[i].d==0) {opendoor(i);sw[i].tm=18;}
        break;
    }
  }
}

static int doortime(int t) {
  switch(t) {
	case SW_DOOR5:		return 90;
  }
  return 0;
}

void SW_cheat_open (void) {
  int i;

  for(i=0;i<MAXSW;++i) if(sw[i].t && !sw[i].tm) switch(sw[i].t) {
	case SW_DOOR: case SW_DOOR5:
	case SW_OPENDOOR:
	  if(fld[sw[i].b][sw[i].a]!=2) break;
	  SW_press(sw[i].x*CELW+4,sw[i].y*CELH+4,1,1,0xFF,-3);
	  break;
  }
}

int SW_press (int x, int y, int r, int h, byte t, int o) {
  int sx,sy,i,p;

  sx=(x-r)/CELW;sy=(y-h+1)/CELH;
  x=(x+r)/CELW;y/=CELH;
  for(i=p=0;i<MAXSW;++i) if(sw[i].t && !sw[i].tm) {
    if(sw[i].x>=sx && sw[i].x<=x && sw[i].y>=sy && sw[i].y<=y && ((sw[i].f&0x8F)&t)) {
      if(sw[i].f&0x70) if((sw[i].f&(t&0x70))!=(sw[i].f&0x70)) continue;
      switch(sw[i].t) {
		case SW_EXIT:
		  g_exit=1;sw[i].tm=9;swsnd=Z_sound(sndswx,128);break;
		case SW_EXITS:
		  g_exit=2;sw[i].tm=9;swsnd=Z_sound(sndswx,128);break;
		case SW_DOOR: case SW_DOOR5:
		  switch(fld[sw[i].b][sw[i].a]) {
			case 2:
			  opendoor(i);sw[i].tm=9;sw[i].d=doortime(sw[i].t);break;
			case 3:
			  if(shutdoor(i)) {sw[i].tm=9;sw[i].d=0;}
			  else {
			    if(!swsnd) swsnd=Z_sound(sndnoway,128);
			    sw[i].d=2;
			  }break;
		  }break;
		case SW_PRESS:
		  sw[i].tm=9;
		  SW_press((dword)sw[i].a*8+4,(dword)sw[i].b*8+12,8,16,(t&0x70)|0x80,o);
		  break;
		case SW_TELE:
		  if(o < -2) break;
		  if(!Z_canfit((dword)sw[i].a*8+4,(dword)sw[i].b*8+7,r,h)) {
		    if(!swsnd) swsnd=Z_sound(sndnotele,128);
		    break;
		  }Z_teleobj(o,(dword)sw[i].a*8+4,(dword)sw[i].b*8+7);
		  sw[i].tm=1;
		  break;
		case SW_OPENDOOR:
		  if(fld[sw[i].b][sw[i].a]!=2) break;
		  opendoor(i);
		  sw[i].tm=1;
		  break;
		case SW_SHUTDOOR:
		  if(fld[sw[i].b][sw[i].a]!=3) break;
		  if(shutdoor(i)) {sw[i].tm=1;sw[i].d=0;}
		  else {
		    if(!swsnd) swsnd=Z_sound(sndnoway,128);
		    sw[i].d=2;
		  }break;
		case SW_SHUTTRAP: case SW_TRAP:
		  if(fld[sw[i].b][sw[i].a]!=3) break;
		  cht=3;chto=255;chf=fldf[sw[i].b][sw[i].a];f_ch=1;
		  door(sw[i].a,sw[i].b);
		  Z_chktrap(1,100,-3,HIT_TRAP);
		  cht=255;chto=2;
		  door(sw[i].a,sw[i].b);
		  fld_need_remap=1;
		  swsnd=Z_sound(sndswn,128);
		  sw[i].tm=1;sw[i].d=20;
		  break;
		case SW_LIFT:
		  if(fld[sw[i].b][sw[i].a]==10) {
		    cht=10;chto=9;f_ch=0;
		  }else if(fld[sw[i].b][sw[i].a]==9) {
		    cht=9;chto=10;f_ch=0;
		  }else break;
		  door(sw[i].a,sw[i].b);
		  fld_need_remap=1;
		  swsnd=Z_sound(sndswx,128);
		  sw[i].tm=9;
		  break;
		case SW_LIFTUP:
		  if(fld[sw[i].b][sw[i].a]!=10) break;
		  cht=10;chto=9;f_ch=0;
		  door(sw[i].a,sw[i].b);
		  fld_need_remap=1;
		  swsnd=Z_sound(sndswx,128);
		  sw[i].tm=1;
		  break;
		case SW_LIFTDOWN:
		  if(fld[sw[i].b][sw[i].a]!=9) break;
		  cht=9;chto=10;f_ch=0;
		  door(sw[i].a,sw[i].b);
		  fld_need_remap=1;
		  swsnd=Z_sound(sndswx,128);
		  sw[i].tm=1;
		  break;
		case SW_SECRET:
		  if(o!=-1 && o!=-2) break;
		  if(o==-1) ++pl1.secrets;
		  else ++pl2.secrets;
		  sw[i].tm=1;sw[i].t=0;break;
      }
      if(sw[i].tm)
        {fldb[sw[i].y][sw[i].x]=walswp[fldb[sw[i].y][sw[i].x]];p=1;}
      if(sw[i].tm==1) sw[i].tm=0;
    }
  }
  return p;
}
