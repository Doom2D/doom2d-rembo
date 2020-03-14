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

#ifdef __cplusplus
extern "C" {
#endif

// заголовок изображения
#pragma pack(1)
typedef struct{
  unsigned short w,h; // W-ширина,H-высота
  short sx,sy;    // сдвиг центра изображения
}vgaimg;

// R-красный,G-зеленый,B-синий
typedef struct{
  unsigned char r,g,b;
}rgb_t;
#pragma pack()

// 256-и цветовая палитра VGA
typedef rgb_t vgapal[256];

// карта цветов
typedef unsigned char colormap[256];

// тип функции перерисовки экрана
typedef void redraw_f(void);

typedef void spr_f(int, int, unsigned char);//typedef void spr_f(void);

vgaimg *V_getvgaimg (int id);
vgaimg *V_loadvgaimg (char *name);

// переключение в режим VGA 320x200,256 цветов
// возвращает 0, если все о'кей
short V_init(void);

// переключение в текстовый режим
void V_done(void);

// ждать обратного хода луча развертки
void V_wait(void);

// вывести картинку i в координатах (x,y)
void V_pic(short x,short y,vgaimg *i);

// вывести спрайт i в координатах (x,y) при помощи функции f
//   параметры: AL=цвет_точки_спрайта EDI=>экран
//   !!! сохраняйте все регистры, кроме EAX
//   !!! EDI должен быть переведен на точку вправо (INC EDI или STOSB)
void V_sprf(short x,short y,vgaimg *i,spr_f *f);

void smoke_sprf(int x, int y, unsigned char c);
void flame_sprf(int x, int y, unsigned char c);

// вывести спрайт i в координатах (x,y)
void V_spr(short x,short y,vgaimg *i);

// вывести зеркально перевернутый спрайт i в координатах (x,y)
void V_spr2(short x,short y,vgaimg *i);

// вывести форму спрайта i в координатах (x,y) одним цветом c
// (подходит для рисования теней)
void V_spr1color(short x,short y,vgaimg *i,unsigned char c);

// вывести точку цвета c в координатах (x,y)
void V_dot(short x,short y,unsigned char c);


void V_manspr(int x,int y,void *p,unsigned char c);

void V_manspr2(int x,int y,void *p,unsigned char c);


// очистить прямоугольник цветом c
// x-левая сторона,w-ширина,y-верх,h-высота
void V_clr(short x,short w,short y,short h,unsigned char c);

// получить текущую палитру в массив p
void VP_getall(void *p);

// установить палитру из массива p
void VP_setall(void *p);

// установить n цветов, начиная с f, из массива p
void VP_set(void *p,short f,short n);

// заполнить палитру одним цветом (r,g,b)
void VP_fill(char r,char g,char b);

// приблизить палитру p к цвету (r,g,b) на s/n
// полученная палитра находится в pal_tmp
void VP_tocolor(void *p,char r,char g,char b,char n,char s);

// приблизить палитру p к палитре p2 на s/n
// полученная палитра находится в pal_tmp
void VP_topal(void *p,void *p2,char n,char s);

// вывести перемасштабированный к размерам (sx,sy) спрайт i в координатах (x,y)
void VM_spr(short x,short y,short sx,short sy,vgaimg *i);

// установить область вывода
void V_setrect(short x,short w,short y,short h);

// установить адрес экранного буфера
// NULL - реальный экран
void V_setscr(void *);

// скопировать прямоугольник на экран
void V_copytoscr(short x,short w,short y,short h);

void V_maptoscr(int,int,int,int,void *);

// вывести символ c
void V_putch(short c);

// вывести строку s
void V_puts(char *s);

// вычислить длину строки s (в точках)
short V_strlen(char *s);

// вычислить ширину символа c (в точках)
short V_chrlen(char c);

// начать анимацию a
void V_start_anim(void *a);

// нарисовать кадр анимации
// возвращает номер кадра или 0, если конец
short V_draw_anim(void);


// полное описание функций RD_* см. в файле REDRAW.DOC

// инициализация экрана
void *RD_init(short x,short w,short y,short h);

// начать перерисовку экрана s при помощи функции f
void RD_start(void *s,redraw_f *f);

// перерисовать прямоугольник
void RD_rect(short x,short w,short y,short h);

// перерисовать спрайт
void RD_spr(short x,short y,vgaimg *s);
void RD_spr2(short x,short y,vgaimg *s);

// окончание перерисовки
void RD_end(void);


// возвращает яркость цвета (r,g,b)
short VP_brightness(char r,char g,char b);

// начать поиск по палитре p, состоящей из n цветов, первый - f
void VP_start_search(rgb_t *p,short n,short f);

// найти подходящий цвет
short VP_findcolor(short r,short g,short b);

// переделать изображение i по карте цветов m
void V_remap(vgaimg *i,colormap m);


// загружает палитру p из файла f (VGAED 2.0)
// возвращает 0 при ошибке
short load_pal(char *f,void *p);

// загружает изображение i из файла f (VGAED 2.0)
// возвращает NULL при ошибке
vgaimg *load_vga(char *f,char *i);

// загружает анимацию из файла f
// возвращает NULL при ошибке
void *load_anim(char *f);

// область вывода
extern short scrw,scrh,scrx,scry;

// адрес экранного буфера
extern unsigned char *scra;

// временная палитра
extern vgapal pal_tmp;

// ждать ли обратного хода луча при установке палитры
// 0-нет, иначе-да
extern char vp_waitrr;

// координаты для вывода следующего символа
extern short vf_x,vf_y;

// расстояние между символами
extern short vf_step;

// цвет символов
// если 0 - не используется
extern unsigned char vf_color;

// шрифт
extern void *vf_font;

// виртуальный экран
extern unsigned char scrbuf[64000];

void V_remap_rect(int x,int y,int w,int h,byte *cmap);

void V_toggle();
void V_rotspr (int x, int y, vgaimg* i, int d);
void V_center(int f);
void V_offset(int ox, int oy);
extern char fullscreen;


#ifdef __cplusplus
}
#endif
