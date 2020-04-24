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
#include "my.h"
#include "render.h"

int sky_type=1;
dword walf[256];
byte fldb[FLDH][FLDW];
byte fldf[FLDH][FLDW];
byte fld[FLDH][FLDW];

void W_init (void) {
  DOT_init();
  SMK_init();
  FX_init();
  WP_init();
  IT_init();
  SW_init();
  PL_init();
  MN_init();
  R_loadsky(1);
}
