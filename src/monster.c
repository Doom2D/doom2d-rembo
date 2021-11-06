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
#include <stdlib.h>
#include <string.h>
#include "files.h"
#include "view.h"
#include "bmap.h"
#include "dots.h"
#include "weapons.h"
#include "player.h"
#include "monster.h"
#include "items.h"
#include "switch.h"
#include "misc.h"
#include "fx.h"
#include "smoke.h"
#include "player.h"
#include "error.h"
#include "game.h"

#define MAX_ATM 90

enum{
  SLEEP,GO,RUN,CLIMB,DIE,DEAD,ATTACK,SHOOT,PAIN,WAIT,REVIVE,RUNOUT
};

typedef struct {
  int r, h, l, mp, rv, jv, sp, minp;
} mnsz_t;

byte nomon = 1;

static char *sleepanim[MN_TN]={
  "AAABBB","AAABBB","AAABBB","AAABBB","AAABBB","AAABBB","AAABBB","AAABBB",
  "A","AAABBB","AAABBB","AAABBB","AAABBB","AAABBB","AAABBB","AAABBB","AAABBB",
  "A","A","AAABBB"
}, *goanim[MN_TN]={
  "AABBCCDD","AABBCCDD","AABBDDAACCDD","AABBDDAACCDD","AABBDDCCDDBB",
  "AABBDDAACCDD","AABBCCDD","AABBCCDD","A","AABB","AABBCCBB",
  "AABBCCDDEEFF","AABBCCDDEEFF","AABBCCDDEEFF","AABBCCDDEEFF","AABBCCDDEEFF",
  "AABB","A","DDEEFFGGHHIIJJKKLLAABBCC","ACDABD"
}, *painanim[MN_TN]={
  "H","H","G","G","G","G","H","H","F","E","G","I","I","J","L","Q","EECCDDCC",
  "A","D","G"
}, *waitanim[MN_TN]={
  "A","A","A","A","A","A","A","A","A","AABB","A","A","A","I","K","A","A",
  "A","D","E"
}, *attackanim[MN_TN]={
  "EEFFGG","EEFFGG","EEEEEF","EEEEEF","EEEEEF","EF","EEFFGG","EEFFGG",
  "BBCCDD","CCDD","DDEEFF","GH","GH","GGGGHH","GGHHII",
  "QQGGGHHHIIJJKKLLMMNNOOPP","BBFFAA","A","OOPPQQ","EEEEFF"
}, *dieanim[MN_TN]={
  "IIIJJJKKKLLLMMM","IIIJJJKKKLLL","HHHIIIJJJKKK","HHHIIIJJJKKK",
  "HHHIIIJJJKKKLLLMMMNNNOOO","HHHIIIJJJKKKLLLMMM",
  "IIIJJJKKKLLLMMMNNN","IIIJJJKKKLLLMMMNNN","GGGHHHIIIJJJKKK",
  "FFFGGGHHHIIIJJJKKK","HHHIIIJJJKKKLLLMMM",
  "JJJJKKKKLLLLMMMMNNNNOOOOPPPPQQQQRRRR","JJJKKKLLLMMMNNNOOO",
  "KKKLLLMMMNNNOOOPPPRRRSSS","MMMNNNOOOPPP","RRRSSSTTTUUUVVVWWWXXXYYY",
  "DDDD","CCCDDDEEEFFFGGG","D","HHHHIIIIJJJJKKKKLLLLMMMM"
}, *slopanim[MN_TN]={
  "","NNNOOOPPPRRRSSSTTT","MMMNNNOOOPPPRRRSSSTTT","MMMNNNOOOPPPRRRSSSTTT","",
  "OOOPPPQQQRRRSSS","","","","","","","","","","","","","","OOPPQQRRSSTTUUVV"
}, *deadanim[MN_TN]={
  "N","M","L","L","P","N","O","O","L","","","S","P","T","Q","Z","C","","D","N"
}, *messanim[MN_TN]={
  "","U","U","U","","T","","","","","","","","","","","","","","W"
};

int hit_xv, hit_yv;
mn_t mn[MAXMN];
int mnum, gsndt;

static void *fsnd,*pauksnd,*trupsnd;
static void *snd[MN_TN][5],*impsitsnd[2],*impdthsnd[2],*firsnd,*slopsnd,*gsnd[4];
static void *swgsnd,*pchsnd,*telesnd;
static void *positsnd[3],*podthsnd[3];
static mnsz_t mnsz[MN_TN+1]={
  /* rad  ht life  pain  rv  jv slop min_pn */
  {    0,  0,   0,    0,  0,  0,   0,     0 }, // none
  {   15, 28,  60,   20,  7, 10,   0,    10 }, // demon
  {   10, 28,  25,   15,  3, 10,  30,     0 }, // imp
  {   10, 28,  15,   10,  3, 10,  30,     0 }, // zomby
  {   10, 28,  20,   10,  3, 10,  30,     0 }, // sergeant
  {   20, 55, 500,   70,  5, 10,   0,    50 }, // cyberdemon
  {   12, 28,  60,   20,  3, 10,  30,    10 }, // chaingunner
  {   12, 32, 150,   40,  3, 10,   0,    30 }, // baron of hell
  {   12, 32,  75,   40,  3, 10,   0,    30 }, // hell knight
  {   15, 28, 100,   10,  4,  4,   0,     0 }, // cacodemon
  {    8, 18,  60,   10,  4,  4,   0,     0 }, // lost soul
  {   15, 28, 100,   10,  4,  4,   0,     0 }, // pain elemental
  {   64, 50, 500,   70,  4, 10,   0,    50 }, // spider mastermind
  {   25, 27, 150,   20,  4, 10,   0,     0 }, // arachnotron
  {   18, 30, 200,   40,  3,  7,   0,    20 }, // mancubus
  {   17, 36, 200,   40,  6, 11,   0,    20 }, // revenant
  {   17, 36, 150,   30,  7, 12,   0,    10 }, // archvile
  {    5,  5,  35,   20, 14,  6,   0,    10 }, // fish
  {    5, 17,  20,    0,  7,  6,   0,     0 }, // barrel
  {   17, 38,  20,   40,  3,  6,   0,    20 }, // robot
  {    8, 26, 400,   70,  8, 10,  30,    50 }  // man
};

void setst (int i, int st) {
  char *a;
  int t;

  switch(mn[i].st) {
    case DIE: case DEAD:
      if(st!=DEAD && st!=REVIVE) return;
  }
  mn[i].ac=0;
  t=mn[i].t-1;
  switch(mn[i].st=st) {
	case SLEEP: a=sleepanim[t];break;
	case PAIN: a=painanim[t];break;
	case WAIT: a=waitanim[t];break;
	case CLIMB:
	case RUN: case RUNOUT:
	case GO: a=goanim[t];break;
	case SHOOT:
	  if(t==MN_SKEL-1) {a="KKKKJJ";break;}
	  if(t==MN_ROBO-1) {a="MN";break;}
	case ATTACK: a=attackanim[t];
	  if(st==ATTACK && t==MN_VILE-1) a="[[\\\\]]";
	  break;
	case DIE:
	  if(g_map==9 && t==MN_BSP-1) Z_sound(pauksnd,128);
	  a=dieanim[t];break;
	case DEAD:
	  a=deadanim[t];
	  if(mn[i].ap==slopanim[t]) a=messanim[t];
	  if(t==MN_BARREL-1) {mn[i].t=0;}
	  break;
	case REVIVE:
	  a=(mn[i].ap==messanim[t])?slopanim[t]:dieanim[t];
	  mn[i].ac=strlen(a)-1;
	  mn[i].o.r=mnsz[t+1].r;mn[i].o.h=mnsz[t+1].h;
	  mn[i].life=mnsz[t+1].l;mn[i].ammo=mn[i].pain=0;
	  ++mnum;
	  break;
  }
  mn[i].ap=a;
}

#define GGAS_TOTAL (MN__LAST-MN_DEMON+16+10)

void MN_alloc (void) {
  int i,j;
  static char sn[MN_TN][5][6]={
	{"DMACT","DMPAIN","SGTATK","SGTSIT","SGTDTH"},
	{"BGACT","POPAIN","CLAW","",""},
	{"POSACT","POPAIN","","",""},
	{"POSACT","POPAIN","","",""},
	{"","DMPAIN","HOOF","CYBSIT","CYBDTH"},
	{"POSACT","POPAIN","","",""},
	{"","DMPAIN","","BRSSIT","BRSDTH"},
	{"","DMPAIN","","KNTSIT","KNTDTH"},
	{"DMACT","DMPAIN","","CACSIT","CACDTH"},
	{"DMACT","DMPAIN","SKLATK","SKLATK","FIRXPL"},
	{"DMACT","PEPAIN","","PESIT","PEDTH"},
	{"","DMPAIN","METAL","SPISIT","SPIDTH"},
	{"BSPACT","DMPAIN","BSPWLK","BSPSIT","BSPDTH"},
	{"DMACT","MNPAIN","MANATK","MANSIT","MANDTH"},
	{"SKEACT","POPAIN","SKEATK","SKESIT","SKEDTH"},
	{"VILACT","VIPAIN","VILATK","VILSIT","VILDTH"},
	{"","","BITE1","",""},
	{"","","","","BAREXP"},
	{"BSPACT","","BSPWLK","BSPSIT","BSPDTH"},
	{"HAHA1","PLPAIN","","STOP1","PDIEHI"}
  };
  static char gsn[6]="GOOD0";
  for(j=0;j<MN_TN;++j) {
    for(i=0;i<5;++i)
      if(sn[j][i][0])
        snd[j][i]=Z_getsnd(sn[j][i]);
      else
        snd[j][i]=NULL;
    logo_gas(j+5,GGAS_TOTAL);
  }
  impsitsnd[0]=Z_getsnd("BGSIT1");
  impsitsnd[1]=Z_getsnd("BGSIT2");
  impdthsnd[0]=Z_getsnd("BGDTH1");
  impdthsnd[1]=Z_getsnd("BGDTH2");
  positsnd[0]=Z_getsnd("POSIT1");
  positsnd[1]=Z_getsnd("POSIT2");
  positsnd[2]=Z_getsnd("POSIT3");
  podthsnd[0]=Z_getsnd("PODTH1");
  podthsnd[1]=Z_getsnd("PODTH2");
  podthsnd[2]=Z_getsnd("PODTH3");
  fsnd=Z_getsnd("FLAME");
  firsnd=Z_getsnd("FIRSHT");
  slopsnd=Z_getsnd("SLOP");
  swgsnd=Z_getsnd("SKESWG");
  pchsnd=Z_getsnd("SKEPCH");
  telesnd=Z_getsnd("TELEPT");
  pauksnd=Z_getsnd("PAUK1");
  trupsnd=Z_getsnd("UTRUP");
  for(i=0;i<4;++i) {gsn[4]=i+'1';gsnd[i]=Z_getsnd(gsn);}
}

void MN_init (void) {
  int i;
  for(i=0;i<MAXMN;++i) {mn[i].t=0;mn[i].st=SLEEP;}
  gsndt=mnum=0;
}

int MN_spawn (int x, int y, byte d, int t) {
  int i;

  if(g_dm && nomon && t<MN_PL_DEAD) return -1;
  for(i=0;i<MAXMN;++i) if(!mn[i].t) goto ok;
  for(i=0;i<MAXMN;++i) if(mn[i].t>=MN_PL_DEAD) goto ok;
  return -1;
ok:
  mn[i].o.x=x;mn[i].o.y=y;
  mn[i].o.xv=mn[i].o.yv=mn[i].o.vx=mn[i].o.vy=0;
  mn[i].d=d;mn[i].t=t;
  mn[i].st=SLEEP;
  if(t<MN_PL_DEAD) {
    mn[i].o.r=mnsz[t].r;mn[i].o.h=mnsz[t].h;
    mn[i].life=mnsz[t].l;
    setst(i,SLEEP);mn[i].s=myrand(18);
    ++mnum;
  }else {mn[i].o.r=8;mn[i].o.h=6;mn[i].life=0;mn[i].st=DEAD;}
  mn[i].aim=-3;mn[i].atm=0;
  mn[i].pain=0;
  mn[i].ammo=0;
  mn[i].ftime=0;
  return i;
}

int MN_spawn_deadpl (obj_t *o, byte c, int t) {
  int i;

  if((i=MN_spawn(o->x,o->y,c,t+MN_PL_DEAD))==-1) return -1;
  mn[i].o=*o;return i;
}

static int isfriend(int a,int b) {
  if(a==MN_BARREL || b==MN_BARREL) return 1;
  if(a==b) switch(a) {
    case MN_IMP: case MN_DEMON:
    case MN_BARON: case MN_KNIGHT:
    case MN_CACO: case MN_SOUL:
    case MN_MANCUB: case MN_SKEL:
    case MN_FISH:
      return 1;
  }
  if(a==MN_SOUL && b==MN_PAIN) return 1;
  if(b==MN_SOUL && a==MN_PAIN) return 1;
  return 0;
}

static int MN_findnewprey(int i) {
  int a,b,l;

  a=!PL_isdead(&pl1);
  if(_2pl) b=!PL_isdead(&pl2); else b=0;
  if(a) {
	if(b) mn[i].aim=(abs(mn[i].o.x-pl1.o.x)+abs(mn[i].o.y-pl1.o.y)
	    <= abs(mn[i].o.x-pl2.o.x)+abs(mn[i].o.y-pl2.o.y))?-1:-2;
	else mn[i].aim=-1;
  }else{
	if(b) mn[i].aim=-2;
	else{
	  for(a=0,b=32000,mn[i].aim=-3;a<MAXMN;++a)
	    if(mn[a].t && mn[a].st!=DEAD && a!=i && !isfriend(mn[a].t,mn[i].t))
	      if((l=abs(mn[i].o.x-mn[a].o.x)+abs(mn[i].o.y-mn[a].o.y))<b)
	        {mn[i].aim=a;b=l;}
	  if(mn[i].aim<0) {mn[i].atm=MAX_ATM;return 0;} else mn[i].atm=0;
	}
  }
  return 1;
}

int Z_getobjpos (int i, obj_t *o) {
  if(i==-1) {*o=pl1.o;return !PL_isdead(&pl1);}
  if(_2pl) if(i==-2) {*o=pl2.o;return !PL_isdead(&pl2);}
  if(i>=0 && i<MAXMN) if(mn[i].t && mn[i].st!=DEAD)
	{*o=mn[i].o;return 1;}
  return 0;
}

static void *wakeupsnd(int t) {
  switch(t) {
	case MN_IMP: return impsitsnd[myrand(2)];
	case MN_ZOMBY: case MN_SERG: case MN_CGUN:
	  return positsnd[myrand(3)];
  }
  return snd[t-1][3];
}

static void *dthsnd(int t) {
  switch(t) {
	case MN_IMP: return impdthsnd[myrand(2)];
	case MN_ZOMBY: case MN_SERG: case MN_CGUN:
	  return podthsnd[myrand(3)];
  }
  return snd[t-1][4];
}

static int canshoot(int t) {
  switch(t) {
	case MN_DEMON: case MN_FISH: case MN_BARREL:
	  return 0;
  }
  return 1;
}

static int shoot(int i,obj_t *o,int n) {
  int xd,yd,m;

  if(mn[i].ammo<0) return 0;
  if(!n) switch(mn[i].t) {
	case MN_FISH: case MN_BARREL:
	case MN_DEMON: return 0;
	case MN_CGUN:
	case MN_BSP:
	case MN_ROBO:
	  if(++mn[i].ammo>=50) mn[i].ammo=(mn[i].t==MN_ROBO)?-200:-50;
	  break;
	case MN_MAN:
	  break;
	case MN_MANCUB:
	  if(++mn[i].ammo>=5) mn[i].ammo=-50;
	  break;
	case MN_SPIDER:
	  if(++mn[i].ammo>=100) mn[i].ammo=-50;
	  break;
	case MN_CYBER:
	  if(rand()&1) return 0;
	  if(++mn[i].ammo>=10) mn[i].ammo=-50;
	  break;
	case MN_BARON: case MN_KNIGHT:
	  if(rand()&7) return 0;
	  break;
	case MN_SKEL:
	  if(rand()&31) return 0;
	  break;
	case MN_VILE:
	  if(rand()&7) return 0;
	  break;
	case MN_PAIN:
	  if(rand()&7) return 0;
	  break;
	default:
	  if(rand()&15) return 0;
  }
  if(!Z_look(&mn[i].o,o,mn[i].d)) return 0;
  mn[i].atm=0;
  mn[i].tx=o->x+(o->xv+o->vx)*6;mn[i].ty=o->y-o->h/2+(o->yv+o->vy)*6;
  if(abs(mn[i].tx-mn[i].o.x)<abs(mn[i].ty-mn[i].o.y+mn[i].o.h/2)) return 0;
  switch(mn[i].t) {
	case MN_IMP: case MN_BARON: case MN_KNIGHT: case MN_CACO:
	  setst(i,SHOOT);Z_sound(firsnd,128);break;
	case MN_SKEL:
	  setst(i,SHOOT);Z_sound(snd[MN_SKEL-1][2],128);break;
	case MN_VILE:
	  mn[i].tx=o->x;mn[i].ty=o->y;
	  setst(i,SHOOT);Z_sound(fsnd,128);
	  Z_sound(snd[MN_VILE-1][2],128);break;
	case MN_SOUL:
	  setst(i,ATTACK);Z_sound(snd[MN_SOUL-1][2],128);
	  yd=mn[i].ty-mn[i].o.y+mn[i].o.h/2;xd=mn[i].tx-mn[i].o.x;
	  if(!(m=max(abs(xd),abs(yd)))) m=1;
	  mn[i].o.xv=xd*16/m;mn[i].o.yv=yd*16/m;
	  break;
	case MN_MANCUB: if(mn[i].ammo==1) Z_sound(snd[MN_MANCUB-1][2],128);
	case MN_ZOMBY: case MN_SERG: case MN_BSP: case MN_ROBO:
	case MN_CYBER: case MN_CGUN: case MN_SPIDER:
	case MN_PAIN: case MN_MAN:
	  setst(i,SHOOT);break;
	default:
	  return 0;
  }
  return 1;
}

static int kick(int i,obj_t *o) {
  switch(mn[i].t) {
	case MN_FISH:
	  setst(i,ATTACK);return 1;
	case MN_DEMON:
	  setst(i,ATTACK);Z_sound(snd[0][2],128);return 1;
	case MN_IMP:
	  setst(i,ATTACK);Z_sound(snd[1][2],128);return 1;
	case MN_SKEL:
	  setst(i,ATTACK);Z_sound(swgsnd,128);return 1;
	case MN_ROBO:
	  setst(i,ATTACK);Z_sound(swgsnd,128);return 1;
	case MN_BARON: case MN_KNIGHT: case MN_CACO: case MN_MANCUB:
	  return shoot(i,o,1);
  }
  return 0;
}

static int iscorpse(obj_t *o,int n) {
  int i;

  if(!n) if(rand()&7) return -3;
  for(i=0;i<MAXMN;++i) if(mn[i].t) if(mn[i].st==DEAD)
    if(Z_overlap(o,&mn[i].o)) switch(mn[i].t) {
      case MN_SOUL: case MN_PAIN:
      case MN_CYBER: case MN_SPIDER:
      case MN_PL_DEAD: case MN_PL_MESS:
      case MN_VILE: case MN_BARREL:
        continue;
      default:
        return i;
    }
  return -3;
}

static int Z_hitobj (int obj, int d, int own, int t) {
  hit_xv=hit_yv=0;
  if(obj==-1) return PL_hit(&pl1,d,own,t);
  else if(obj==-2 && _2pl) return PL_hit(&pl2,d,own,t);
  else if(obj<0 || obj>=MAXMN) return 0;
  if(mn[obj].t) return MN_hit(obj,d,own,t);
  return 0;
}

void MN_act (void) {
  int i,st,sx,sy,t;
  static obj_t o;
  static int pt_x=0,pt_xs=1,pt_y=0,pt_ys=1;

  if(abs(pt_x+=pt_xs) > 123) pt_xs=-pt_xs;
  if(abs(pt_y+=pt_ys) > 50) pt_ys=-pt_ys;
  if(gsndt>0) if(--gsndt==0) {
	Z_sound(gsnd[myrand(4)],128);
  }
  for(i=0;i<MAXMN;++i) if((t=mn[i].t)!=0) {
  switch(t) {
	case MN_FISH:
	  if(!Z_inwater(mn[i].o.x,mn[i].o.y,mn[i].o.r,mn[i].o.h)) break;
	case MN_SOUL: case MN_PAIN: case MN_CACO:
	  if(mn[i].st!=DIE && mn[i].st!=DEAD) --mn[i].o.yv;
	  break;
  }z_mon=1;st=Z_moveobj(&mn[i].o);z_mon=0;
  BM_mark(&mn[i].o,BM_MONSTER);
  if(st&Z_FALLOUT) {
    if(t==MN_ROBO) g_exit=1;
    mn[i].t=0;--mnum;continue;
  }
  if(st&Z_HITWATER) Z_splash(&mn[i].o,mn[i].o.r+mn[i].o.h);
  SW_press(mn[i].o.x,mn[i].o.y,mn[i].o.r,mn[i].o.h,8,i);
  if(mn[i].ftime) {
    --mn[i].ftime;
    SMK_flame(mn[i].o.x,mn[i].o.y-mn[i].o.h/2,
      mn[i].o.xv+mn[i].o.vx,mn[i].o.yv+mn[i].o.vy,
      mn[i].o.r/2,mn[i].o.h/2,rand()%(200*2+1)-200,-500,1,mn[i].fobj);
  }
  if(st&Z_INWATER) mn[i].ftime=0;
  if(mn[i].st==DEAD) continue;
  if(st&Z_INWATER) if(!(rand()&31)) switch(t) {
    case MN_FISH:
      if(rand()&3) break;
    case MN_ROBO: case MN_BARREL:
    case MN_PL_DEAD: case MN_PL_MESS:
      FX_bubble(mn[i].o.x+((rand()&1)*2-1)*myrand(mn[i].o.r+1),
        mn[i].o.y-myrand(mn[i].o.h+1),0,0,1
      );
      break;
    default:
      FX_bubble(mn[i].o.x,mn[i].o.y-mn[i].o.h*3/4,0,0,5);
  }

  if(t==MN_BARREL) {

    if(!mn[i].ap[++mn[i].ac]) {
      mn[i].ac=0;if(mn[i].st==DIE || mn[i].st==DEAD) {mn[i].t=0;}
    }else if(mn[i].st==DIE && mn[i].ac==2) Z_explode(mn[i].o.x,mn[i].o.y-8,30,mn[i].aim);
    continue;
  }
  if(t==MN_SOUL) if(st&Z_HITAIR) Z_set_speed(&mn[i].o,16);
  if(mn[i].ammo<0) ++mn[i].ammo;
  if(mn[i].o.yv<0)
	if(st&Z_INWATER) mn[i].o.yv=-4;
  ++mn[i].atm;
  switch(mn[i].st) {
   case PAIN:
	if(mn[i].pain>=mnsz[t].mp)
	  {mn[i].pain=mnsz[t].mp;Z_sound(snd[t-1][1],128);}
	if((mn[i].pain-=5)<=mnsz[t].minp)
	  {setst(i,GO);mn[i].pain=0;mn[i].ammo=-9;}
	break;
   case SLEEP:
	if(++mn[i].s>=18) mn[i].s=0; else break;
	if(Z_look(&mn[i].o,&pl1.o,mn[i].d))
	  {setst(i,GO);mn[i].aim=-1;mn[i].atm=0;Z_sound(wakeupsnd(t),128);}
	if(_2pl) if(Z_look(&mn[i].o,&pl2.o,mn[i].d))
	  {setst(i,GO);mn[i].aim=-2;mn[i].atm=0;Z_sound(wakeupsnd(t),128);}
	break;
   case WAIT:
	if(--mn[i].s<0) setst(i,GO);
	break;
   case GO:
        if(st&Z_BLOCK) {mn[i].d^=1;setst(i,RUNOUT);mn[i].s=40;break;}
	if(t==MN_VILE) if(iscorpse(&mn[i].o,0)>=0) {
	  setst(i,ATTACK);mn[i].o.xv=0;break;
	}
	if(!Z_getobjpos(mn[i].aim,&o) || mn[i].atm>MAX_ATM)
	  if(!MN_findnewprey(i)) {
		mn[i].aim=-3;
		o.x=mn[i].o.x+pt_x;o.y=mn[i].o.y+pt_y;
		o.xv=o.vx=o.yv=o.vy=o.r=0;o.h=1;
	  }else Z_getobjpos(mn[i].aim,&o);
	  if(Z_overlap(&mn[i].o,&o)) {
	    mn[i].atm=0;
	    if(kick(i,&o)) break;
	  }
	  sx=o.x-mn[i].o.x;
	  sy=o.y-o.h/2-mn[i].o.y+mn[i].o.h/2;
	  if(!(st&Z_BLOCK)) if(abs(sx)<20)
	    if(t!=MN_FISH) {setst(i,RUN);mn[i].s=15;mn[i].d=rand()&1;break;}
	  if(st&Z_HITWALL) {
		if(SW_press(mn[i].o.x,mn[i].o.y,mn[i].o.r,mn[i].o.h,2,i))
		  {setst(i,WAIT);mn[i].s=4;break;}
		switch(t) {
		  case MN_CACO: case MN_SOUL: case MN_PAIN: case MN_FISH:
			break;
		  default:
			if(Z_canstand(mn[i].o.x,mn[i].o.y,mn[i].o.r))
			  {mn[i].o.yv=-mnsz[t].jv;setst(i,CLIMB);break;}
		}break;
	  }
	  mn[i].d=(sx>0)?1:0;
	  if(canshoot(t))
		if(abs(sx)>abs(sy)) if(shoot(i,&o,0)) break;

	  switch(t) {
		case MN_FISH:
		  if(!(st&Z_INWATER)) {
		    if(Z_canstand(mn[i].o.x,mn[i].o.y,mn[i].o.r)) {
		      mn[i].o.yv=-6;
		      mn[i].o.vx+=rand()%17-8;
		    }setst(i,PAIN);mn[i].pain+=50;break;
		  }
		case MN_CACO: case MN_SOUL: case MN_PAIN:
		  if(abs(sy)>4) mn[i].o.yv=(sy<0)?-4:4; else mn[i].o.yv=0;
		  if(t==MN_FISH) if(mn[i].o.yv<0)
		    if(!Z_inwater(mn[i].o.x,mn[i].o.y-8,mn[i].o.r,mn[i].o.h))
		      {mn[i].o.yv=0;setst(i,RUN);mn[i].d=rand()&1;mn[i].s=20;}
		  break;
		default:
		  if(sy<-20) if(Z_canstand(mn[i].o.x,mn[i].o.y,mn[i].o.r))
			if(!(rand()&3)) mn[i].o.yv=-mnsz[t].jv;
	  }
	if(++mn[i].s>=8) {
	  mn[i].s=0;
	  if(!(rand()&7)) Z_sound(snd[t-1][0],128);
	}
	mn[i].o.xv=((mn[i].d)?1:-1)*mnsz[t].rv;
	if(st&Z_INWATER) mn[i].o.xv/=2;
	  else if(t==MN_FISH) mn[i].o.xv=0;
	break;
   case RUN:
        if(st&Z_BLOCK) {setst(i,RUNOUT);mn[i].d^=1;mn[i].s=40;break;}
	if(--mn[i].s<=0 || ((st&Z_HITWALL) && mn[i].o.yv+mn[i].o.vy==0)) {
	  setst(i,GO);mn[i].s=0;if(st&(Z_HITWALL|Z_BLOCK)) mn[i].d^=1;
	  if(!(rand()&7)) Z_sound(snd[t-1][0],128);
	}mn[i].o.xv=((mn[i].d)?1:-1)*mnsz[t].rv;
	if(st&Z_INWATER) mn[i].o.xv/=2;
	  else if(t==MN_FISH) mn[i].o.xv=0;
	break;
   case RUNOUT:
        if(!(st&Z_BLOCK) && mn[i].s>0) mn[i].s=0;
	if(--mn[i].s<=-18) {
	  setst(i,GO);mn[i].s=0;if(st&(Z_HITWALL|Z_BLOCK)) mn[i].d^=1;
	  if(!(rand()&7)) Z_sound(snd[t-1][0],128);
	}mn[i].o.xv=((mn[i].d)?1:-1)*mnsz[t].rv;
	if(st&Z_INWATER) mn[i].o.xv/=2;
	  else if(t==MN_FISH) mn[i].o.xv=0;
	break;
   case CLIMB:
	if(mn[i].o.yv+mn[i].o.vy>=0 || !(st&Z_HITWALL)) {
	  setst(i,GO);mn[i].s=0;
	  if(st&(Z_HITWALL|Z_BLOCK)) {mn[i].d^=1;setst(i,RUN);mn[i].s=15;}
	}mn[i].o.xv=((mn[i].d)?1:-1)*mnsz[t].rv;
	if(st&Z_INWATER) mn[i].o.xv/=2;
	  else if(t==MN_FISH) mn[i].o.xv=0;
	break;
   case ATTACK:
   case SHOOT:
	if(t==MN_SOUL) {if(st&(Z_HITWALL|Z_HITCEIL|Z_HITLAND)) setst(i,GO); break;}
	if(t!=MN_FISH) mn[i].o.xv=Z_dec(mn[i].o.xv,1);
	if(t==MN_VILE && mn[i].st==SHOOT) {
          if(!Z_getobjpos(mn[i].aim,&o)) {setst(i,GO);break;}
          if(!Z_look(&mn[i].o,&o,mn[i].d)) {setst(i,GO);break;}
          if(Z_inwater(o.x,o.y,o.r,o.h)) {setst(i,GO);break;}
          mn[i].tx=o.x;mn[i].ty=o.y;
          Z_hitobj(mn[i].aim,2,i,HIT_SOME);
	}break;
  }
  if(mn[i].st==REVIVE) {
    if(--mn[i].ac==0) setst(i,GO);
  }else ++mn[i].ac;
  if(!mn[i].ap[mn[i].ac]) switch(mn[i].st) {
	case ATTACK:
	  switch(t) {
		case MN_SOUL: mn[i].ac=0;
		case MN_IMP:
		case MN_DEMON:
		  if(Z_hit(&mn[i].o,15,i,HIT_SOME)) if(t==MN_SOUL) setst(i,GO);
		  break;
		case MN_FISH:
		  if(Z_hit(&mn[i].o,10,i,HIT_SOME))
		    Z_sound(snd[MN_FISH-1][2],128);
		  break;
		case MN_SKEL: case MN_ROBO:
		  o=mn[i].o;o.xv=mn[i].d?50:-50;
		  if(Z_hit(&o,50,i,HIT_SOME)) Z_sound(pchsnd,128);
		  break;
		case MN_VILE:
		  sx=iscorpse(&mn[i].o,1);
		  if(sx==-3) break;
		  if(!mn[sx].t || mn[sx].st!=DEAD) break;
            	  setst(sx,REVIVE);Z_sound(slopsnd,128);
		  hit_xv=hit_yv=0;MN_hit(i,5,-3,HIT_SOME);
		  break;
	  }if(t!=MN_SOUL && mn[i].st!=DIE) setst(i,GO);
	  break;
	case SHOOT:
	  switch(t) {
		case MN_IMP:
		  WP_ball1(mn[i].o.x+(mn[i].d*2-1)*mn[i].o.r,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_ZOMBY:
		  WP_pistol(mn[i].o.x+(mn[i].d*2-1)*mn[i].o.r,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_SERG:
		  WP_shotgun(mn[i].o.x+(mn[i].d*2-1)*mn[i].o.r,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_MAN:
		  WP_dshotgun(mn[i].o.x+(mn[i].d*2-1)*mn[i].o.r,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  mn[i].ammo=-36;break;
		case MN_CYBER:
		  WP_rocket(mn[i].o.x+(mn[i].d*2-1)*mn[i].o.r,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_SKEL:
		  WP_revf(mn[i].o.x+(mn[i].d*2-1)*mn[i].o.r,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i,mn[i].aim);
		  break;
		case MN_CGUN:
		case MN_SPIDER:
		  WP_mgun(mn[i].o.x+(mn[i].d*2-1)*mn[i].o.r,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_BSP:
		  WP_aplasma(mn[i].o.x+(mn[i].d*2-1)*mn[i].o.r,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_ROBO:
		  WP_plasma(mn[i].o.x+(mn[i].d*2-1)*15,mn[i].o.y-30,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_MANCUB:
		  WP_manfire(mn[i].o.x+(mn[i].d*2-1)*mn[i].o.r,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_BARON: case MN_KNIGHT:
		  WP_ball7(mn[i].o.x,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_CACO:
		  WP_ball2(mn[i].o.x,mn[i].o.y-mn[i].o.h/2,mn[i].tx,mn[i].ty,i);
		  break;
		case MN_PAIN:
		  if((sx=MN_spawn(mn[i].o.x,mn[i].o.y,mn[i].d,MN_SOUL))==-1) break;
		  Z_getobjpos(mn[sx].aim=mn[i].aim,&o);mn[sx].atm=0;
		  shoot(sx,&o,1);
		  break;
	  }
	  if(t==MN_CGUN || t==MN_SPIDER || t==MN_BSP || t==MN_MANCUB || t==MN_ROBO)
	     if(!Z_getobjpos(mn[i].aim,&o)) MN_findnewprey(i);
		else if(shoot(i,&o,0)) break;
	  setst(i,GO);break;
	case DIE:
	  setst(i,DEAD);
	  if(t==MN_PAIN || t==MN_SOUL) mn[i].ftime=0;
	  if(t==MN_PAIN) {
		if((sx=MN_spawn(mn[i].o.x-15,mn[i].o.y,0,MN_SOUL))==-1) break;
		setst(sx,GO);
		if((sx=MN_spawn(mn[i].o.x+15,mn[i].o.y,1,MN_SOUL))==-1) break;
		setst(sx,GO);
		if((sx=MN_spawn(mn[i].o.x,mn[i].o.y-10,1,MN_SOUL))==-1) break;
		setst(sx,GO);
	  }break;
	default: mn[i].ac=0;
  }
  switch(mn[i].st) {
	case GO: case RUN: case CLIMB: case RUNOUT:
	  if(t==MN_CYBER || t==MN_SPIDER || t==MN_BSP) {
	    if(mn[i].ac==0 || mn[i].ac==6) Z_sound(snd[t-1][2],128);
	  }else if(t==MN_ROBO)
	    if(mn[i].ac==0 || mn[i].ac==12) Z_sound(snd[t-1][2],128);
  }
  }
}

void MN_mark (void) {
  int i;
  for(i=0;i<MAXMN;++i) if(mn[i].t!=0) BM_mark(&mn[i].o,BM_MONSTER);
}

int MN_hit(int n,int d,int o,int t) {
  int i;

  if(mn[n].st==DEAD || mn[n].st==DIE) return 0;
  if(o==n) {
	if(t!=HIT_ROCKET && t!=HIT_ELECTRO) return 0;
	if(mn[n].t==MN_CYBER || mn[n].t==MN_BARREL) return 1;
  }
  if(o>=0) {
	if(mn[o].t==MN_SOUL && mn[n].t==MN_PAIN) return 0;
	if(mn[o].t==mn[n].t) switch(mn[n].t) {
	  case MN_IMP: case MN_DEMON:
	  case MN_BARON: case MN_KNIGHT:
	  case MN_CACO: case MN_SOUL:
	  case MN_MANCUB: case MN_SKEL:
	  case MN_FISH:
		return 0;
	}
  }
  if(t==HIT_FLAME) if(mn[n].ftime && mn[n].fobj==o) {if(g_time&31) return 1;}
    else {mn[n].ftime=255;mn[n].fobj=o;}
  if(t==HIT_ELECTRO) if(mn[n].t==MN_FISH)
    {setst(n,RUN);mn[n].s=20;mn[n].d=rand()&1;return 1;}
  if(t==HIT_TRAP) mn[n].life=-100;
  if(mn[n].t==MN_ROBO) d=0;
  if((mn[n].life-=d)<=0) --mnum;
  if(!mn[n].pain) mn[n].pain=3;
  mn[n].pain+=d;
  if(mn[n].st!=PAIN) {
	if(mn[n].pain>=mnsz[mn[n].t].minp) setst(n,PAIN);
  }
  if(mn[n].t!=MN_BARREL)
    DOT_blood(mn[n].o.x,mn[n].o.y-mn[n].o.h/2,hit_xv,hit_yv,d*2);
  mn[n].aim=o;mn[n].atm=0;
  if(mn[n].life<=0) {
	if(mn[n].t!=MN_BARREL)
	  if(o==-1) ++pl1.kills;
	  else if(o==-2) ++pl2.kills;
	setst(n,DIE);
	switch(mn[n].t) {
	  case MN_ZOMBY: i=I_CLIP;break;
	  case MN_SERG: i=I_SGUN;break;
	  case MN_CGUN: i=I_MGUN;break;
	  case MN_MAN: i=I_KEYR;break;
	  default: i=0;
	}if(i) IT_spawn(mn[n].o.x,mn[n].o.y,i);
	mn[n].o.xv=0;mn[n].o.h=6;
	if(mn[n].life<=-mnsz[mn[n].t].sp)
	  switch(mn[n].t) {
		case MN_IMP: case MN_ZOMBY: case MN_SERG: case MN_CGUN:
		case MN_MAN:
		  mn[n].ap=slopanim[mn[n].t-1];
		  Z_sound(slopsnd,128);
		  break;
		case MN_BSP: if(g_map==9) break;
		default:
		  Z_sound(dthsnd(mn[n].t),128);
	  }
	else if(mn[n].t!=MN_BSP || g_map!=9) Z_sound(dthsnd(mn[n].t),128);
	mn[n].life=0;
  }else if(mn[n].st==SLEEP) {setst(n,GO);mn[n].pain=mnsz[mn[n].t].mp;}
  return 1;
}

#define hit(o,x,y) (y<=o.y && y>o.y-o.h && x>=o.x-o.r && x<=o.x+o.r)

int Z_gunhit (int x, int y, int o, int xv, int yv) {
  int i;

  if(o!=-1) if(hit(pl1.o,x,y)) if(PL_hit(&pl1,3,o,HIT_SOME))
    {pl1.o.vx+=xv;pl1.o.vy+=yv;return -1;}
  if(_2pl && o!=-2) if(hit(pl2.o,x,y)) if(PL_hit(&pl2,3,o,HIT_SOME))
    {pl2.o.vx+=xv;pl2.o.vy+=yv;return -2;}

  for(i=0;i<MAXMN;++i) if(mn[i].t && o!=i)
    if(hit(mn[i].o,x,y)) if(MN_hit(i,3,o,HIT_SOME))
      {mn[i].o.vx+=xv;mn[i].o.vy+=yv;return 1;}
  return 0;
}

static void goodsnd(void) {
  if(!g_dm) return;
  gsndt=18;
}

int Z_hit (obj_t *o, int d, int own, int t) {
  int i;

  hit_xv=o->xv+o->vx;
  hit_yv=o->yv+o->vy;
  if(Z_overlap(o,&pl1.o)) if(PL_hit(&pl1,d,own,t)) {
	pl1.o.vx+=(o->xv+o->vx)*((t==HIT_BFG)?8:1)/4;
	pl1.o.vy+=(o->yv+o->vy)*((t==HIT_BFG)?8:1)/4;
	if(t==HIT_BFG) goodsnd();
	return -1;
  }
  if(_2pl) if(Z_overlap(o,&pl2.o)) if(PL_hit(&pl2,d,own,t)) {
	pl2.o.vx+=(o->xv+o->vx)*((t==HIT_BFG)?8:1)/4;
	pl2.o.vy+=(o->yv+o->vy)*((t==HIT_BFG)?8:1)/4;
	if(t==HIT_BFG) goodsnd();
	return -2;
  }

  for(i=0;i<MAXMN;++i) if(mn[i].t)
    if(Z_overlap(o,&mn[i].o)) if(MN_hit(i,d,own,t)) {
	  mn[i].o.vx+=(o->xv+o->vx)*((t==HIT_BFG)?8:1)/4;
	  mn[i].o.vy+=(o->yv+o->vy)*((t==HIT_BFG)?8:1)/4;
	  return 1;
    }
  return 0;
}

void MN_killedp (void) {
  int i;
  for(i=0;i<MAXMN;++i) if(mn[i].t==MN_MAN)
    if(mn[i].st!=DEAD && mn[i].st!=DIE && mn[i].st!=SLEEP)
      Z_sound(trupsnd,128);
}

void Z_explode (int x,int y,int rad,int own) {
  long r;
  int dx,dy,m,i;

  if(x<-100 || x>FLDW*CELW+100) return;
  if(y<-100 || y>FLDH*CELH+100) return;
  r=(long)rad*rad;
  dx=pl1.o.x-x;dy=pl1.o.y-pl1.o.h/2-y;
  if((long)dx*dx+(long)dy*dy<r) {
    if(!(m=max(abs(dx),abs(dy)))) m=1;
	pl1.o.vx+=hit_xv=dx*10/m;
	pl1.o.vy+=hit_yv=dy*10/m;
	PL_hit(&pl1,100*(rad-m)/rad,own,HIT_ROCKET);
  }
  if(_2pl) {
    dx=pl2.o.x-x;dy=pl2.o.y-pl2.o.h/2-y;
    if((long)dx*dx+(long)dy*dy<r) {
      if(!(m=max(abs(dx),abs(dy)))) m=1;
	  pl2.o.vx+=hit_xv=dx*10/m;
	  pl2.o.vy+=hit_yv=dy*10/m;
      PL_hit(&pl2,100*(rad-m)/rad,own,HIT_ROCKET);
    }
  }
  for(i=0;i<MAXMN;++i) if(mn[i].t) {
    dx=mn[i].o.x-x;dy=mn[i].o.y-mn[i].o.h/2-y;
    if((long)dx*dx+(long)dy*dy<r) {
      if(!(m=max(abs(dx),abs(dy)))) m=1;
	  mn[i].o.vx+=hit_xv=dx*10/m;
	  mn[i].o.vy+=hit_yv=dy*10/m;
	  MN_hit(i,mn[i].o.r*10*(rad-m)/rad,own,HIT_ROCKET);
    }
  }
}

void Z_bfg9000 (int x,int y,int own) {
  int dx,dy,i;

  hit_xv=hit_yv=0;
  if(x<-100 || x>FLDW*CELW+100) return;
  if(y<-100 || y>FLDH*CELH+100) return;
  dx=pl1.o.x-x;dy=pl1.o.y-pl1.o.h/2-y;
  if(own!=-1) if((long)dx*dx+(long)dy*dy<16000)
   if(Z_cansee(x,y,pl1.o.x,pl1.o.y-pl1.o.h/2)) {
    if(PL_hit(&pl1,50,own,HIT_SOME))
      WP_bfghit(pl1.o.x,pl1.o.y-pl1.o.h/2,own);
  }
  if(_2pl) {
    dx=pl2.o.x-x;dy=pl2.o.y-pl2.o.h/2-y;
    if(own!=-2) if((long)dx*dx+(long)dy*dy<16000)
     if(Z_cansee(x,y,pl2.o.x,pl2.o.y-pl2.o.h/2)) {
      if(PL_hit(&pl2,50,own,HIT_SOME))
	WP_bfghit(pl2.o.x,pl2.o.y-pl2.o.h/2,own);
    }
  }
  for(i=0;i<MAXMN;++i) if(mn[i].t && own!=i) {
    dx=mn[i].o.x-x;dy=mn[i].o.y-mn[i].o.h/2-y;
    if((long)dx*dx+(long)dy*dy<16000)
     if(Z_cansee(x,y,mn[i].o.x,mn[i].o.y-mn[i].o.h/2)) {
      if(MN_hit(i,50,own,HIT_SOME))
	WP_bfghit(mn[i].o.x,mn[i].o.y-mn[i].o.h/2,own);
    }
  }
}

int Z_chktrap (int t, int d, int o, int ht) {
  int i,s;

  hit_xv=hit_yv=0;
  s=0;
  if(Z_istrapped(pl1.o.x,pl1.o.y,pl1.o.r,pl1.o.h)) {
    s=1;
	if(t) PL_hit(&pl1,d,o,ht);
  }
  if(_2pl) if(Z_istrapped(pl2.o.x,pl2.o.y,pl2.o.r,pl2.o.h)) {
	s=1;
	if(t) PL_hit(&pl2,d,o,ht);
  }
  for(i=0;i<MAXMN;++i) if(mn[i].t && mn[i].st!=DEAD)
    if(Z_istrapped(mn[i].o.x,mn[i].o.y,mn[i].o.r,mn[i].o.h)) {
      s=1;
	  if(t) MN_hit(i,d,o,ht);
	}
  return s;
}

void Z_teleobj (int o, int x, int y) {
  obj_t *p;

  if(o==-1) p=&pl1.o;
  else if(o==-2) p=&pl2.o;
  else if(o>=0 && o<MAXMN) p=&mn[o].o;
  else return;
  FX_tfog(p->x,p->y);FX_tfog(p->x=x,p->y=y);
  Z_sound(telesnd,128);
}

void MN_warning (int l,int t,int r,int b) {
  int i;

  for(i=0;i<MAXMN;++i) if(mn[i].t && mn[i].t!=MN_CACO && mn[i].t!=MN_SOUL
      && mn[i].t!=MN_PAIN && mn[i].t!=MN_FISH)
    if(mn[i].st!=DIE && mn[i].st!=DEAD && mn[i].st!=SLEEP)
      if(mn[i].o.x+mn[i].o.r>=l && mn[i].o.x-mn[i].o.r<=r
      && mn[i].o.y>=t && mn[i].o.y-mn[i].o.h<=b)
        if(Z_canstand(mn[i].o.x,mn[i].o.y,mn[i].o.r))
          mn[i].o.yv=-mnsz[mn[i].t].jv;
}
