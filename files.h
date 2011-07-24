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

// File operations

///#include <io.h>
//#include <fcntl.h>
#include <stdio.h>

#define MAX_WADS 20
#define MAX_WAD 2000

typedef char wadname[8];

typedef struct {
  int o,l;
  char n[8];
}wad_t;

typedef struct {
  int o,l;
  char n[8];
  int f;
}mwad_t;

void F_startup(void);
void F_addwad(char *);
void F_initwads(void);
void F_allocres(void);
//void F_preload(void);
void F_readstr(FILE*,char *,int);
void F_readstrz(FILE*,char *,int);
void F_loadres(int,void *,dword,dword);
int F_getresid(char *);
void F_getresname(char *,int);
int F_findres(char *);
int F_getsprid(char[4],int,int);
int F_getreslen(int);
void F_loadmap(char[8]);
void F_loadmus(char[8]);
void F_freemus(void);
void F_nextmus(char*);

void F_getsavnames(void);
void F_loadgame(int);
void F_savegame(int,char*);

void F_randmus(char*);//

extern char wads[MAX_WADS][__MAX_PATH];
extern int wad_num;
