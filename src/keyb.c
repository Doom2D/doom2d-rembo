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

#include "keyb.h"
#include "error.h"
#include <SDL.h>

// массив клавиш: 0 - отпущена, иначе - нажата
unsigned char *keys = NULL;

static key_f *key_proc = NULL;

void K_init (void) {
  keys = SDL_GetKeyState(NULL);
}

void K_done (void) {
  // not needed for sdl
}

// установить функцию обработки клавиш
void K_setkeyproc (key_f *k) {
  key_proc = k;
}

void updatee_keys (void) {
    SDL_Event event;
    while (SDL_PollEvent (&event)) {
      /*
       if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN && event.key.keysym.mod & KMOD_LALT) {
            V_toggle();
       }
       else */
       if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
         if (event.type == SDL_KEYDOWN && key_proc!=NULL)
             key_proc(event.key.keysym.sym, event.type == SDL_KEYDOWN);
       }
       else if (event.type == SDL_QUIT) {
           ERR_quit();
       }
    }
}
