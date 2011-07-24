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
#include <string.h>
#include <stdlib.h>
#include "files.h"
#include "memory.h"
#include "vga.h"
#include "error.h"
#include "keyb.h"
#include "sound.h"
#include "view.h"
#include "player.h"
#include "switch.h"
#include "menu.h"
#include "misc.h"

#include "SDL.h"
extern SDL_Surface *screen;

#define QSND_NUM 14

enum{HIT100,ARMOR,JUMP,WPNS,IMMORTAL,SPEED,OPEN,EXIT};

extern int PL_JUMP,PL_RUN;
extern byte _warp,cheat,p_fly;

extern byte g_music[8];

extern byte savname[7][24],savok[7];
void load_game(int);

static byte panim[]=
  "BBDDAACCDDAABBDDAACCDDAABBDDAACCDDAAEEEEEFEFEFEFEFEFEFEFEFEFEEEEE";
static byte *panimp=panim;

#define PCOLORN 10
byte pcolortab[PCOLORN]={
  0x18,0x20,0x40,0x58,0x60,0x70,0x80,0xB0,0xC0,0xD0
};
int p1color=5,p2color=4;

static byte ibuf[24],input=0;
static int icur;

enum{MENU,MSG};
enum{CANCEL,NEWGAME,LOADGAME,SAVEGAME,OPTIONS,QUITGAME,QUIT,ENDGAME,ENDGM,
  PLR1,PLR2,COOP,DM,VOLUME,GAMMA,LOAD,SAVE,PLCOLOR,PLCEND,MUSIC,INTERP,
  SVOLM,SVOLP,MVOLM,MVOLP,GAMMAM,GAMMAP,PL1CM,PL1CP,PL2CM,PL2CP};

#ifndef DEMO
static int qsnd[QSND_NUM];
#endif

static char *main_txt[]={
  "NEW GAME","LOAD GAME","SAVE GAME","OPTIONS","EXIT"
},*opt_txt[]={
  "RESTART","VOLUME","BRIGHTNESS","MUSIC","FULLSCREEN:"
},*ngplr_txt[]={
  "ONE PLAYER","TWO PLAYERS"
},*ngdm_txt[]={
  "COOPERATIVE","DEATHMATCH"
},*vol_txt[]={
  "SOUND","MUSIC"
},*plcolor_txt[]={
  "FIRST","SECOND"
},*gamma_txt[]={
  ""
};

static byte main_typ[]={
  NEWGAME,LOADGAME,SAVEGAME,OPTIONS,QUITGAME
},ngplr_typ[]={
  PLR1,PLR2
},ngdm_typ[]={
  COOP,DM
},opt_typ[]={
  ENDGAME,VOLUME,GAMMA,MUSIC,INTERP
},quit_typ[]={
  QUIT,CANCEL
},endgm_typ[]={
  ENDGM,CANCEL
},vol_typ[]={
  SVOLM,MVOLM
},plcolor_typ[]={
  PL1CM,PL2CM
},gamma_typ[]={
  GAMMAM
},load_typ[]={
  LOAD,LOAD,LOAD,LOAD,LOAD,LOAD,LOAD
},save_typ[]={
  SAVE,SAVE,SAVE,SAVE,SAVE,SAVE,SAVE
};

static menu_t main_mnu={
  MENU,5,0,80,"MENU",main_txt,main_typ
},opt_mnu={
  MENU,5,0,75,"OPTIONS",opt_txt,opt_typ
},ngplr_mnu={
  MENU,2,0,90,"NEW GAME",ngplr_txt,ngplr_typ
},ngdm_mnu={
  MENU,2,0,90,"GAME TYPE",ngdm_txt,ngdm_typ
},vol_mnu={
  MENU,2,0,40,"VOLUME",vol_txt,vol_typ
},plcolor_mnu={
  MENU,2,0,90,"COLOR",plcolor_txt,plcolor_typ
},gamma_mnu={
  MENU,1,0,85,"BRIGHTNESS",gamma_txt,gamma_typ
},load_mnu={
  MENU,7,0,85,"LOAD GAME",NULL,load_typ
},save_mnu={
  MENU,7,0,85,"SAVE GAME",NULL,save_typ
},quit1_msg={
  MSG,0,0,0,"ARE YOU SURE?",NULL,quit_typ
},quit2_msg={
  MSG,0,0,0,"ARE YOU SURE?",NULL,quit_typ
},quit3_msg={
  MSG,0,0,0,"ARE YOU SURE?",NULL,quit_typ
},endgm_msg={
  MSG,0,0,0,"RESTART LEVEL?",NULL,endgm_typ
};

static menu_t *qmsg[3]={&quit1_msg,&quit2_msg,&quit3_msg};

static menu_t *mnu=NULL;

static byte gm_redraw=0;
static int gm_tm=0;
short lastkey=0;
static void *csnd1,*csnd2,*msnd1,*msnd2,*msnd3,*msnd4,*msnd5,*msnd6;
static int movsndt=0;
static vgaimg *msklh[2],*mbarl,*mbarm,*mbarr,*mbaro,*mslotl,*mslotm,*mslotr;
static byte cbuf[32];

static snd_t *voc=NULL;
static int voc_ch=0;

void GMV_stop(void) {
  if(voc) {
    if(voc_ch) {S_stop(voc_ch);voc_ch=0;}
    free(voc);voc=NULL;
  }
}

void GMV_say(char *nm) {
  int r,len;
  snd_t *p;
  byte *d;

  if((r=F_findres(nm))==-1) return;
  if(!(p=malloc((len=F_getreslen(r))+16))) return;
  p->len=len;p->rate=11000;
  p->lstart=p->llen=0;
  GMV_stop();
  F_loadres(r,p+1,0,len);
  for(d=(byte*)(p+1);len;--len,++d) *d^=128;
  voc=p;
  voc_ch=S_play(voc,-1,1024,255);
}

void G_code(void) {
  void *s;
  s=csnd2;
  if(memcmp(cbuf+32-5,"IDDQD",5)==0) {
    PL_hit(&pl1,400,0,HIT_SOME);
    if(_2pl) PL_hit(&pl2,400,0,HIT_SOME);
    s=csnd1;
  }else if(memcmp(cbuf+32-4,"TANK",4)==0) {
    pl1.life=pl1.armor=200;pl1.drawst|=PL_DRAWARMOR|PL_DRAWLIFE;
    if(_2pl) {pl2.life=pl2.armor=200;pl2.drawst|=PL_DRAWARMOR|PL_DRAWLIFE;}
  }else if(memcmp(cbuf+32-8,"BULLFROG",8)==0) {
    PL_JUMP=(PL_JUMP==10)?20:10;
  }else if(memcmp(cbuf+32-8,"FORMULA1",8)==0) {
    PL_RUN=(PL_RUN==8)?24:8;
  }else if(memcmp(cbuf+32-5,"RAMBO",5)==0) {
    pl1.ammo=pl1.shel=pl1.rock=pl1.cell=pl1.fuel=30000;
    pl1.wpns=0x7FF;pl1.drawst|=PL_DRAWWPN|PL_DRAWKEYS;
    pl1.keys=0x70;
    if(_2pl) {
      pl2.ammo=pl2.shel=pl2.rock=pl2.cell=pl1.fuel=30000;
      pl2.wpns=0x7FF;pl2.drawst|=PL_DRAWWPN|PL_DRAWKEYS;
      pl2.keys=0x70;
    }
  }else if(memcmp(cbuf+32-5,"UJHTW",5)==0) {
    p_immortal=!p_immortal;
  }else if(memcmp(cbuf+32-9,",TKSQJHTK",9)==0) {
    p_fly=!p_fly;
  }else if(memcmp(cbuf+32-6,"CBVCBV",6)==0) {
    SW_cheat_open();
  }else if(memcmp(cbuf+32-7,"GOODBYE",7)==0) {
    g_exit=1;
  }else if(memcmp(cbuf+32-9,"GJITKYF",7)==0) {
    if(cbuf[30]>='0' && cbuf[30]<='9' && cbuf[31]>='0' && cbuf[31]<='9') {
      g_map=(cbuf[30]=='0')?0:(cbuf[30]-'0')*10;
      g_map+=(cbuf[31]=='0')?0:(cbuf[31]-'0');
      G_start();
    }
  }else return;
  memset(cbuf,0,32);
  Z_sound(s,128);
}

void GM_set(menu_t *m) {
  mnu=m;gm_redraw=1;
  if(g_st==GS_GAME) {
	//V_setrect(0,SCRW,0,SCRH);V_clr(0,SCRW,0,SCRH,0);//V_setrect(0,320,0,200);V_clr(0,320,0,200,0);
	//if(_2pl) {V_setrect(SCRW-120,120,0,SCRH);w_o=0;Z_clrst();w_o=SCRH/2;Z_clrst();}//if(_2pl) {V_setrect(200,120,0,200);w_o=0;Z_clrst();w_o=100;Z_clrst();}
	//else {V_setrect(SCRW-120,120,0,SCRH);w_o=0;Z_clrst();}//else {V_setrect(200,120,50,100);w_o=50;Z_clrst();}
	//pl1.drawst=pl2.drawst=0xFF;V_setrect(0,SCRW,0,SCRH);//V_setrect(0,320,0,200);
  }
}

void setgamma(int);

void GM_command(int c) {
  switch(c) {
    case CANCEL:
      GM_set(NULL);break;
    case INTERP:
      fullscreen=!fullscreen;
      V_toggle();
      GM_set(mnu);
      break;
    case MUSIC:
      F_freemus();
      F_nextmus(g_music);
      F_loadmus(g_music);
      S_startmusic(music_time*2);
      GM_set(mnu);
      break;
    case NEWGAME:
      GMV_say("_NEWGAME");
      GM_set(&ngplr_mnu);break;
    case PLR2:
      GMV_say("_2PLAYER");
      GM_set(&ngdm_mnu);break;
    case PLR1:
      GMV_say("_1PLAYER");
      ngdm_mnu.cur=0;
    case COOP: case DM:
      if(c==COOP) GMV_say("_COOP");
      else if(c==DM) GMV_say("_DM");
      if(c!=PLR1) {GM_set(&plcolor_mnu);break;}
    case PLCEND:
      _2pl=ngplr_mnu.cur;
      g_dm=ngdm_mnu.cur;
      g_map=(_warp)?_warp:1;
      PL_reset();
      if(_2pl) {
        pl1.color=pcolortab[p1color];
        pl2.color=pcolortab[p2color];
      }else pl1.color=0x70;
      G_start();
      GM_set(NULL);break;
    case OPTIONS:
      GMV_say("_RAZNOE");
      GM_set(&opt_mnu);break;
    case LOADGAME:
      GMV_say("_OLDGAME");
      F_getsavnames();GM_set(&load_mnu);break;
    case SAVEGAME:
      if(g_st!=GS_GAME) break;
      GMV_say("_SAVEGAM");
      F_getsavnames();GM_set(&save_mnu);break;
    case SAVE:
	  input=1;memcpy(ibuf,savname[save_mnu.cur],24);icur=strlen(ibuf);
	  GM_set(mnu);break;
    case LOAD:
	  if(!savok[load_mnu.cur]) break;
	  load_game(load_mnu.cur);
	  GM_set(NULL);break;
	case VOLUME:
	  GMV_say("_VOLUME");
	  GM_set(&vol_mnu);break;
	case GAMMA:
	  GMV_say("_GAMMA");
	  GM_set(&gamma_mnu);break;
	case QUITGAME:
	  GMV_say((rand()&1)?"_EXIT1":"_EXIT2");
	  GM_set(qmsg[myrand(3)]);break;
	case ENDGAME:
	  if(g_st!=GS_GAME) break;
	  GMV_say("_RESTART");
	  GM_set(&endgm_msg);break;
	case QUIT:
	  F_freemus();
	  GMV_stop();
#ifndef DEMO
	  c=Z_sound(M_lock(qsnd[myrand(QSND_NUM)]),256);//for(c=(Z_sound(M_lock(qsnd[random2(QSND_NUM)]),256)+9)<<16,timer=0;timer<c;);
          S_wait();
#endif
	  ERR_quit();break;
    case ENDGM:
	  PL_reset();G_start();
	  GM_set(NULL);break;
	case PL1CM:
	  if(--p1color<0) p1color=PCOLORN-1; break;
	case PL1CP:
	  if(++p1color>=PCOLORN) p1color=0; break;
	case PL2CM:
	  if(--p2color<0) p2color=PCOLORN-1; break;
	case PL2CP:
	  if(++p2color>=PCOLORN) p2color=0; break;
	case SVOLM:
	  S_volume(snd_vol-8);break;
	case SVOLP:
	  S_volume(snd_vol+8);break;
	case MVOLM:
	  S_volumemusic(mus_vol-8);break;
	case MVOLP:
	  S_volumemusic(mus_vol+8);break;
	case GAMMAM: setgamma(gammaa-1);break;
	case GAMMAP: setgamma(gammaa+1);break;
  }
}

/*
byte keychar[2][128]={{
  0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
  'Q','W','E','R','T','Y','U','I','O','P','[',']','\r',0,'A','S',
  'D','F','G','H','J','K','L',';','\'',0,0,'\\','Z','X','C','V',
  'B','N','M',',','.','/',0,'*',0,' ',0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
},{
  0,0,'!','\"','#','$','%',':','&','*','(',')','_','+',0,0,
  'x','x','x','x','x','x','x','x','x','x','x','x','\r',0,'x','x',
  'x','x','x','x','x','x','x','x','x',0,0,0,'x','x','x','x',
  'x','x','x','x','x','?',0,'*',0,' ',0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
}};
*/

struct {
    int keysym;
    byte ch;
} keychar[] = {
    {SDLK_SPACE, ' '},
    {SDLK_0, '0'},
    {SDLK_1, '1'},
    {SDLK_2, '2'},
    {SDLK_3, '3'},
    {SDLK_4, '4'},
    {SDLK_5, '5'},
    {SDLK_6, '6'},
    {SDLK_7, '7'},
    {SDLK_8, '8'},
    {SDLK_9, '9'},
    {SDLK_UNDERSCORE, '_'},
    {SDLK_a, 'A'},
    {SDLK_b, 'B'},
    {SDLK_c, 'C'},
    {SDLK_d, 'D'},
    {SDLK_e, 'E'},
    {SDLK_f, 'F'},
    {SDLK_g, 'G'},
    {SDLK_h, 'H'},
    {SDLK_i, 'I'},
    {SDLK_j, 'J'},
    {SDLK_k, 'K'},
    {SDLK_l, 'L'},
    {SDLK_m, 'M'},
    {SDLK_n, 'N'},
    {SDLK_o, 'O'},
    {SDLK_p, 'P'},
    {SDLK_q, 'Q'},
    {SDLK_r, 'R'},
    {SDLK_s, 'S'},
    {SDLK_t, 'T'},
    {SDLK_u, 'U'},
    {SDLK_v, 'V'},
    {SDLK_w, 'W'},
    {SDLK_x, 'X'},
    {SDLK_y, 'Y'},
    {SDLK_z, 'Z'},
    {SDLK_COMMA,','},
    {0}
};

byte get_keychar(int keysym)
{
    int i = 0;
    while (keychar[i].keysym) {
        if (keychar[i].keysym == keysym) return keychar[i].ch;
        i++;
    }
    return 0;
}

extern vgapal main_pal,std_pal;
extern byte shot_vga;

static void shot(void) {
  static int num=1;
  char fn[13];
  sprintf(fn,"shot%04d.bmp",num);
  SDL_SaveBMP(screen, fn);
  ++num;
}

int GM_act(void) {
  byte c;

  if(mnu==&plcolor_mnu) {
    if(*(++panimp)==0) panimp=panim;
    GM_set(mnu);
  }
  if(movsndt>0) --movsndt; else movsndt=0;
  if(g_st==GS_TITLE) if(!mnu) if(lastkey) {
    GM_set(&main_mnu);Z_sound(msnd3,128);
    lastkey=0;
    return 1;
  }
  if(input) switch(lastkey) {
    case SDLK_RETURN: case SDLK_KP_ENTER://case 0x1C: case 0x9C:
      F_savegame(save_mnu.cur,ibuf);
      input=0;GM_set(NULL);break;
    case 1: input=0;GM_set(mnu);break;
    case SDLK_BACKSPACE://case 0x0E:
        if(icur) {ibuf[--icur]=0;GM_set(mnu);} break;
    default:
      if(icur>=23) break;
      c=get_keychar(lastkey);//c=keychar[(keys[0x2A] || keys[0x36])?1:0][lastkey];
      if(!c) break;
      ibuf[icur]=c;ibuf[++icur]=0;GM_set(mnu);
  }else {
      switch(lastkey) {
    case SDLK_ESCAPE://case 1:
      if(!mnu) {GM_set(&main_mnu);Z_sound(msnd3,128);}
      else {GM_set(NULL);Z_sound(msnd4,128);}
      break;
    case SDLK_F5:
      if(mnu) break;
      Z_sound(msnd3,128);
      GMV_say("_GAMMA");
      GM_set(&gamma_mnu);break;
    case SDLK_F4://case 0x3E:
      if(mnu) break;
      Z_sound(msnd3,128);
      GMV_say("_VOLUME");
      GM_set(&vol_mnu);break;
    case SDLK_F2://case 0x3C:
      if(mnu) break;
      if(g_st!=GS_GAME) break;
      Z_sound(msnd3,128);
      F_getsavnames();GM_set(&save_mnu);break;
    case SDLK_F3://case 0x3D:
      if(mnu) break;
      Z_sound(msnd3,128);
      F_getsavnames();GM_set(&load_mnu);break;
    case SDLK_F10://case 0x44:
      if(mnu) break;
      Z_sound(msnd3,128);
      GM_command(QUITGAME);break;
    case SDLK_UP: case SDLK_KP8://case 0x48: case 0xC8:
      if(!mnu) break;
      if(mnu->type!=MENU) break;
      if(--mnu->cur<0) mnu->cur=mnu->n-1;
      GM_set(mnu);
      Z_sound(msnd1,128);break;
    case SDLK_DOWN: case SDLK_KP5: case SDLK_KP2://case 0x50: case 0xD0: case 0x4C:
      if(!mnu) break;
      if(mnu->type!=MENU) break;
      if(++mnu->cur>=mnu->n) mnu->cur=0;
      GM_set(mnu);
      Z_sound(msnd1,128);break;
    case SDLK_LEFT: case SDLK_RIGHT: case SDLK_KP4: case SDLK_KP6://case 0x4B: case 0x4D: case 0xCB: case 0xCD:
	  if(!mnu) break;
	  if(mnu->type!=MENU) break;
	  if(mnu->t[mnu->cur]<SVOLM) break;
	  GM_command(mnu->t[mnu->cur]+((lastkey==SDLK_LEFT || lastkey==SDLK_KP4)?0:1));//GM_command(mnu->t[mnu->cur]+((lastkey==0x4B || lastkey==0xCB)?0:1));
	  GM_set(mnu);
	  if(!movsndt) movsndt=Z_sound((lastkey==SDLK_LEFT || lastkey==SDLK_KP4)?msnd5:msnd6,255);//if(!movsndt) movsndt=Z_sound((lastkey==0x4B || lastkey==0xCB)?msnd5:msnd6,255);
	  break;
    case SDLK_RETURN: case SDLK_SPACE: case SDLK_KP_ENTER://case 0x1C: case 0x39: case 0x9C:
	  if(!mnu) break;
	  if(mnu->type!=MENU) break;
	  if(mnu->t[mnu->cur]>=PL1CM) {
	    Z_sound(msnd2,128);
	    GM_command(PLCEND);
	    break;
	  }
	  if(mnu->t[mnu->cur]>=SVOLM) break;
	  Z_sound(msnd2,128);
	  GM_command(mnu->t[mnu->cur]);
      break;
    case SDLK_y://case 0x15:
      if(!mnu) break;
      if(mnu->type!=MSG) break;
      Z_sound(msnd3,128);
      GM_command(mnu->t[0]);
      break;
    case SDLK_n://case 0x31:
      if(!mnu) break;
      if(mnu->type!=MSG) break;
      Z_sound(msnd4,128);
      GM_command(mnu->t[1]);
      break;
    case SDLK_F1://case 0x3B:
      if(shot_vga) {shot();Z_sound(msnd4,128);}
      break;
  }
  }
  lastkey=0;
  return((mnu)?1:0);
}

void G_keyf(int k, int press) {
  int i;

  lastkey=k;
  if(!_2pl || cheat) {
    for(i=0;i<31;++i) cbuf[i]=cbuf[i+1];
    cbuf[31]=get_keychar(k);
  }
}

void GM_init(void) {
#ifndef DEMO
  int i;
  static char nm[QSND_NUM][6]={
	"CYBSIT","KNTDTH","MNPAIN","PEPAIN","SLOP","MANSIT","BOSPN","VILACT",
	"PLFALL","BGACT","BGDTH2","POPAIN","SGTATK","VILDTH"
  };
  char s[8];

  s[0]='D';s[1]='S';
  for(i=0;i<QSND_NUM;++i) {
    memcpy(s+2,nm[i],6);
    qsnd[i]=F_getresid(s);
  }
#endif
  csnd1=Z_getsnd("HAHA1");
  csnd2=Z_getsnd("RADIO");
  msnd1=Z_getsnd("PSTOP");
  msnd2=Z_getsnd("PISTOL");
  msnd3=Z_getsnd("SWTCHN");
  msnd4=Z_getsnd("SWTCHX");
  msnd5=Z_getsnd("SUDI");
  msnd6=Z_getsnd("TUDI");
  msklh[0]=M_lock(F_getresid("M_SKULL1"));
//  msklh[0]=load_vga("vga\\spr.vga","M_SKULL1");
  msklh[1]=M_lock(F_getresid("M_SKULL2"));
  mbarl=M_lock(F_getresid("M_THERML"));
  mbarm=M_lock(F_getresid("M_THERMM"));
  mbarr=M_lock(F_getresid("M_THERMR"));
  mbaro=M_lock(F_getresid("M_THERMO"));
  mslotl=M_lock(F_getresid("M_LSLEFT"));
  mslotm=M_lock(F_getresid("M_LSCNTR"));
  mslotr=M_lock(F_getresid("M_LSRGHT"));
  K_setkeyproc(G_keyf);
}

int GM_draw(void) {
  int i,j,k,y;

  ++gm_tm;
  V_setrect(0,SCRW,0,SCRH);//V_setrect(0,320,0,200);
  if(!mnu && !gm_redraw) return 0;
  gm_redraw=0;
  if(!mnu) return 1;
  if(mnu->type==MENU) {
    y=(200-mnu->n*16-20)/2;
    Z_gotoxy(mnu->x,y-10);Z_printbf(mnu->ttl);
    for(i=0;i<mnu->n;++i) {
	  if(mnu->t[i]==LOAD || mnu->t[i]==SAVE) {
		V_spr(mnu->x,j=y+i*16+29,mslotl);
		for(k=8;k<184;k+=8)
		  V_spr(mnu->x+k,j,mslotm);
		V_spr(mnu->x+184,j,mslotr);
		Z_gotoxy(mnu->x+4,j-8);
		if(input && i==save_mnu.cur) Z_printsf("%s_",ibuf);
		else Z_printsf("%s",savname[i]);
	  }else{
	    Z_gotoxy(mnu->x+((mnu->t[i]>=SVOLM)?((mnu->t[i]>=PL1CM)?50:152):0),y+i*16+20);
	    Z_printbf(mnu->m[i]);
	  }
	  if(mnu->t[i]==MUSIC) {
	    Z_printbf(" '%.8s'",g_music);
	  }else if(mnu->t[i]==INTERP) {
	    Z_printbf("%s",fullscreen?"ON":"OFF");
	  }else if(mnu->t[i]>=PL1CM) {
	    V_manspr(mnu->x+((mnu->t[i]==PL1CM)?15:35),y+i*16+20+14,
	      PL_getspr(*panimp,0),
	      pcolortab[(mnu->t[i]==PL1CM)?p1color:p2color]
	    );
	  }else if(mnu->t[i]>=SVOLM) {
		V_spr(mnu->x,j=y+i*16+20,mbarl);
		for(k=8;k<144;k+=8)
		  V_spr(mnu->x+k,j,mbarm);
		V_spr(mnu->x+144,j,mbarr);
		switch(mnu->t[i]) {
		  case SVOLM: k=snd_vol;break;
		  case MVOLM: k=mus_vol;break;
		  case GAMMAM: k=gammaa<<5;break;
		}
		V_spr(mnu->x+8+k,j,mbaro);
	  }
    }
    V_spr(mnu->x-25,y+mnu->cur*16+20-8,msklh[(gm_tm/6)&1]);
  }else{
    Z_gotoxy((320-strlen(mnu->ttl)*7)/2,90);Z_printsf(mnu->ttl);
    Z_gotoxy(136,100);Z_printsf("(Y/N)");
  }
  return 1;
}
