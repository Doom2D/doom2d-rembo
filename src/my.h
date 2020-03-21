/*
   Copyright (C) Prikol Software 1996-1997
   Copyright (C) Aleksey Volynskov 1996-1997

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

#ifndef MY_H_INCLUDED
#define MY_H_INCLUDED

#include <stdint.h> // int16_t int32_t
#include <stdio.h> // FILE

void mysplitpath(const char* path, char* drv, char* dir, char* name, char* ext);

size_t myfreadc (void *ptr, size_t size, size_t n, FILE *f);

void myfread (void *ptr, size_t size, size_t n, FILE *f);
int8_t myfread8 (FILE *f);
int16_t myfread16 (FILE *f);
int32_t myfread32 (FILE *f);

void myfwrite (void *ptr, size_t size, size_t n, FILE *f);
void myfwrite8 (int8_t x, FILE *f);
void myfwrite16 (int16_t x, FILE *f);
void myfwrite32 (int32_t x, FILE *f);

int fexists (char *filename);

#endif /* MY_H_INCLUDED */
