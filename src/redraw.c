/* Copyright (C) 1996-1997 Aleksey Volynskov
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

/*
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//#include "..\averr.h"
#include "vga.h"
#include "glob.h"

#define MAXRECT 500

typedef struct{short x,w,y,h;}rect;

static rect *rt,*rp;
static redraw_f *fn;
static short sl,st,sr,sb;


void *RD_init(short x,short w,short y,short h) {
  rect *p;

  if(!(p=malloc((MAXRECT*2+1)*sizeof(rect))))
    {error(0,0,ENOMEM,NULL,NULL);return NULL;}//{error(EZ_VGALIB,ET_STD,ENOMEM,NULL,NULL);return NULL;}
  p->x=x;p->w=x+w-1;p->y=y;p->h=y+h-1;
  memset(p+1,0,MAXRECT*2*sizeof(rect));
  return p;
}

void RD_start(void *p,redraw_f *f) {
  rt=p;fn=f;
  sl=rt->x;sr=rt->w;
  st=rt->y;sb=rt->h;
  ++rt;
  rp=rt;
}

static void add(short x,short r,short y,short b) {
  int i;
  short rx,rr,ry,rb;

  for(i=0;i<MAXRECT;++i) if(rp[i].w) {
    if(x>(rr=(rx=rp[i].x)+rp[i].w-1)) continue;
    if(r<rx) continue;
    if(y>(rb=(ry=rp[i].y)+rp[i].h-1)) continue;
    if(b<ry) continue;
    if(x<rx) add(x,rx-1,y,b);
    if(r>rr) add(rr+1,r,y,b);
    if(y<ry) add(max(rx,x),min(rr,r),y,ry-1);
    if(b>rb) add(max(rx,x),min(rr,r),rb+1,b);
    return;
  }
  for(i=0;i<MAXRECT;++i) if(!rp[i].w) {
    rp[i].x=x;rp[i].y=y;rp[i].w=r-x+1;rp[i].h=b-y+1;
    return;
  }
}

void RD_rect(short x,short w,short y,short h) {
  if(x>sr) return;
  if(x+w<=sl) return;
  if(y>sb) return;
  if(y+h<=st) return;
  w+=x-1;h+=y-1;
  if(x<sl) x=sl;
  if(y<st) y=st;
  if(w>sr) w=sr;
  if(h>sb) h=sb;
  add(x,w,y,h);
}

void RD_spr(short x,short y,vgaimg *v) {
  RD_rect(x-v->sx,v->w,y-v->sy,v->h);
}

void RD_spr2(short x,short y,vgaimg *v) {
  RD_rect(x-v->w+v->sx,v->w,y-v->sy,v->h);
}

void RD_end(void) {
  int i;

  V_setscr(scrbuf);
  rp=rt+MAXRECT;
  for(i=0;i<MAXRECT;++i) if(rt[i].w) {
    add(rt[i].x,rt[i].x+rt[i].w-1,rt[i].y,rt[i].y+rt[i].h-1);
  }
  for(i=0;i<MAXRECT;++i) if(rp[i].w) {
    scrx=rp[i].x;scrw=rp[i].w;scry=rp[i].y;scrh=rp[i].h;
    fn();
  }
  for(i=0;i<MAXRECT;++i) if(rp[i].w) {
    V_copytoscr(rp[i].x,rp[i].w,rp[i].y,rp[i].h);
  }
  memcpy(rp,rt,MAXRECT*sizeof(rect));
  memset(rt,0,MAXRECT*sizeof(rect));
}
*/
