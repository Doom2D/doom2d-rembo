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
#include <string.h>
#include "vga.h"
#include "keyb.h"
#include "view.h"
#include "dots.h"
#include "smoke.h"
#include "weapons.h"
#include "monster.h"
#include "fx.h"
#include "items.h"
#include "switch.h"
#include "player.h"
#include "misc.h"
#include "my.h"

extern int hit_xv,hit_yv;

#define PL_RAD 8
#define PL_HT 26

#define PL_SWUP 4
#define PL_FLYUP 4

#define PL_AQUA_AIR 1091

byte p_immortal=0,p_fly=0;

int PL_JUMP=10,PL_RUN=8;

int wp_it[11]={0,I_CSAW,0,I_SGUN,I_SGUN2,I_MGUN,I_LAUN,I_PLAS,I_BFG,I_GUN2,0};

enum{STAND,GO,DIE,SLOP,DEAD,MESS,OUT,FALL};

typedef void fire_f(int,int,int,int,int);

player_t pl1,pl2;
static int aitime;
static void *aisnd[3];
static void *pdsnd[5];

static void *snd[11];
byte plr_goanim[]="BDACDA",
  plr_dieanim[]="HHHHIIIIJJJJKKKKLLLLMMMM",
  plr_slopanim[]="OOPPQQRRSSTTUUVVWW";


#pragma pack(1)
struct {
    int ku,kd,kl,kr,kf,kj,kwl,kwr,kp;
} _keys;
#pragma pack()

void PL_save_player (player_t *p, FILE *h) {
  myfwrite32(p->o.x, h);
  myfwrite32(p->o.y, h);
  myfwrite32(p->o.xv, h);
  myfwrite32(p->o.yv, h);
  myfwrite32(p->o.vx, h);
  myfwrite32(p->o.vy, h);
  myfwrite32(p->o.r, h);
  myfwrite32(p->o.h, h);
  myfwrite32(p->looky, h);
  myfwrite32(p->st, h);
  myfwrite32(p->s, h);
  myfwrite32(p->life, h);
  myfwrite32(p->armor, h);
  myfwrite32(p->hit, h);
  myfwrite32(p->hito, h);
  myfwrite32(p->pain, h);
  myfwrite32(p->air, h);
  myfwrite32(p->invl, h);
  myfwrite32(p->suit, h);
  myfwrite8(p->d, h);
  myfwrite32(p->frag, h);
  myfwrite32(p->ammo, h);
  myfwrite32(p->shel, h);
  myfwrite32(p->rock, h);
  myfwrite32(p->cell, h);
  myfwrite32(p->fuel, h);
  myfwrite32(p->kills, h);
  myfwrite32(p->secrets, h);
  myfwrite8(p->fire, h);
  myfwrite8(p->cwpn, h);
  myfwrite8(p->csnd, h);
  myfwrite8(p->amul, h);
  myfwrite16(p->wpns, h);
  myfwrite8(p->wpn, h);
  myfwrite8(p->f, h);
  myfwrite8(p->drawst, h);
  myfwrite8(p->color, h);
  myfwrite32(p->id, h);
  myfwrite8(p->keys, h);
  myfwrite8(p->lives, h);
  // k* not saved
}

void PL_savegame (FILE *h) {
  PL_save_player(&pl1, h);
  if (_2pl) {
    PL_save_player(&pl2, h);
  }
  myfwrite32(PL_JUMP, h);
  myfwrite32(PL_RUN, h);
  myfwrite8(p_immortal, h);
}

void PL_load_player (player_t *p, FILE *h) {
  p->o.x = myfread32(h);
  p->o.y = myfread32(h);
  p->o.xv = myfread32(h);
  p->o.yv = myfread32(h);
  p->o.vx = myfread32(h);
  p->o.vy = myfread32(h);
  p->o.r = myfread32(h);
  p->o.h = myfread32(h);
  p->looky = myfread32(h);
  p->st = myfread32(h);
  p->s = myfread32(h);
  p->life = myfread32(h);
  p->armor = myfread32(h);
  p->hit = myfread32(h);
  p->hito = myfread32(h);
  p->pain = myfread32(h);
  p->air = myfread32(h);
  p->invl = myfread32(h);
  p->suit = myfread32(h);
  p->d = myfread8(h);
  p->frag = myfread32(h);
  p->ammo = myfread32(h);
  p->shel = myfread32(h);
  p->rock = myfread32(h);
  p->cell = myfread32(h);
  p->fuel = myfread32(h);
  p->kills = myfread32(h);
  p->secrets = myfread32(h);
  p->fire = myfread8(h);
  p->cwpn = myfread8(h);
  p->csnd = myfread8(h);
  p->amul = myfread8(h);
  p->wpns = myfread16(h);
  p->wpn = myfread8(h);
  p->f = myfread8(h);
  p->drawst = myfread8(h);
  p->color = myfread8(h);
  p->id = myfread32(h);
  p->keys = myfread8(h);
  p->lives = myfread8(h);
  // k* not saved
}

void PL_loadgame (FILE *h) {
  PL_load_player(&pl1, h);
  if (_2pl) {
    PL_load_player(&pl2, h);
  }
  PL_JUMP = myfread32(h);
  PL_RUN = myfread32(h);
  p_immortal = myfread8(h);
}

static int nonz(int a) {return (a)?a:1;}

static int firediry(player_t *p) {
  if(p->f&PLF_UP) return -42;
  if(p->f&PLF_DOWN) return 19;
  return 0;
}

static void fire(player_t *p) {
  static fire_f *ff[11]={
    WP_pistol,WP_pistol,WP_pistol,WP_shotgun,WP_dshotgun,
    WP_mgun,WP_rocket,WP_plasma,WP_bfgshot,WP_shotgun,WP_pistol};
  static int ft[11]={5,2,6,18,36,2,12,2,0,2,1};

  if(p->cwpn) return;
  if(p->wpn==8) {
    if(!p->fire)
      if(keys[p->kf] && p->cell>=40)
	{Z_sound(snd[5],128);p->fire=21;p->cell-=40;p->drawst|=PL_DRAWWPN;return;}
      else return;
    if(p->fire==1) p->cwpn=12;
    else return;
  }else if(p->wpn==1) {
    if(!p->csnd) {
      if(!keys[p->kf]) {Z_sound(snd[7],128);p->csnd=13;return;}
    }
    if(keys[p->kf] && !p->fire) {
      p->fire=2;
	  WP_chainsaw(p->o.x+((p->d)?4:-4),p->o.y,(g_dm)?9:3,p->id);
      if(!p->csnd) {Z_sound(snd[8],128);p->csnd=29;}
    }return;
  }else if(p->fire) return;
  if(keys[p->kf] || p->wpn==8) {
    switch(p->wpn) {
      case 2: case 5:
	if(!p->ammo) return;
	--p->ammo;p->drawst|=PL_DRAWWPN;break;
      case 3: case 9:
	if(!p->shel) return;
	--p->shel;p->drawst|=PL_DRAWWPN;break;
      case 4:
	if(p->shel<2) return;
	p->shel-=2;p->drawst|=PL_DRAWWPN;break;
      case 6:
	if(!p->rock) return;
	--p->rock;p->drawst|=PL_DRAWWPN;break;
      case 7:
	if(!p->cell) return;
	--p->cell;p->drawst|=PL_DRAWWPN;break;
      case 10:
	if(!p->fuel) return;
	--p->fuel;p->drawst|=PL_DRAWWPN;break;
    }
    if(p->wpn==10)
      WP_ognemet(p->o.x,p->o.y-15,p->o.x+((p->d)?30:-30),p->o.y-15+firediry(p),
        p->o.xv+p->o.vx,p->o.yv+p->o.vy,p->id);
    else if(p->wpn>=1) ff[p->wpn] (p->o.x,p->o.y-15,p->o.x+((p->d)?30:-30),
      p->o.y-15+firediry(p),p->id);
    else WP_punch(p->o.x+((p->d)?4:-4),p->o.y,3,p->id);
    p->fire=ft[p->wpn];
    if(p->wpn>=2) p->f|=PLF_FIRE;
  }
}

static void chgwpn(player_t *p) {
  if(p->cwpn) return;
  if(p->fire && p->wpn!=1) return;
  if(keys[p->kwl]) {
	do{ if(--p->wpn<0) p->wpn=10; }while(!(p->wpns&(1<<p->wpn)));
	p->cwpn=3;
  }else if(keys[p->kwr]) {
	do{ if(++p->wpn>10) p->wpn=0; }while(!(p->wpns&(1<<p->wpn)));
	p->cwpn=3;
  }
  if(p->cwpn) {
	p->drawst|=PL_DRAWWPN;p->fire=0;
	if(p->wpn==1) Z_sound(snd[6],128);
  }
}

static void jump(player_t *p,int st) {
  if(Z_canbreathe(p->o.x,p->o.y,p->o.r,p->o.h)) {
	if(p->air<PL_AIR) {p->air=PL_AIR;p->drawst|=PL_DRAWAIR;}
  }else {
	if(--p->air < -9) {
	  p->air=0;
	  PL_hit(p,10,-3,HIT_WATER);
	}else if((p->air&31)==0) {
	  FX_bubble(p->o.x,p->o.y-20,0,0,5);
	}
	p->drawst|=PL_DRAWAIR;
  }
  if(keys[p->kj]) {
    if(p_fly) {
      p->o.yv=-PL_FLYUP;
    }else{
      if(Z_canstand(p->o.x,p->o.y,p->o.r)) p->o.yv=-PL_JUMP;
      else if(st&Z_INWATER) p->o.yv=-PL_SWUP;
    }
  }
}

int PL_isdead(player_t *p) {
  switch(p->st) {
	case DEAD: case MESS:
	case OUT:
	  return 1;
  }
  return 0;
}

void PL_init(void) {
  p_immortal=0;
  PL_JUMP=10;PL_RUN=8;
  aitime=0;
}

void PL_alloc(void) {
  int i,j;
  static char nm[][6]={
	"OOF",
	"PLPAIN",
	"PLDETH",
	"SLOP",
	"PDIEHI",
	"BFG",
	"SAWUP",
	"SAWIDL",
	"SAWFUL",
	"SAWHIT",
	"PLFALL"
  };
  char s[6];
  for(i=0;i<11;++i) snd[i]=Z_getsnd(nm[i]);
  memcpy(s,"AIx",4);
  for(i=0;i<3;++i) {
    s[2]=i+'1';
    aisnd[i]=Z_getsnd(s);
  }
  memcpy(s,"PLDTHx",6);
  for(i=0;i<5;++i) {
    s[5]=i+'1';
    pdsnd[i]=Z_getsnd(s);
  }
}

void PL_restore(player_t *p) {
  p->o.xv=p->o.yv=p->o.vx=p->o.vy=0;
  p->o.r=PL_RAD;p->o.h=PL_HT;
  p->pain=0;
  p->invl=p->suit=0;
  switch(p->st) {
    case DEAD: case MESS: case OUT:
    case DIE: case SLOP: case FALL:
      p->life=100;p->armor=0;p->air=PL_AIR;
      p->wpns=5;
      p->wpn=2;
      p->ammo=50;p->fuel=p->shel=p->rock=p->cell=0;
      p->amul=1;
  }
  p->st=STAND;
  p->fire=p->cwpn=p->csnd=0;
  p->f=0;
  p->drawst=0xFF;
  p->looky=0;
  p->keys=(g_dm)?0x70:0;
}

void PL_reset(void) {
  pl1.st=pl2.st=DEAD;
  pl1.frag=pl2.frag=0;
}

void PL_spawn(player_t *p,int x,int y,char d) {
  PL_restore(p);
  p->o.x=x;p->o.y=y;p->d=d;
  p->kills=p->secrets=0;
}

int PL_hit(player_t *p,int d,int o,int t) {
  if(!d) return 0;
  switch(p->st) {
    case DIE: case SLOP:
	case DEAD: case MESS:
    case OUT: case FALL:
      return 0;
  }
  if(t==HIT_TRAP) {if(!p_immortal) {p->armor=0;p->life=-100;}}
  else if(t!=HIT_ROCKET && t!=HIT_ELECTRO) {
    if(p->id==-1) {if(o==-1) return 0;}
	else if(o==-2) return 0;
  }
  if(t!=HIT_WATER && t!=HIT_ELECTRO)
	DOT_blood(p->o.x,p->o.y-15,hit_xv,hit_yv,d*2);
	else if(t==HIT_WATER) FX_bubble(p->o.x,p->o.y-20,0,0,d/2);
  if(p_immortal || p->invl) return 1;
  p->hit+=d;
  p->hito=o;
  return 1;
}

void PL_damage(player_t *p) {
  int i;

  if(!p->hit && p->life>0) return;
  switch(p->st) {
    case DIE: case SLOP:
	case DEAD: case MESS:
    case OUT: case FALL:
	  return;
  }
  i=p->hit*p->life/nonz(p->armor*3/4+p->life);
  p->pain+=p->hit;
  p->drawst|=PL_DRAWLIFE|PL_DRAWARMOR;
  if((p->armor-=p->hit-i)<0) {p->life+=p->armor;p->armor=0;}
  if((p->life-=i)<=0) {
    if(p->life>-30) {p->st=DIE;p->s=0;Z_sound(pdsnd[rand()%5],128);}
	else {p->st=SLOP;p->s=0;Z_sound(snd[3],128);}
	if(p->amul>1) IT_spawn(p->o.x,p->o.y,I_BPACK);
	if(!g_dm) {
	  if(p->keys&16) IT_spawn(p->o.x,p->o.y,I_KEYR);
	  if(p->keys&32) IT_spawn(p->o.x,p->o.y,I_KEYG);
	  if(p->keys&64) IT_spawn(p->o.x,p->o.y,I_KEYB);
	}
	for(i=1,p->wpns>>=1;i<11;++i,p->wpns>>=1)
	  if(i!=2) if(p->wpns&1) IT_spawn(p->o.x,p->o.y,wp_it[i]);
	p->wpns=5;p->wpn=2;
	p->f|=PLF_PNSND;
	p->drawst|=PL_DRAWWPN;
    if(g_dm && _2pl) {
	  if(p->id==-1) {
		if(p->hito==-2) {++pl2.kills;++pl2.frag;}
		else if(p->hito==-1) --pl1.frag;
	  }else{
	    if(p->hito==-1) {++pl1.kills;++pl1.frag;}
	    else if(p->hito==-2) --pl2.frag;
	  }
	  pl1.drawst|=PL_DRAWFRAG;
	  if(_2pl) pl2.drawst|=PL_DRAWFRAG;
	}
	p->life=0;return;
  }
  return;
}

void PL_cry(player_t *p) {
  Z_sound(snd[(p->pain>20)?1:0],128);
  p->f|=PLF_PNSND;
}

int PL_give(player_t *p,int t) {
  int i;

  switch(p->st) {
    case DIE: case SLOP:
    case DEAD: case MESS: case OUT:
      return 0;
  }
  switch(t) {
    case I_STIM: case I_MEDI:
      if(p->life>=100) return 0;
      if((p->life+=((t==I_MEDI)?25:10))>100) p->life=100;
      p->drawst|=PL_DRAWLIFE;return 1;
    case I_CLIP:
      if(p->ammo>=200*p->amul) return 0;
      if((p->ammo+=10)>200*p->amul) p->ammo=200*p->amul;
      p->drawst|=PL_DRAWWPN;return 1;
    case I_AMMO:
      if(p->ammo>=200*p->amul) return 0;
      if((p->ammo+=50)>200*p->amul) p->ammo=200*p->amul;
	  p->drawst|=PL_DRAWWPN;return 1;
    case I_SHEL:
      if(p->shel>=50*p->amul) return 0;
      if((p->shel+=4)>50*p->amul) p->shel=50*p->amul;
      p->drawst|=PL_DRAWWPN;return 1;
    case I_SBOX:
      if(p->shel>=50*p->amul) return 0;
      if((p->shel+=25)>50*p->amul) p->shel=50*p->amul;
      p->drawst|=PL_DRAWWPN;return 1;
    case I_ROCKET:
      if(p->rock>=50*p->amul) return 0;
      if((++p->rock)>50*p->amul) p->rock=50*p->amul;
      p->drawst|=PL_DRAWWPN;return 1;
    case I_RBOX:
      if(p->rock>=50*p->amul) return 0;
      if((p->rock+=5)>50*p->amul) p->rock=50*p->amul;
      p->drawst|=PL_DRAWWPN;return 1;
    case I_CELL:
      if(p->cell>=300*p->amul) return 0;
      if((p->cell+=40)>300*p->amul) p->cell=300*p->amul;
      p->drawst|=PL_DRAWWPN;return 1;
    case I_CELP:
      if(p->cell>=300*p->amul) return 0;
      if((p->cell+=100)>300*p->amul) p->cell=300*p->amul;
      p->drawst|=PL_DRAWWPN;return 1;
    case I_BPACK:
      if(p->amul==1) {p->amul=2;i=1;} else i=0;
      i|=PL_give(p,I_CLIP);
      i|=PL_give(p,I_SHEL);
      i|=PL_give(p,I_ROCKET);
      i|=PL_give(p,I_CELL);
      return i;
    case I_CSAW:
      if(!(p->wpns&2)) {p->wpns|=2;p->drawst|=PL_DRAWWPN;return 1;}
      return 0;
    case I_GUN2:
      i=PL_give(p,I_SHEL);
      if(!(p->wpns&512)) {p->wpns|=512;i=1;}
      p->drawst|=PL_DRAWWPN;return i;
    case I_SGUN:
      i=PL_give(p,I_SHEL);
      if(!(p->wpns&8)) {p->wpns|=8;i=1;}
      p->drawst|=PL_DRAWWPN;return i;
    case I_SGUN2:
      i=PL_give(p,I_SHEL);
      if(!(p->wpns&16)) {p->wpns|=16;i=1;}
      p->drawst|=PL_DRAWWPN;return i;
    case I_MGUN:
      i=PL_give(p,I_AMMO);
      if(!(p->wpns&32)) {p->wpns|=32;i=1;}
      p->drawst|=PL_DRAWWPN;return i;
    case I_LAUN:
      i=PL_give(p,I_ROCKET);
      i|=PL_give(p,I_ROCKET);
      if(!(p->wpns&64)) {p->wpns|=64;i=1;}
      p->drawst|=PL_DRAWWPN;return i;
    case I_PLAS:
      i=PL_give(p,I_CELL);
      if(!(p->wpns&128)) {p->wpns|=128;i=1;}
      p->drawst|=PL_DRAWWPN;return i;
    case I_BFG:
      i=PL_give(p,I_CELL);
      if(!(p->wpns&256)) {p->wpns|=256;i=1;}
      p->drawst|=PL_DRAWWPN;return i;
    case I_ARM1:
      if(p->armor>=100) return 0;
      p->armor=100;p->drawst|=PL_DRAWARMOR;return 1;
    case I_ARM2:
      if(p->armor>=200) return 0;
      p->armor=200;p->drawst|=PL_DRAWARMOR;return 1;
    case I_MEGA:
      i=0;
      if(p->life<200) {p->life=200;p->drawst|=PL_DRAWLIFE;i=1;}
      if(p->armor<200) {p->armor=200;p->drawst|=PL_DRAWARMOR;i=1;}
      return i;
    case I_SUPER:
      if(p->life<200) {p->life=min(p->life+100,200);p->drawst|=PL_DRAWLIFE;return 1;}
      return 0;
    case I_INVL:
	  p->invl=PL_POWERUP_TIME;
	  return 1;
    case I_SUIT:
	  p->suit=PL_POWERUP_TIME;
	  return 1;
    case I_AQUA:
      if(p->air >= PL_AQUA_AIR) return 0;
      p->air=PL_AQUA_AIR;p->drawst|=PL_DRAWAIR;
      return 1;
    case I_KEYR:
      if(p->keys&16) return 0;
      p->keys|=16;p->drawst|=PL_DRAWKEYS;return 1;
    case I_KEYG:
      if(p->keys&32) return 0;
      p->keys|=32;p->drawst|=PL_DRAWKEYS;return 1;
    case I_KEYB:
      if(p->keys&64) return 0;
      p->keys|=64;p->drawst|=PL_DRAWKEYS;return 1;
    default:
	  return 0;
  }
}

void PL_act(player_t *p) {
  int st;

  if(--aitime<0) aitime=0;
  SW_press(p->o.x,p->o.y,p->o.r,p->o.h,4|p->keys,p->id);
  if(!p->suit) if((g_time&15)==0)
    PL_hit(p,Z_getacid(p->o.x,p->o.y,p->o.r,p->o.h),-3,HIT_SOME);
  if(p->st!=FALL && p->st!=OUT) {
	if(((st=Z_moveobj(&p->o))&Z_FALLOUT) && p->o.y>=FLDH*CELH+50) {
	  switch(p->st) {
		case DEAD: case MESS: case DIE: case SLOP:
		  p->s=5;break;
		default:
		  p->s=Z_sound(snd[10],128);
	          if(g_dm) --p->frag;
	  }p->st=FALL;
	}
  }else st=0;
  if(st&Z_HITWATER) Z_splash(&p->o,PL_RAD+PL_HT);
  if(p->f&PLF_FIRE) if(p->fire!=2) p->f-=PLF_FIRE;
  if(keys[p->ku]) {p->f|=PLF_UP;p->looky-=5;}
  else{
    p->f&=0xFFFF-PLF_UP;
	if(keys[p->kd])
	  {p->f|=PLF_DOWN;p->looky+=5;}
	else {p->f&=0xFFFF-PLF_DOWN;p->looky=Z_dec(p->looky,5);}
  }
  if(keys[p->kp]) SW_press(p->o.x,p->o.y,p->o.r,p->o.h,1|p->keys,p->id);
  if(p->fire) --p->fire;
  if(p->cwpn) --p->cwpn;
  if(p->csnd) --p->csnd;
  if(p->invl) --p->invl;
  if(p->suit) --p->suit;
  switch(p->st) {
    case DIE:
      p->o.h=7;
      if(!plr_dieanim[++p->s]) {p->st=DEAD;MN_killedp();}
      p->o.xv=Z_dec(p->o.xv,1);
      break;
    case SLOP:
      p->o.h=6;
      if(!plr_slopanim[++p->s]) {p->st=MESS;MN_killedp();}
      p->o.xv=Z_dec(p->o.xv,1);
      break;
	case GO:
	  chgwpn(p);fire(p);jump(p,st);
	  if(p_fly)
	    SMK_gas(p->o.x,p->o.y-2,2,3,p->o.xv+p->o.vx,p->o.yv+p->o.vy,128);
	  if((p->s+=abs(p->o.xv)/2) >= 24) p->s%=24;
	  if(!keys[p->kl] && !keys[p->kr]) {
		if(p->o.xv) p->o.xv=Z_dec(p->o.xv,1);
		else p->st=STAND;
		break;
	  }
	  if(p->o.xv<PL_RUN && keys[p->kr]) {p->o.xv+=PL_RUN>>3;p->d=1;}
	    else if(PL_RUN>8)
	      SMK_gas(p->o.x,p->o.y-2,2,3,p->o.xv+p->o.vx,p->o.yv+p->o.vy,32);
	  if(p->o.xv>-PL_RUN && keys[p->kl]) {p->o.xv-=PL_RUN>>3;p->d=0;}
	    else if(PL_RUN>8)
	      SMK_gas(p->o.x,p->o.y-2,2,3,p->o.xv+p->o.vx,p->o.yv+p->o.vy,32);
	  break;
	case STAND:
	  chgwpn(p);fire(p);jump(p,st);
	  if(p_fly)
	    SMK_gas(p->o.x,p->o.y-2,2,3,p->o.xv+p->o.vx,p->o.yv+p->o.vy,128);
	  if(keys[p->kl]) {p->st=GO;p->s=0;p->d=0;}
      else if(keys[p->kr]) {p->st=GO;p->s=0;p->d=1;}
      break;
    case DEAD:
    case MESS:
    case OUT:
	  p->o.xv=Z_dec(p->o.xv,1);
	  if(keys[p->ku] || keys[p->kd] || keys[p->kl] || keys[p->kr] ||
	     keys[p->kf] || keys[p->kj] || keys[p->kp] || keys[p->kwl] || keys[p->kwr]) {
		if(p->st!=OUT) MN_spawn_deadpl(&p->o,p->color,(p->st==MESS)?1:0);
		PL_restore(p);
		if(g_dm) {G_respawn_player(p);break;}
		if(!_2pl) {
		  if(--p->lives==0) {G_start();break;}
		  else{p->o.x=dm_pos[0].x;p->o.y=dm_pos[0].y;p->d=dm_pos[0].d;}
		  p->drawst|=PL_DRAWLIVES;
		}
		if(p->id==-1)
		  {p->o.x=dm_pos[0].x;p->o.y=dm_pos[0].y;p->d=dm_pos[0].d;}
		else {p->o.x=dm_pos[1].x;p->o.y=dm_pos[1].y;p->d=dm_pos[1].d;}
	  }break;
	case FALL:
	  if(--p->s<=0) p->st=OUT;
	  break;
  }
}

static void chk_bfg(player_t *p,int x,int y) {
  int dx,dy;

  if(aitime) return;
  switch(p->st) {
    case DIE: case SLOP: case FALL:
    case DEAD: case MESS: case OUT:
      return;
  }
  dx=p->o.x-x;dy=p->o.y-p->o.h/2-y;
  if(dx*dx+dy*dy<=1600) {
    aitime=Z_sound(aisnd[rand()%3],128)*4;
  }
}

void bfg_fly(int x,int y,int o) {
//  if(!g_dm) return;
  if(o!=-1) chk_bfg(&pl1,x,y);
  if(_2pl) if(o!=-2) chk_bfg(&pl2,x,y);
  if(o==-1 || o==-2) MN_warning(x-50,y-50,x+50,y+50);
}
