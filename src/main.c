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
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "config.h"
#include "memory.h"
#include "keyb.h"
#include "sound.h"
#include "files.h"
#include "view.h"
#include "menu.h"
#include "player.h"
#include "my.h"
#include "render.h"
#include "game.h"
#include "music.h"

#include <SDL.h>

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
  char *pw;
#ifndef WIN32 
  pw = "/usr/share/doom2d-rembo/doom2d.wad";
#else
  pw = "doom2d.wad";
#endif
  if (fexists(pw))
    F_addwad(pw);
  else
    F_addwad("doom2d.wad");
  CFG_args(argc, argv);
  CFG_load();
  F_initwads();
  M_startup();
  F_allocres();
  G_init();
  K_init();
  logo("S_init: настройка звука\n");
  S_init();
  S_initmusic();
  R_init();
  GM_init();
  F_loadmus("MENU");
  S_startmusic(0);
  
  Uint32 ticks = SDL_GetTicks();
  for (;;) {
    updatee_keys();
    S_updatemusic();
    Uint32 t = SDL_GetTicks();
    if (t - ticks > DELAY) {
      ticks = t;
      G_act();
    }
    R_draw();
    //SDL_Delay(1);
  }
}
