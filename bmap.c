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
    BM_clear(BM_WALL);
    int x,y;
    for(x=0; x<FLDW; x++)
        for(y=0; y<FLDH; y++)
            if (fld[y][x] == 1 || fld[y][x] == 2)
                bmap[y/4][x/4]|=BM_WALL;
    fld_need_remap = 0;
}
