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
#include "error.h"
#include "view.h"
#include "memory.h"
#include "misc.h"
#include "files.h"
#include "system.h"

#include <string.h>
#include <assert.h>

int SCRW = 800;
int SCRH = 600;
char fullscreen = OFF;

byte bright[256];
byte mixmap[256][256];
byte clrmap[256*12];

byte *buffer;
int buf_w, buf_h, pitch;
static int offx, offy;
static int cx1, cx2, cy1, cy2;
static byte flametab[16] = {
  0xBC,0xBA,0xB8,0xB6,0xB4,0xB2,0xB0,0xD5,0xD6,0xD7,0xA1,0xA0,0xE3,0xE2,0xE1,0xE0
};

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

static void draw_rect (int x, int y, int w, int h, int c) {
  int i;
  int x0 = max(x, cx1);
  int y0 = max(y, cy1);
  int x1 = min(x + w - 1, cx2);
  int y1 = min(y + h - 1, cy2);
  int len = x1 - x0;
  for (i = y0; i <= y1; i++) {
    memset(&buffer[i * pitch + x0], c, len);
  }
}

void V_setrect (short x, short w, short y, short h) {
  cx1 = max(x, 0);
  cx2 = min(x + w - 1, buf_w - 1);
  cy1 = max(y, 0);
  cy2 = min(y + h - 1, buf_h - 1);
}

static void putpixel (int x, int y, byte color) {
  if (x >= cx1 && x <= cx2 && y >= cy1 && y <= cy2) {
    buffer[y * pitch + x] = color;
  }
}

static byte getpixel (int x, int y) {
  return x >= cx1 && x <= cx2 && y >= cy1 && y <= cy2 ? buffer[y * pitch + x] : 0;
}

static void mappixel (int x, int y, byte *cmap) {
  byte c = getpixel(x, y);
  putpixel(x, y, cmap[c]);
}

void V_center (int f) {
  if (f) {
    V_offset(buf_w / 2 - 320 / 2, buf_h / 2 - 200 / 2);
  } else {
    V_offset(0, 0);
  }
}

void V_offset (int ox, int oy) {
  offx = ox;
  offy = oy;
}

static void draw_spr (short x, short y, vgaimg *i, int d, int c) {
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
                rx = (d & 1) ? (i->w-lx-1) : (lx);
                ry = (d & 2) ? (i->h-ly-1) : (ly);
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

void V_rotspr (int x, int y, vgaimg* i, int d) {
    x+=i->w*((d&1)?1:0);
    y+=i->h*((d&2)?1:0);
    draw_spr(x,y,i,d,0);
}

void V_pic (short x, short y, vgaimg *i) {
    draw_spr(x, y, i, 0, 0);
}

void V_manspr (int x, int y, void *p, unsigned char c) {
    draw_spr(x, y, p, 0, c);
}

void V_manspr2(int x,int y,void *p, unsigned char c) {
    draw_spr(x, y, p, 1, c);
}

void V_dot (short x, short y, unsigned char c) {
    putpixel(x, y, c);
}

void smoke_sprf (int x, int y, byte c) {
    byte t = getpixel(x,y);
    c = c + bright[t];
    c += 0x60;
    c ^= 0xF;
    putpixel(x,y,mixmap[c][t]);
}

void flame_sprf (int x, int y, byte c) {
    byte t = getpixel(x,y);
    c = c + bright[t];
    putpixel(x,y,flametab[c]);
}

void V_sprf (short x,short y,vgaimg *i,spr_f *f) {
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

void V_spr (short x, short y, vgaimg *i) {
    draw_spr(x, y, i, 0, 0);
}

void V_spr2 (short x, short y, vgaimg *i) {
    draw_spr(x,y,i,1,0);
}

void V_clr (short x, short w, short y, short h, unsigned char c) {
    draw_rect(x, y, w, h, c);
}

void V_setscr (void *p) {
  Y_repaint();
}

void V_copytoscr (short x, short w, short y, short h) {
  Y_repaint_rect(x, y, w, h);
}

void V_maptoscr (int x, int w, int y, int h, void *cmap) {
    int cx,cy;
    for (cx=x; cx<x+w; cx++)
        for (cy=y; cy<y+h; cy++)
            mappixel(cx,cy,(byte*)cmap);
    V_copytoscr(x,w,y,h);
}

void V_remap_rect (int x, int y, int w, int h, byte *cmap) {
    int cx,cy;
    for (cx=x; cx<x+w; cx++)
        for (cy=y; cy<y+h; cy++)
            mappixel(cx,cy,cmap);
}
