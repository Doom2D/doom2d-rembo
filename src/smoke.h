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

// Smoke

#define MAXSMOK 500

void SMK_init(void);
void SMK_alloc(void);
void SMK_act(void);
void SMK_draw(void);
void SMK_add(int x,int y,int xv,int yv,byte t,byte s,short o);
void SMK_gas(int x,int y,int rx,int ry,int xv,int yv,int k);
void SMK_flame(int x,int y,int ox,int oy,int rx,int ry,int xv,int yv,int k,int o);
