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
#include "error.h"
#include "sound.h"
#include "view.h"
#include "player.h"
#include "switch.h"
#include "menu.h"
#include "misc.h"
#include "render.h"
#include "config.h"
#include "game.h"
#include "player.h"
#include "sound.h"
#include "music.h"
#include "input.h"

#include <sys/stat.h>

#define QSND_NUM 14

enum{HIT100,ARMOR,JUMP,WPNS,IMMORTAL,SPEED,OPEN,EXIT};

static byte panim[] = "BBDDAACCDDAABBDDAACCDDAABBDDAACCDDAAEEEEEFEFEFEFEFEFEFEFEFEFEEEEE";
byte *panimp = panim;
byte _warp;

byte pcolortab[PCOLORN] = {
  0x18, 0x20, 0x40, 0x58, 0x60, 0x70, 0x80, 0xB0, 0xC0, 0xD0
};
int p1color = 5;
int p2color = 4;

char ibuf[24];
byte input=0;
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

menu_t main_mnu={
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

menu_t *mnu=NULL;
byte gm_redraw=0;

short lastkey=0;
static void *csnd1,*csnd2,*msnd1,*msnd2,*msnd3,*msnd4,*msnd5,*msnd6;
static int movsndt=0;
static byte cbuf[32];

static snd_t *voc=NULL;
static int voc_ch=0;

static void GMV_stop (void) {
  if(voc) {
    if(voc_ch) {S_stop(voc_ch);voc_ch=0;}
    free(voc);voc=NULL;
  }
}

void GMV_say (char *nm) {
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

static void GM_set (menu_t *m) {
  mnu=m;gm_redraw=1;
  if(g_st==GS_GAME) {
	//V_setrect(0,SCRW,0,SCRH);V_clr(0,SCRW,0,SCRH,0);//V_setrect(0,320,0,200);V_clr(0,320,0,200,0);
	//if(_2pl) {V_setrect(SCRW-120,120,0,SCRH);w_o=0;Z_clrst();w_o=SCRH/2;Z_clrst();}//if(_2pl) {V_setrect(200,120,0,200);w_o=0;Z_clrst();w_o=100;Z_clrst();}
	//else {V_setrect(SCRW-120,120,0,SCRH);w_o=0;Z_clrst();}//else {V_setrect(200,120,50,100);w_o=50;Z_clrst();}
	//pl1.drawst=pl2.drawst=0xFF;V_setrect(0,SCRW,0,SCRH);//V_setrect(0,320,0,200);
  }
}

void G_code (void) {
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

static void GM_command (int c) {
  switch(c) {
    case CANCEL:
      GM_set(NULL);break;
    case INTERP:
      R_toggle_fullscreen();
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
	case GAMMAM:
    R_setgamma(R_getgamma() - 1);
    break;
	case GAMMAP:
    R_setgamma(R_getgamma() + 1);
    break;
  }
}

struct {
    int keysym;
    byte ch;
} keychar[] = {
    {KEY_SPACE, ' '},
    {KEY_0, '0'},
    {KEY_1, '1'},
    {KEY_2, '2'},
    {KEY_3, '3'},
    {KEY_4, '4'},
    {KEY_5, '5'},
    {KEY_6, '6'},
    {KEY_7, '7'},
    {KEY_8, '8'},
    {KEY_9, '9'},
    //{KEY_UNDERSCORE, '_'},
    {KEY_A, 'A'},
    {KEY_B, 'B'},
    {KEY_C, 'C'},
    {KEY_D, 'D'},
    {KEY_E, 'E'},
    {KEY_F, 'F'},
    {KEY_G, 'G'},
    {KEY_H, 'H'},
    {KEY_I, 'I'},
    {KEY_J, 'J'},
    {KEY_K, 'K'},
    {KEY_L, 'L'},
    {KEY_M, 'M'},
    {KEY_N, 'N'},
    {KEY_O, 'O'},
    {KEY_P, 'P'},
    {KEY_Q, 'Q'},
    {KEY_R, 'R'},
    {KEY_S, 'S'},
    {KEY_T, 'T'},
    {KEY_U, 'U'},
    {KEY_V, 'V'},
    {KEY_W, 'W'},
    {KEY_X, 'X'},
    {KEY_Y, 'Y'},
    {KEY_Z, 'Z'},
    {KEY_COMMA,','},
    {0}
};

static byte get_keychar (int keysym) {
    int i = 0;
    while (keychar[i].keysym) {
        if (keychar[i].keysym == keysym) return keychar[i].ch;
        i++;
    }
    return 0;
}

static void shot (void) {
/*
  static int num=1;
  char fn[100];//...
#ifndef WIN32
  char *e = getenv("HOME");
  strncpy(fn, e, 60);
  sprintf(&fn[strlen(fn)],"/.doom2d-rembo",num);
  mkdir(fn, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  sprintf(&fn[strlen(fn)],"/shot%04d.bmp",num);
#else
  sprintf(fn,"shot%04d.bmp",num);
#endif
  SDL_SaveBMP(screen, fn);
  ++num;
*/
}

int GM_act (void) {
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
  if (input) {
    switch (lastkey) {
      case KEY_RETURN:
      case KEY_KP_ENTER:
        F_savegame(save_mnu.cur, ibuf);
        input = 0;
        GM_set(NULL);
        break;
      case KEY_ESCAPE:
        input = 0;
        GM_set(mnu);
        break;
      case KEY_BACKSPACE:
        if (icur) {
          icur -= 1;
          ibuf[icur] = 0;
          GM_set(mnu);
        }
        break;
      default:
        if (icur < 23) {
          c = get_keychar(lastkey);
          if (c != 0) {
            ibuf[icur] = c;
            icur += 1;
            ibuf[icur] = 0;
            GM_set(mnu);
          }
        }
        break;
    }
  } else {
    switch (lastkey) {
      case KEY_ESCAPE:
        if (mnu == NULL) {
          GM_set(&main_mnu);
          Z_sound(msnd3, 128);
        } else {
          GM_set(NULL);
          Z_sound(msnd4, 128);
        }
        break;
      case KEY_F5:
        if (mnu == NULL) {
          Z_sound(msnd3, 128);
          GMV_say("_GAMMA");
          GM_set(&gamma_mnu);
        }
        break;
      case KEY_F4:
        if (mnu == NULL) {
          Z_sound(msnd3, 128);
          GMV_say("_VOLUME");
          GM_set(&vol_mnu);
        }
        break;
      case KEY_F2:
        if (mnu == NULL && g_st == GS_GAME) {
          Z_sound(msnd3, 128);
          F_getsavnames();
          GM_set(&save_mnu);
        }
        break;
      case KEY_F3:
        if (mnu == NULL) {
          Z_sound(msnd3, 128);
          F_getsavnames();
          GM_set(&load_mnu);
        }
        break;
      case KEY_F10:
        if (mnu == NULL) {
          Z_sound(msnd3, 128);
          GM_command(QUITGAME);
        }
        break;
      case KEY_UP:
      case KEY_KP_8:
        if (mnu != NULL && mnu->type == MENU) {
          mnu->cur -= 1;
          if (mnu->cur < 0) {
            mnu->cur = mnu->n - 1;
          }
          GM_set(mnu);
          Z_sound(msnd1, 128);
        }
        break;
      case KEY_DOWN:
      case KEY_KP_5:
      case KEY_KP_2:
        if (mnu != NULL && mnu->type == MENU) {
          mnu->cur += 1;
          if (mnu->cur >= mnu->n) {
            mnu->cur = 0;
          }
          GM_set(mnu);
          Z_sound(msnd1, 128);
        }
        break;
      case KEY_LEFT:
      case KEY_RIGHT:
      case KEY_KP_4:
      case KEY_KP_6:
        if (mnu != NULL && mnu->type == MENU && mnu->t[mnu->cur] >= SVOLM) {
          GM_command(mnu->t[mnu->cur] + (lastkey == KEY_LEFT || lastkey == KEY_KP_4));
          GM_set(mnu);
          if (!movsndt) {
            movsndt = Z_sound(lastkey == KEY_LEFT || lastkey == KEY_KP_4 ? msnd5 : msnd6, 255);
          }
        }
        break;
      case KEY_RETURN:
      case KEY_SPACE:
      case KEY_KP_ENTER:
        if (mnu != NULL && mnu->type == MENU) {
          if (mnu->t[mnu->cur] >= PL1CM) {
            Z_sound(msnd2, 128);
            GM_command(PLCEND);
            break;
          } else if (mnu->t[mnu->cur] < SVOLM) {
            Z_sound(msnd2,128);
            GM_command(mnu->t[mnu->cur]);
          }
        }
        break;
      case KEY_Y:
        if (mnu != NULL && mnu->type == MSG) {
          Z_sound(msnd3, 128);
          GM_command(mnu->t[0]);
        }
        break;
      case KEY_N:
        if (mnu != NULL && mnu->type == MSG) {
          Z_sound(msnd4, 128);
          GM_command(mnu->t[1]);
        }
        break;
      case KEY_F1:
        if (shot_vga) {
          shot();
          Z_sound(msnd4, 128);
        }
        break;
    }
  }
  lastkey = KEY_UNKNOWN;
  return mnu ? 1 : 0;
}

void G_keyf (int key, int down) {
  int i;
  if (down) {
    lastkey = key;
    if (!_2pl || cheat) {
      for (i = 0; i < 31; ++i) {
        cbuf[i] = cbuf[i + 1];
      }
      cbuf[31] = get_keychar(key);
    }
  }
}

void GM_init (void) {
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
  F_loadmus("MENU");
  S_startmusic(0);
}
