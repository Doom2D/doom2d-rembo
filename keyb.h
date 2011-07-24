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

#ifndef MYKEYB
#define MYKEYB

#ifdef __cplusplus
extern "C" {
#endif

void K_init();
void K_done();

// тип функции обработки клавиш
typedef void key_f(int k,int pressed);

// установить функцию обработки клавиш
void K_setkeyproc(key_f *);

// массив клавиш: 0 - отпущена, иначе - нажата
extern unsigned char *keys;

void updatee_keys();

#ifdef __cplusplus
}
#endif

#endif
