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

// Monsters

#define MAXMN 200

enum{
  MN_NONE,MN_DEMON,MN_IMP,MN_ZOMBY,MN_SERG,MN_CYBER,MN_CGUN,
  MN_BARON,MN_KNIGHT,MN_CACO,MN_SOUL,MN_PAIN,MN_SPIDER,MN_BSP,
  MN_MANCUB,MN_SKEL,MN_VILE,MN_FISH,MN_BARREL,MN_ROBO,MN_MAN,
  MN__LAST,
  MN_PL_DEAD=100,MN_PL_MESS
};

void MN_init(void);
void MN_alloc(void);
int MN_spawn(int,int,byte,int);
int MN_spawn_deadpl(obj_t *,byte,int);
void MN_act(void);
void MN_mark(void);
void MN_draw(void);
void MN_warning(int l,int t,int r,int b);

void MN_killedp(void);
