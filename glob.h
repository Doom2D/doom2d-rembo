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

// Globals

#ifndef MYGLOB
#define MYGLOB

#ifndef NULL
#define NULL 0
#endif

#define ON 1
#define OFF 0
#define TRUE 1
#define FALSE 0

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

void logo(const char *,...);
void logo_gas(int,int);

extern int gammaa;
extern int snd_card;
extern int _cpu;


#define __MAX_PATH 255
#define __MAX_DRIVE 50
#define __MAX_DIR 100
#define __MAX_FNAME 50
#define __MAX_EXT 50


#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define myrand(a) (rand()%(a))

#include <stdio.h>
void myfread(void *ptr, size_t n, size_t size, FILE *f);
size_t myfreadc(void *ptr, size_t n, size_t size, FILE *f);
void myfwrite(void *ptr, size_t n, size_t size, FILE *f);

extern int SCRW;
extern int SCRH;

#define DELAY 50

#endif
