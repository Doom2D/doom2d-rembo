/*
   Драйвер клавиатуры V1.1 для DOS4GW (а также DirectX 3)
   Copyright (C) Алексей Волынсков, 1996

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

#ifndef KEYB_H_INCLUDED
#define KEYB_H_INCLUDED

typedef void key_f (int k, int pressed);

extern unsigned char *keys;

void K_init (void);
void K_done (void);
void K_setkeyproc (key_f *k);
void updatee_keys (void);

#endif /* KEYB_H_INCLUDED */
