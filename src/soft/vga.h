/*
   Управление графикой VGA для DOS4GW
   Модуль версии 1.0
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

#ifndef VGA_H_INCLUDED
#define VGA_H_INCLUDED

#include "glob.h"

#pragma pack(1)
typedef struct {
  unsigned short w, h; // W-ширина,H-высота
  short sx, sy;    // сдвиг центра изображения
} vgaimg;
#pragma pack()

// карта цветов
typedef unsigned char colormap[256];

typedef void spr_f(int, int, unsigned char);

extern int SCRW;
extern int SCRH;
extern char fullscreen;

extern byte *buffer;
extern int buf_w, buf_h, pitch;

extern byte bright[256];
extern byte mixmap[256][256];
extern byte clrmap[256*12];

vgaimg *V_getvgaimg (int id);
vgaimg *V_loadvgaimg (char *name);

// ждать обратного хода луча развертки
void V_wait (void);

// вывести картинку i в координатах (x,y)
void V_pic (short x, short y, vgaimg *i);

// вывести спрайт i в координатах (x,y) при помощи функции f
//   параметры: AL=цвет_точки_спрайта EDI=>экран
//   !!! сохраняйте все регистры, кроме EAX
//   !!! EDI должен быть переведен на точку вправо (INC EDI или STOSB)
void V_sprf (short x, short y, vgaimg *i, spr_f *f);

void smoke_sprf (int x, int y, unsigned char c);
void flame_sprf (int x, int y, unsigned char c);

// вывести спрайт i в координатах (x,y)
void V_spr (short x, short y, vgaimg *i);

// вывести зеркально перевернутый спрайт i в координатах (x,y)
void V_spr2 (short x, short y, vgaimg *i);

// вывести точку цвета c в координатах (x,y)
void V_dot (short x, short y, unsigned char c);

void V_manspr (int x, int y, void *p, unsigned char c);
void V_manspr2 (int x, int y, void *p, unsigned char c);

// очистить прямоугольник цветом c
// x-левая сторона,w-ширина,y-верх,h-высота
void V_clr (short x, short w, short y, short h, unsigned char c);

// установить область вывода
void V_setrect (short x, short w, short y, short h);

// установить адрес экранного буфера
// NULL - реальный экран
void V_setscr (void *);

// скопировать прямоугольник на экран
void V_copytoscr (short x, short w, short y, short h);

void V_maptoscr (int, int, int, int, void *);

// переделать изображение i по карте цветов m
void V_remap (vgaimg *i, colormap m);

void V_remap_rect (int x, int y, int w, int h, byte *cmap);

void V_rotspr (int x, int y, vgaimg* i, int d);
void V_center (int f);
void V_offset (int ox, int oy);

#endif /* VGA_H_INCLUDED */
