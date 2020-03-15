/*
   Copyright (C) Prikol Software 1996-1997
   Copyright (C) Aleksey Volynskov 1996-1997

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

#ifndef BMAP_H_INCLUDED
#define BMAP_H_INCLUDED

#include "glob.h"
#include "view.h" // obj_t

#define BM_WALL		1
#define BM_PLR1		2
#define BM_PLR2		4
#define BM_MONSTER	8

extern byte fld_need_remap;
extern byte bmap[FLDH/4][FLDW/4];

void BM_clear (byte f);
void BM_mark (obj_t *o, byte f);
void BM_remapfld (void);

#endif /* BMAP_H_INCLUDED */
