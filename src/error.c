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
#include <stdarg.h>
#include <stdlib.h>
#include "sound.h"
#include "render.h"
#include "memory.h"
#include "error.h"
#include "config.h"
#include "music.h"

void logo (const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  vprintf(s, ap);
  va_end(ap);
  fflush(stdout);
}

void logo_gas (int cur, int all) {
  // stub
}

static void close_all (void) {
  S_done();
  S_donemusic();
  M_shutdown();
  R_done();
}

void ERR_failinit (char *s, ...) {
  va_list ap;
  //close_all();
  va_start(ap, s);
  vprintf(s, ap);
  va_end(ap);
  puts("");
  exit(1);
}

void ERR_fatal (char *s,...) {
  va_list ap;
  close_all();
  puts("\nКРИТИЧЕСКАЯ ОШИБКА:");
  va_start(ap, s);
  vprintf(s, ap);
  va_end(ap);
  puts("");
  exit(2);
}

void ERR_quit (void) {
  puts("Спасибо за то, что вы играли в Операцию \"Смятка\"!");
  //F_loadres(F_getresid("ENDOOM"),p,0,4000);
  close_all();
  CFG_save();
  exit(0);
}
