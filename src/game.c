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
#include <stdlib.h>
#include <string.h>
#include "files.h"
#include "memory.h"
#include "keyb.h"
#include "sound.h"
#include "view.h"
#include "bmap.h"
#include "fx.h"
#include "switch.h"
#include "weapons.h"
#include "items.h"
#include "dots.h"
#include "smoke.h"
#include "player.h"
#include "monster.h"
#include "menu.h"
#include "misc.h"
#include "map.h"
#include "my.h"

#include <SDL_keysym.h>

#define LT_DELAY 8
#define LT_HITTIME 6

#define GETIME 1092


int A8_start(char*);
int A8_nextframe(void);
void A8_close(void);


byte transdraw=0;


void FX_trans1(int t);
extern unsigned char fx_scr1[64000],fx_scr2[64000];

extern short lastkey;


extern int hit_xv,hit_yv;

//void setgamma(int);

extern int PL_JUMP;

extern map_block_t blk;

extern byte cheat;

byte _2pl=0,g_dm=0,g_st=GS_TITLE,g_exit=0,g_map=1,_warp=0;
char g_music[8]="MENU";
byte _net=0;
int g_sttm=1092;
dword g_time;
int dm_pnum,dm_pl1p,dm_pl2p;
pos_t dm_pos[100];

static void *telepsnd;
void *scrnh[3];
void *cd_scr;

extern int sky_type;
void *ltn[2][2];
int lt_time,lt_type,lt_side,lt_ypos,lt_force;
void *ltnsnd[2];

int g_trans=0,g_transt;

static void set_trans(int st) {
  switch(g_st) {
    case GS_ENDANIM: case GS_END2ANIM: case GS_DARKEN:
    case GS_BVIDEO: case GS_EVIDEO: case GS_END3ANIM:
      g_st=st;return;
  }
  switch(g_st=st) {
    case GS_ENDANIM: case GS_END2ANIM: case GS_DARKEN:
    case GS_BVIDEO: case GS_EVIDEO: case GS_END3ANIM:
      return;
  }
  g_trans=1;g_transt=0;
}

void G_savegame(FILE* h) {
  myfwrite8(_2pl, h);
  myfwrite8(g_dm, h);
  myfwrite8(g_exit, h);
  myfwrite8(g_map, h);
  myfwrite32(g_time, h);
  myfwrite32(dm_pl1p, h);
  myfwrite32(dm_pl2p, h);
  myfwrite32(dm_pnum, h);
  int i = 0;
  while (i < dm_pnum) {
    myfwrite32(dm_pos[i].x, h);
    myfwrite32(dm_pos[i].y, h);
    myfwrite8(dm_pos[i].d, h);
    i += 1;
  }
  myfwrite8(cheat, h);
  myfwrite(g_music, 8, 1, h);
}

void G_loadgame(FILE* h) {
  _2pl = myfread8(h);
  g_dm = myfread8(h);
  g_exit = myfread8(h);
  g_map = myfread8(h);
  g_time = myfread32(h);
  dm_pl1p = myfread32(h);
  dm_pl2p = myfread32(h);
  dm_pnum = myfread32(h);
  int i = 0;
  while (i < dm_pnum) {
    dm_pos[i].x = myfread32(h);
    dm_pos[i].y = myfread32(h);
    dm_pos[i].d = myfread8(h);
    i += 1;
  }
  cheat = myfread8(h);
  myfread(g_music, 8, 1, h);
  F_loadmus(g_music);
}

int G_load (FILE *h) {
  switch (blk.t) {
	case MB_MUSIC:
	  myfread(g_music, 8, 1, h);
	  if (music_random) {
      F_randmus(g_music);
    }
    F_loadmus(g_music);
	  return 1;
  }
  return 0;
}

void load_game(int n) {
  F_freemus();
  W_init();
  F_loadgame(n);
  set_trans(GS_GAME);
  pl1.drawst=0xFF;
  if(_2pl) pl2.drawst=0xFF;
  BM_remapfld();
  BM_clear(BM_PLR1|BM_PLR2|BM_MONSTER);
  BM_mark(&pl1.o,BM_PLR1);
  if(_2pl) BM_mark(&pl2.o,BM_PLR2);
  MN_mark();
  S_startmusic(music_time);
}

void G_start(void) {
  char s[8];

  F_freemus();
  sprintf(s,"MAP%02u",(word)g_map);
  F_loadmap(s);
  set_trans(GS_GAME);
  pl1.drawst=0xFF;
  if(_2pl) pl2.drawst=0xFF;
  g_exit=0;
  itm_rtime=(g_dm)?1092:0;
  p_immortal=0;PL_JUMP=10;
  g_time=0;
  lt_time=1000;
  lt_force=1;
  if(!_2pl) pl1.lives=3;
  BM_remapfld();
  BM_clear(BM_PLR1|BM_PLR2|BM_MONSTER);
  BM_mark(&pl1.o,BM_PLR1);
  if(_2pl) BM_mark(&pl2.o,BM_PLR2);
  MN_mark();
  S_startmusic(music_time);
}

#define GGAS_TOTAL (MN__LAST-MN_DEMON+16+10)

void G_init(void) {
  int i,j;
  char s[9];

  logo("G_init: настройка ресурсов игры ");
  logo_gas(5,GGAS_TOTAL);
  telepsnd=Z_getsnd("TELEPT");
  ltnsnd[0]=Z_getsnd("THUND1");
  ltnsnd[1]=Z_getsnd("THUND2");
  DOT_alloc();
  SMK_alloc();
  FX_alloc();
  WP_alloc();
  IT_alloc();
  SW_alloc();
  PL_alloc();
  MN_alloc();
  Z_initst();
  logo_gas(GGAS_TOTAL,GGAS_TOTAL);
  logo("\n");
  GM_init();
  pl1.color=0x70;
  pl2.color=0x60;
  g_trans=0;
}

int G_beg_video(void) {
/*
  switch(g_map) {
    case 3: return A8_start("FALL");
    case 4: return A8_start("KORIDOR");
    case 5: return A8_start("SKULL");
    case 6: return A8_start("TORCHES");
    case 7: return A8_start("CACO");
    case 8: return A8_start("DARTS");
    case 9: return A8_start("FISH");
    case 10: return A8_start("TRAP");
    case 11: return A8_start("JAIL");
    case 12: return A8_start("MMON1");
    case 13: return A8_start("TOWER");
    case 14: return A8_start("SAPOG");
    case 15: return A8_start("SWITCH");
    case 16: return A8_start("ACCEL");
    case 17: return A8_start("MEAT");
    case 18: return A8_start("LEGION");
    case 19: return A8_start("CLOUDS");
  }
*/
  return 0;
}


int G_end_video(void) {
/*
  switch(g_map) {
    case 1: return A8_start("TRUBA");
    case 10: return A8_start("GOTCHA");
  }
*/
  return 0;
}


void G_act(void) {
  static byte pcnt=0;
/*
  if(g_trans) {
    if(g_transt==0) {
      V_setscr(NULL);memcpy(fx_scr1,scra,64000);
      V_setscr(fx_scr2);
      transdraw=1;G_draw();transdraw=0;
      V_setscr(scrbuf);
    }
    FX_trans1(g_transt*2);
    V_copytoscr(0,320,0,200);
    if(++g_transt>32) {
      g_trans=0;
    }
    return;
    g_trans=0;
  }
*/ g_trans=0;

  
  if(g_st==GS_BVIDEO || g_st==GS_EVIDEO) {
    if(!A8_nextframe() || lastkey==SDLK_ESCAPE) {
      if(lastkey==SDLK_ESCAPE) lastkey=0;
      A8_close();
      if(g_st==GS_BVIDEO) G_start();
      else goto inter;
    }
//    V_copytoscr(0,SCRW,0,SCRH);//V_copytoscr(0,320,0,200);
    return;
  }else if(g_st==GS_ENDANIM || g_st==GS_END2ANIM || g_st==GS_END3ANIM) {
    if(!A8_nextframe()) {
      switch(g_st) {
        case GS_ENDANIM: g_st=GS_DARKEN;break;
        case GS_END2ANIM: g_st=GS_END3ANIM;A8_start("KONEC");break;
        case GS_END3ANIM: g_st=GS_ENDSCR;lastkey=0;break;
      }g_sttm=0;return;
    }
//    V_copytoscr(0,SCRW,0,SCRH);//V_copytoscr(0,320,0,200);
    return;
  }else if(g_st==GS_DARKEN) {
      g_st=GS_END2ANIM;A8_start("CREDITS");
    return;
  }
  
  if(GM_act()) return;

  

  switch(g_st) {
	case GS_TITLE: case GS_ENDSCR:
            
	  return;
	case GS_INTER:
#ifdef DEMO
	  if(keys[0x39] || keys[0x1C] || keys[0x9C]) {
	    set_trans(GS_TITLE);
	  }
#else
          if(keys[SDLK_SPACE] || keys[SDLK_RETURN] || keys[SDLK_KP_ENTER])//if(keys[0x39] || keys[0x1C] || keys[0x9C])
	    if(!G_beg_video()) G_start(); else {
	      g_st=GS_BVIDEO;F_freemus();
	    }
#endif
	  return;
  }


  if(sky_type==2) {
    if(lt_time>LT_DELAY || lt_force) {
      if(!(rand()&31) || lt_force) {
        lt_force=0;
        lt_time=-LT_HITTIME;
        lt_type=rand()%2;
        lt_side=rand()&1;
        lt_ypos=rand()&31;
        Z_sound(ltnsnd[rand()&1],128);
      }
    }else ++lt_time;
  }
  ++g_time;
  pl1.hit=0;pl1.hito=-3;
  if(_2pl) {pl2.hit=0;pl2.hito=-3;}
  G_code();

  IT_act();
  SW_act();
  if(_2pl) {
	if(pcnt) {PL_act(&pl1);PL_act(&pl2);}
	else {PL_act(&pl2);PL_act(&pl1);}
	pcnt^=1;
  }else PL_act(&pl1);
  MN_act();
  if(fld_need_remap) BM_remapfld();
  BM_clear(BM_PLR1|BM_PLR2|BM_MONSTER);
  BM_mark(&pl1.o,BM_PLR1);
  if(_2pl) BM_mark(&pl2.o,BM_PLR2);
  MN_mark();
  WP_act();
  DOT_act();
  SMK_act();
  FX_act();
  if(_2pl) {
	PL_damage(&pl1);PL_damage(&pl2);
	if(!(pl1.f&PLF_PNSND) && pl1.pain) PL_cry(&pl1);
	if(!(pl2.f&PLF_PNSND) && pl2.pain) PL_cry(&pl2);
	if((pl1.pain-=5) < 0) {pl1.pain=0;pl1.f&=(0xFFFF-PLF_PNSND);}
	if((pl2.pain-=5) < 0) {pl2.pain=0;pl2.f&=(0xFFFF-PLF_PNSND);}
  }else{
	PL_damage(&pl1);
	if(!(pl1.f&PLF_PNSND) && pl1.pain) PL_cry(&pl1);
	if((pl1.pain-=5) < 0) {pl1.pain=0;pl1.f&=(0xFFFF-PLF_PNSND);}
  }
  if(g_exit==1) {

	if(G_end_video()) {
	  F_freemus();
	  g_st=GS_EVIDEO;
	  return;
	}

inter:
	switch(g_map) {
	  case 19: g_st=GS_ENDANIM;A8_start("FINAL");break;
	  case 31: case 32: g_map=16;set_trans(GS_INTER);break;
	  default: ++g_map;set_trans(GS_INTER);break;
	}
	F_freemus();
	if(g_st==GS_INTER) {
	  F_loadmus("INTERMUS");
        }else {F_loadmus("\x8a\x8e\x8d\x85\x96\x0");if(mus_vol>0) {S_volumemusic(128);} }
	S_startmusic(0);
  }else if(g_exit==2) {
	switch(g_map) {
	  case 31: g_map=32;set_trans(GS_INTER);break;
	  case 32: g_map=16;set_trans(GS_INTER);break;
	  default: g_map=31;set_trans(GS_INTER);break;
	}
	F_freemus();
	F_loadmus("INTERMUS");
	S_startmusic(0);
  }
  
#ifdef DEMO
  if(g_dm && g_time>10920) {set_trans(GS_INTER);}
#endif
}

void G_respawn_player(player_t *p) {
  int i;

  if(dm_pnum==2) {
    if(p==&pl1) i=dm_pl1p^=1;
    else i=dm_pl2p^=1;
    p->o.x=dm_pos[i].x;p->o.y=dm_pos[i].y;p->d=dm_pos[i].d;
    FX_tfog(dm_pos[i].x,dm_pos[i].y);Z_sound(telepsnd,128);
    return;
  }
  do{i=myrand(dm_pnum);}while(i==dm_pl1p || i==dm_pl2p);
  p->o.x=dm_pos[i].x;p->o.y=dm_pos[i].y;p->d=dm_pos[i].d;
  if(p==&pl1) dm_pl1p=i; else dm_pl2p=i;
  FX_tfog(dm_pos[i].x,dm_pos[i].y);Z_sound(telepsnd,128);
}
