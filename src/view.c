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
#include <stdlib.h>
#include "vga.h"
#include "memory.h"
#include "files.h"
#include "error.h"
#include "view.h"
#include "dots.h"
#include "smoke.h"
#include "weapons.h"
#include "items.h"
#include "switch.h"
#include "fx.h"
#include "player.h"
#include "monster.h"
#include "misc.h"
#include "map.h"
#include "sound.h"

#define ANIT 5

/*
#define WD 200
#define HT 98

#define MAXX (FLDW*CELW-WD/2)
#define MAXY (FLDH*CELH-HT/2)
*/

int WD; //
int HT; //

extern map_block_t blk;

extern byte clrmap[256*12];
void V_remap_rect(int,int,int,int,byte *);

byte w_horiz=ON;
void *horiz=NULL;//static void *horiz=NULL;
int w_o,w_x,w_y,sky_type=1;
void *walp[256];
dword walf[256];
int walh[256];
byte walswp[256];
byte walani[256];
int anih[ANIT][5];
byte anic[ANIT];
byte fldb[FLDH][FLDW];
byte fldf[FLDH][FLDW];
byte fld[FLDH][FLDW];

extern int lt_time,lt_type,lt_side,lt_ypos;
extern void *ltn[2][2];

static void getname(int n,char *s) {
  if(walh[n]==-1) {memset(s,0,8);return;}
  if(walh[n]==-2) {
    memcpy(s,"_WATER_",8);s[7]=(byte)walp[n]-1+'0';
    return;
  }
  F_getresname(s,walh[n]&0x7FFF);
}

static short getani(char *n) {
  if(strncasecmp(n,"WALL22_1",8)==0) return 1;
  if(strncasecmp(n,"WALL58_1",8)==0) return 2;
  if(strncasecmp(n,"W73A_1",8)==0) return 3;
  if(strncasecmp(n,"RP2_1",8)==0) return 4;
  return 0;
}

void W_savegame(FILE* h) {
  char s[8];
  int i;

  myfwrite(&sky_type,1,4,h);
  for(i=1;i<256;++i) {
    getname(i,s);myfwrite(s,1,8,h);
  }
  myfwrite(walf,1,sizeof(walf),h);
  myfwrite(walswp,1,sizeof(walswp),h);
  myfwrite(fldb,1,FLDW*FLDH,h);
  myfwrite(fld,1,FLDW*FLDH,h);
  myfwrite(fldf,1,FLDW*FLDH,h);
}

void W_loadgame(FILE* h) {
  char s[8];
  int i;

  myfread(&sky_type,1,4,h);
  for(i=1;i<256;++i) {
    walani[i]=0;
    myfread(s,1,8,h);if(!s[0]) {walh[i]=-1;walp[i]=NULL;continue;}
    walani[i]=getani(s);
    if(strncasecmp(s,"_WATER_",7)==0) { //if(memicmp(s,"_WATER_",7)==0) {
      walh[i]=-2;walp[i]=(void*)(s[7]-'0'+1);
    }else walp[i]=M_lock(walh[i]=F_getresid(s));
  }
  myfread(walf,1,sizeof(walf),h);
  for(i=1;i<256;++i) if(walf[i]&1) walh[i]|=0x8000;
  myfread(walswp,1,sizeof(walswp),h);
  myfread(fldb,1,FLDW*FLDH,h);
  myfread(fld,1,FLDW*FLDH,h);
  myfread(fldf,1,FLDW*FLDH,h);
  strcpy(s,"RSKY1");s[4]=sky_type+'0';
  M_unlock(horiz);
  horiz=M_lock(F_getresid(s));
}

void W_adjust(void) {
  int MAXX =(FLDW*CELW-WD/2);//
  int MAXY =(FLDH*CELH-HT/2);//

  if(w_x<WD/2) w_x=WD/2;
  if(w_y<HT/2) w_y=HT/2;
  if(w_x>MAXX) w_x=MAXX;
  if(w_y>MAXY) w_y=MAXY;
}

void W_draw(void) {
  W_adjust();
  V_setrect(0,WD,w_o+1,HT);
  if(w_horiz) {


      //V_pic(127-(word)(w_x-WD/2)*56U/(word)(MAXX-WD/2),w_o+123-(word)(w_y-HT/2)*28U/(word)(MAXY-HT/2),horiz);
      vgaimg *img = (vgaimg*)horiz;
      int x=0;
      int d=0;
      do {
         int y=w_o;
         d &= ~2;
         do {
             V_rotspr(x,y,img,d);
             y+=img->h;
             d^=2;
         } while (y<HT+w_o);
         x+=img->w;
         d^=1;
      } while (x<WD);



    if(sky_type==2) if(lt_time<0) {
      if(!lt_side) V_spr(0,w_o+lt_ypos,ltn[lt_type][(lt_time<-5)?0:1]);
      else V_spr2(WD-1,w_o+lt_ypos,ltn[lt_type][(lt_time<-5)?0:1]);
    }
  }else V_clr(0,WD,w_o+1,HT,0x97);
  Z_drawfld((byte *)fldb, 1);
  DOT_draw();
  IT_draw();
  PL_draw(&pl1);
  if(_2pl) PL_draw(&pl2);
  MN_draw();
  WP_draw();
  SMK_draw();
  FX_draw();
  Z_drawfld((byte *)fldf, 0);
  if(sky_type==2)
    if(lt_time==-4 || lt_time==-2)
      V_remap_rect(0,WD,w_o+1,HT,clrmap+256*11);
}


void W_init(void) {
  int i,j;
  static char *anm[ANIT-1][5]={
    {"WALL22_1","WALL23_1","WALL23_2",NULL,NULL},
    {"WALL58_1","WALL58_2","WALL58_3",NULL,NULL},
    {"W73A_1","W73A_2",NULL,NULL,NULL},
    {"RP2_1","RP2_2","RP2_3","RP2_4",NULL}
  };

  for(i=1;i<ANIT;++i) {
    for(j=0;anm[i-1][j];++j)
      anih[i][j]=F_getresid(anm[i-1][j]);
    for(;j<5;++j) anih[i][j]=-1;
  }
  memset(anic,0,sizeof(anic));
  DOT_init();
  SMK_init();
  FX_init();
  WP_init();
  IT_init();
  SW_init();
  PL_init();
  MN_init();
  M_unlock(horiz);
  horiz=M_lock(F_getresid("RSKY1"));

  free_chunks();
}

void W_act(void) {
  int i,a;

  if(g_time%3!=0) return;
  for(i=1;i<256;++i) if((a=walani[i])!=0) {
    if(anih[a][++anic[a]]==-1) anic[a]=0;
    walp[i]=M_lock(anih[a][anic[a]]);
  }
}

static void unpack(void *buf,int len,void *obuf) {
  byte *p,*o;
  int l,n;

  for(p=(byte*)buf,o=(byte*)obuf,l=len;l;++p,--l) if(*p==255) {
    n=*((word*)(++p));memset(o,*(p+=2),n);o+=n;l-=3;
  }else *(o++)=*p;
}

int W_load(FILE* h) {
  int i,j,k,g;
  static wall_t w;
  void *p,*buf;

  switch(blk.t) {
	case MB_WALLNAMES:
	  for(i=0;i<256;++i) {walh[i]=-1;walswp[i]=i;walani[i]=0;}
	  for(i=1;i<256 && blk.sz>0;++i,blk.sz-=sizeof(w)) {
		myfread(&w,1,sizeof(w),h);

                if(strncasecmp(w.n,"_WATER_",7)==0) //if(memicmp(w.n,"_WATER_",7)==0)
		  {walp[i]=(void*)(w.n[7]-'0'+1);walh[i]=-2;continue;}
		walp[i]=M_lock(walh[i]=F_getresid(w.n));
		if(w.n[0]=='S' && w.n[1]=='W' && w.n[4]=='_') walswp[i]=0;
		walf[i]=(w.t)?1:0;if(w.t) walh[i]|=0x8000;

                if(strncasecmp(w.n,"VTRAP01",8)==0) walf[i]|=2; //if(memicmp(w.n,"VTRAP01",8)==0) walf[i]|=2;
		walani[i]=getani(w.n);
	  }
	  for(j=i,i=1;i<256;++i) if(walswp[i]==0) {
		if(j>=256) break;
		F_getresname(w.n,walh[i]&0x7FFF);
		w.n[5]^=1;
		g=F_getresid(w.n)|(walh[i]&0x8000);
		for(k=1;k<256;++k) if(walh[k]==g) break;
		if(k>=256) {
		  walh[k=j++]=g;walp[k]=M_lock(g);
		  walf[k]=(g&0x8000)?1:0;
		}
		walswp[i]=k;walswp[k]=i;
	  }
	  return 1;
	case MB_BACK:  p=fldb;goto unp;
	case MB_WTYPE: p=fld;goto unp;
	case MB_FRONT: p=fldf;
	unp: switch(blk.st) {
	    case 0: myfread(p,1,FLDW*FLDH,h);break;
	    case 1:
	      if(!(buf=malloc(blk.sz)))
	        ERR_fatal("Не хватает памяти");
	      myfread(buf,1,blk.sz,h);
	      unpack(buf,blk.sz,p);free(buf);break;
	    default: return 0;
	  }return 1;
	case MB_SKY:
	  sky_type=0;myfread(&sky_type,1,2,h);
	  strcpy(w.n,"RSKY1");w.n[4]=sky_type+'0';
	  M_unlock(horiz);
	  horiz=M_lock(F_getresid(w.n));
	  return 1;
  }return 0;
}
