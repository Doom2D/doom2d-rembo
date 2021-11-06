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

#include <stdio.h>
#include "glob.h"
#include "view.h"
#include "bmap.h"

byte fld_need_remap=1;

byte bmap[FLDH/4][FLDW/4];

void BM_mark(obj_t *o,byte f) {
  int x,y;
  int xs,ys,xe,ye;

  if((xs=(o->x-o->r)>>5)<0) xs=0;
  if((xe=(o->x+o->r)>>5)>=FLDW/4) xe=FLDW/4-1;
  if((ys=(o->y-o->h)>>5)<0) ys=0;
  if((ye=o->y>>5)>=FLDH/4) ye=FLDH/4-1;
  for(y=ys;y<=ye;++y)
	for(x=xs;x<=xe;++x)
	  bmap[y][x]|=f;
}

void BM_clear(byte f)
{
    int x,y;
    for(x=0; x<FLDW/4; x++)
        for (y=0; y<FLDH/4; y++)
            bmap[y][x]&=~f;
}

void BM_remapfld(void)
{
    int x,y;
    BM_clear(BM_WALL);
    for(x=0; x<FLDW; x++)
        for(y=0; y<FLDH; y++)
            if (fld[y][x] == 1 || fld[y][x] == 2)
                bmap[y/4][x/4]|=BM_WALL;
    fld_need_remap = 0;
}
