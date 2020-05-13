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
