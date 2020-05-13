/* Copyright (C) 1996-1997 Aleksey Volynskov
 * Copyright (C) 2011 Rambo
 * Copyright (C) 2020 SovietPony
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

#include "glob.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "files.h"
#include "memory.h"

static byte m_active;
static void *resp[MAX_WAD];
static short resl[MAX_WAD];

void M_startup (void) {
  if(m_active) return;
  logo("M_startup: настройка памяти\n");
  memset(resp,0,sizeof(resp));
  memset(resl,0,sizeof(resl));
  //  logo("  свободно DPMI-памяти: %uK\n",dpmi_memavl()>>10);
  m_active=TRUE;
}

void M_shutdown (void) {
  if(!m_active) return;
  m_active=FALSE;
}

static void allocres (int h) {
  int *p,s;

  if(h>d_start && h<d_end) s=1; else s=0;
  if(!(p=malloc(wad[h].l+4+s*8)))
    ERR_fatal("M_lock: не хватает памяти");
  *p=h;
  ++p;
  resp[h]=p;
  if(s) {
    p[0]=p[1]=p[2]=p[3]=0;
    F_loadres(h,p,0,2);
    F_loadres(h,p+1,2,2);
    F_loadres(h,p+2,4,2);
    F_loadres(h,p+3,6,2);
    F_loadres(h,p+4,8,wad[h].l-8);
  }else F_loadres(h,p,0,wad[h].l);
}

void *M_lock (int h) {
  if(h==-1 || h==0xFFFF) return NULL;
  if(h>=MAX_WAD) ERR_fatal("M_lock: странный номер ресурса");
  if(!resl[h]) if(!resp[h]) allocres(h);
  ++resl[h];
  return resp[h];
}

void M_unlock (void *p) {
  int h;

  if(!p) return;
  h=((int*)p)[-1];
  if(h>=MAX_WAD) ERR_fatal("M_unlock: странный номер ресурса");
  if(!resl[h]) return;
  --resl[h];
}

int M_locked (int h) {
  return (h != -1) && (h != 0xFFFF) && (resl[h] != 0);
}

int M_was_locked (int h) {
  return (h != -1) && (h != 0xFFFF) && (resp[h] != NULL);
}
