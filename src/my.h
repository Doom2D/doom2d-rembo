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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

void mysplitpath (const char *path, char *drv, char *dir, char *name, char *ext);

size_t myfreadc(void *ptr, size_t size, size_t n, FILE *f);

void myfread (void *ptr, size_t size, size_t n, FILE *f);
void myfread8 (uint8_t *x, FILE *f);
void myfread16 (uint16_t *x, FILE *f);
void myfread32 (uint32_t *x, FILE *f);

void myfwrite(void *ptr, size_t size, size_t n, FILE *f);
void myfwrite8 (uint8_t x, FILE *f);
void myfwrite16 (uint16_t x, FILE *f);
void myfwrite32 (uint32_t x, FILE *f);

void myrandomize (void);
int fexists (char *filename);

#ifdef __cplusplus
}
#endif
