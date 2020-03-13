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
#include "vga.h"
#include <SDL.h>
#include "error.h"
#include "view.h"
#include "memory.h"
#include "misc.h"

#include <assert.h>


// адрес экранного буфера
unsigned char *scra;

// виртуальный экран
unsigned char scrbuf[64000];


int SCRW = 800;
int SCRH = 600;

SDL_Surface* screen = NULL;

int cx1,cx2,cy1,cy2;

char fullscreen = OFF;

byte bright[256];
byte mixmap[256][256];
byte clrmap[256*12];

static byte flametab[16] = {
  0xBC,0xBA,0xB8,0xB6,0xB4,0xB2,0xB0,0xD5,0xD6,0xD7,0xA1,0xA0,0xE3,0xE2,0xE1,0xE0
};

extern void *walp[256];

#define HQ 2

vgaimg *V_getvgaimg (int id) {
  int loaded = M_was_locked(id);
  vgaimg *v = M_lock(id);
  if (v != NULL && !loaded) {
    v->w = short2host(v->w);
    v->h = short2host(v->h);
    v->sx = short2host(v->sx);
    v->sy = short2host(v->sy);
  }
  return v;
}

vgaimg *V_loadvgaimg (char *name) {
  return V_getvgaimg(F_getresid(name));
}

short V_init(void)
{
    Uint32 flags = SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_HWPALETTE;
    if (fullscreen) flags = flags | SDL_FULLSCREEN;
    screen = SDL_SetVideoMode(SCRW, SCRH, 8, flags);
    if (!screen) ERR_failinit("Unable to set video mode: %s\n", SDL_GetError());
    SCRW /= HQ;
    SCRH /= HQ;
    return 0;
}

// переключение в текстовый режим
void V_done(void)
{
    SDL_Quit();
}

void draw_rect (int x, int y, int w, int h, int c)
{
    SDL_Rect dstrect;
    dstrect.x = x*HQ;
    dstrect.y = y*HQ;
    dstrect.w = w*HQ;
    dstrect.h = h*HQ;
    SDL_FillRect(screen, &dstrect, c);
}

// установить область вывода
void V_setrect(short x,short w,short y,short h)
{
    SDL_Rect r;
    r.x=x*HQ;
    r.y=y*HQ;
    r.w=w*HQ;
    r.h=h*HQ;
    SDL_SetClipRect(screen, &r);
    SDL_GetClipRect(screen, &r);
    cx1 = x;
    cx2 = x+w-1;
    cy1 = y;
    cy2 = y+h-1;
    if (cx1<0) cx1=0;
    if (cx2>=SCRW) cx2=SCRW-1;
    if (cy1<0) cy1=0;
    if (cy2>=SCRH) cy2=SCRH-1;
}

void putpixel(int x, int y, Uint8 color)
{
    if(x>=cx1 && x<=cx2 && y>=cy1 && y<=cy2) {
        x*=HQ;
        y*=HQ;
        Uint8 *p = (Uint8 *)screen->pixels + y*screen->pitch + x;
        *p = color;
        *(p+1) = color;
        p += screen->pitch;
        *p = color;
        *(p+1) = color;
    }
}

byte getpixel(int x, int y)
{
    if(x>=cx1 && x<=cx2 && y>=cy1 && y<=cy2) {
        x*=HQ;
        y*=HQ;
        return *((Uint8 *)screen->pixels + y*screen->pitch + x);
    }
    return 0;
}

void mappixel(int x,int y,byte* cmap)
{
    byte c = getpixel(x,y);
    putpixel(x,y,cmap[c]);
}

int offx = 0;
int offy = 0;

void V_center(int f)
{
    if (f) V_offset(SCRW/2-320/2, SCRH/2-200/2);
    else V_offset(0, 0);
}

void V_offset(int ox, int oy)
{
    offx=ox;
    offy=oy;
}

void draw_spr(short x,short y,vgaimg *i, int d, int c)
{
    if (i==NULL) return;
    x += offx;
    y += offy;
    if (d & 1) x=x-i->w+i->sx; else x-=i->sx;
    if (d & 2) y=y-i->h+i->sy; else y-=i->sy;
    if(x+i->w>=cx1 && x<=cx2 && y+i->h>=cy1 && y<=cy2) {
        int lx, ly;
        byte *p = (byte*)i + sizeof(vgaimg);
        for (ly=0; ly<i->h; ly++) {
            for(lx=0; lx<i->w; lx++) {
                int rx,ry;
                rx = (d & 1) ? (i->w-lx-1) : (rx=lx);
                ry = (d & 2) ? (i->h-ly-1) : (ry=ly);
                if (*p) {
                    byte t = *p;
                    if (c) if (t>=0x70 && t<=0x7F) t=t-0x70+c;
                    putpixel(x+rx,y+ry,t);
                }
                p++;
            }
        }
    }
}

void V_rotspr (int x, int y, vgaimg* i, int d)
{
    x+=i->w*((d&1)?1:0);
    y+=i->h*((d&2)?1:0);
    draw_spr(x,y,i,d,0);
}

void V_pic(short x,short y,vgaimg *i)
{
    draw_spr(x,y,i, 0, 0);
}

void V_manspr(int x,int y,void *p, unsigned char c)
{
    draw_spr(x,y,p, 0, c);
}

void V_manspr2(int x,int y,void *p, unsigned char c)
{
    draw_spr(x,y,p, 1, c);
}

// вывести точку цвета c в координатах (x,y)
void V_dot(short x,short y, unsigned char c)
{
    putpixel(x,y,c);
}

void smoke_sprf(int x, int y, byte c)
{
    byte t = getpixel(x,y);
    c = c + bright[t];
    c += 0x60;
    c ^= 0xF;
    putpixel(x,y,mixmap[c][t]);
}

void flame_sprf(int x, int y, byte c)
{
    byte t = getpixel(x,y);
    c = c + bright[t];
    putpixel(x,y,flametab[c]);
}

void V_sprf(short x,short y,vgaimg *i,spr_f *f)
{
    if (i==NULL) return;
    x-=i->sx;
    y-=i->sy;
    int cx, cy;
    byte *p = (byte*)i;
    p+=sizeof(vgaimg);
    for (cy=y; cy<y+i->h; cy++) {
        for(cx=x; cx<x+i->w; cx++) {
            if (*p) {
                (*f)(cx, cy, *p);
            }
            p++;
        }
    }
}

void V_spr(short x,short y,vgaimg *i)
{
    draw_spr(x,y,i,0, 0);
}

void V_spr2(short x,short y,vgaimg *i)
{
    draw_spr(x,y,i,1,0);
}

void V_clr(short x,short w,short y,short h,unsigned char c)
{
    draw_rect(x,y,w,h, c);
}

// установить палитру из массива p
void VP_setall(void *p)
{
    VP_set(p, 0, 256);
}

// установить n цветов, начиная с f, из массива p
void VP_set(void *p,short f,short n)
{
    byte *ptr = (byte*)p;
    SDL_Color colors[256];
    int i;
    for(i=f;i<f+n;i++)
    {
      colors[i].r=ptr[0]*4;
      colors[i].g=ptr[1]*4;
      colors[i].b=ptr[2]*4;
      ptr+=3;
    }
    SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, colors, f, n);
}

// установить адрес экранного буфера
// NULL - реальный экран
void V_setscr(void *p)
{
    if (screen) SDL_Flip(screen);
}

// скопировать прямоугольник на экран
void V_copytoscr(short x,short w,short y,short h)
{
    x*=HQ; y*=HQ; w*=HQ; h*=HQ;
    SDL_UpdateRect(screen, x, y, w, h);
}

void V_maptoscr(int x,int w,int y,int h,void *cmap)
{
    int cx,cy;
    for (cx=x; cx<x+w; cx++)
        for (cy=y; cy<y+h; cy++)
            mappixel(cx,cy,(byte*)cmap);
    V_copytoscr(x,w,y,h);
}

void V_remap_rect(int x,int y,int w,int h,byte *cmap)
{
    int cx,cy;
    for (cx=x; cx<x+w; cx++)
        for (cy=y; cy<y+h; cy++)
            mappixel(cx,cy,cmap);
}

void Z_drawfld (byte *fld, int bg)
{
    byte *p = fld;
    int x, y;
    for (y = 0; y < FLDH; y++) {
        for (x = 0; x < FLDW; x++) {
            int sx = x * CELW - w_x + WD / 2;
            int sy = y * CELH - w_y + HT / 2 + 1 + w_o;
            if (*p) {
              vgaimg *pic = walp[*p];
              // special pointer value setted for _WATER_* in view.c
              if ((intptr_t)pic <= 3) {
                if (!bg) {
                  byte *cmap = clrmap + ((intptr_t)pic+7)*256;
                  V_remap_rect(sx, sy, CELW, CELH, cmap);
                }
              } else {
                V_pic(sx, sy, pic);
              }
            }
            p++;
        }
    }
}

void V_toggle()
{
    if (!SDL_WM_ToggleFullScreen(screen)) {
        int ncolors = screen->format->palette->ncolors;
        SDL_Color colors[256];
        int i;
        for (i=0; i<ncolors; i++) {
            colors[i].r = screen->format->palette->colors[i].r;
            colors[i].g = screen->format->palette->colors[i].g;
            colors[i].b = screen->format->palette->colors[i].b;
        }

        Uint32 flags = screen->flags;

        SDL_FreeSurface(screen);

        screen = SDL_SetVideoMode(0, 0, 0, flags ^ SDL_FULLSCREEN);
        if(screen == NULL) {
            ERR_fatal("Unable to set video mode\n");
            exit(1);
        }

        SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, ncolors);
    }
}
