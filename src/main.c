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
///#include <process.h>
#include <stdarg.h>
///#include <conio.h>
#include <time.h>
///#include <dos.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "error.h"
#include "config.h"
#include "memory.h"
#include "keyb.h"
#include "sound.h"
#include "vga.h"
#include "files.h"
#include "view.h"
#include "menu.h"
#include "player.h"
#include "misc.h"

#include <SDL.h>

int gammaa=0;

char main_pal[256][3],std_pal[256][3];
byte mixmap[256][256];
byte clrmap[256*12];

void logo(const char *s,...) {
  va_list ap;
  int x,y;

  va_start(ap,s);
  vprintf(s,ap);
  va_end(ap);
  fflush(stdout);
}

void logo_gas(int cur,int all) {
}

byte gamcor[5][64]={
  #include "gamma.dat"
};

void setgamma(int g) {
  int t;

  if(g>4) g=4;
  if(g<0) g=0;
  gammaa=g;
  for(t=0;t<256;++t) {
	std_pal[t][0]=gamcor[gammaa][main_pal[t][0]];
	std_pal[t][1]=gamcor[gammaa][main_pal[t][1]];
	std_pal[t][2]=gamcor[gammaa][main_pal[t][2]];
  }
  VP_setall(std_pal);
}

void myrandomize(void);

byte bright[256];

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO)<0) ERR_failinit("Unable to init SDL: %s\n", SDL_GetError());
  SDL_WM_SetCaption("Doom 2D v1.351", "Doom 2D");
  int i;
  
  pl1.ku=SDLK_KP8;
  pl1.kd=SDLK_KP5;
  pl1.kl=SDLK_KP4;
  pl1.kr=SDLK_KP6;
  pl1.kf=SDLK_PAGEDOWN;
  pl1.kj=SDLK_DELETE;
  pl1.kwl=SDLK_HOME;
  pl1.kwr=SDLK_END;
  pl1.kp=SDLK_KP8;
  pl1.id=-1;
  pl2.ku=SDLK_e;
  pl2.kd=SDLK_d;
  pl2.kl=SDLK_s;
  pl2.kr=SDLK_f;
  pl2.kf=SDLK_a;
  pl2.kj=SDLK_q;
  pl2.kwl=SDLK_1;
  pl2.kwr=SDLK_2;
  pl2.kp=SDLK_e;
  pl2.id=-2;
  myrandomize();
  F_startup();
  F_addwad("doom2d.wad");
  CFG_args(argc, argv);
  CFG_load();
  F_initwads();
  M_startup();
  F_allocres();
  F_loadres(F_getresid("PLAYPAL"),main_pal,0,768);
  for(i=0;i<256;++i)
    bright[i]=((int)main_pal[i][0]+main_pal[i][1]+main_pal[i][2])*8/(63*3);
  F_loadres(F_getresid("MIXMAP"),mixmap,0,0x10000);
  F_loadres(F_getresid("COLORMAP"),clrmap,0,256*12);
  G_init();
  K_init();
  logo("S_init: настройка звука\n");
  S_init();
  S_initmusic();
  logo("V_init: настройка видео\n");
  if(V_init()!=0) ERR_failinit("Не могу установить видеорежим VGA");
  setgamma(gammaa);
  V_setscr(scrbuf);
  GM_init();
  F_loadmus("MENU");S_startmusic(0);
  
  Uint32 ticks = 0;
  for(;;) {

    updatee_keys();

    S_updatemusic();

    G_act();

    G_draw();
    
    Uint32 t;
    while ((t = SDL_GetTicks())-ticks < DELAY) {
        SDL_Delay(1);
    }
    ticks = t - t % DELAY;

  }
}
