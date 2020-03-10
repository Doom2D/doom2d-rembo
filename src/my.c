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

#include <SDL.h>
#include "glob.h"
#include "error.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

void mysplitpath(const char* path, char* drv, char* dir, char* name, char* ext)
{
     const char* end; /* end of processed string */
     const char* p;   /* search pointer */
     const char* s;   /* copy pointer */

     /* extract drive name */
     if (path[0] && path[1]==':') {
         if (drv) {
             *drv++ = *path++;
             *drv++ = *path++;
             *drv = '\0';
         }
     } else if (drv)
         *drv = '\0';

     /* search for end of string or stream separator */
     for(end=path; *end && *end!=':'; )
         end++;

     /* search for begin of file extension */
     for(p=end; p>path && *--p!='\\' && *p!='/'; )
         if (*p == '.') {
             end = p;
             break;
         }

     if (ext)
         for(s=end; (*ext=*s++); )
             ext++;

     /* search for end of directory name */
     for(p=end; p>path; )
         if (*--p=='\\' || *p=='/') {
             p++;
             break;
         }

     if (name) {
         for(s=p; s<end; )
             *name++ = *s++;

         *name = '\0';
     }

     if (dir) {
         for(s=path; s<p; )
             *dir++ = *s++;

         *dir = '\0';
     }
}

size_t myfreadc (void *ptr, size_t size, size_t n, FILE *f) {
  return fread(ptr, size, n, f);
}

void myfread (void *ptr, size_t size, size_t n, FILE *f) {
  if (myfreadc(ptr, size, n, f) != n) {
    ERR_fatal("File reading error\n");
  }
}

void myfread8 (uint8_t *x, FILE *f) {
  myfread(x, 1, 1, f);
}

void myfread16 (uint16_t *x, FILE *f) {
  myfread(x, 2, 1, f);
  *x = short2host(*x);
}

void myfread32 (uint32_t *x, FILE *f) {
  myfread(x, 4, 1, f);
  *x = int2host(*x);
}

void myfwrite (void *ptr, size_t size, size_t n, FILE *f) {
  assert(fwrite(ptr, size, n, f) == n);
}

void myfwrite8 (uint8_t x, FILE *f) {
  myfwrite(&x, 1, 1, f);
}

void myfwrite16 (uint16_t x, FILE *f) {
  x = short2host(x);
  myfwrite(&x, 2, 1, f);
}

void myfwrite32 (uint32_t x, FILE *f) {
  x = int2host(x);
  myfwrite(&x, 4, 1, f);
}

void myrandomize(void)
{
    srand(SDL_GetTicks());
}

int fexists(char * filename)
{
    FILE *f;
    if ((f = fopen(filename, "r")))
    {
        fclose(f);
        return 1;
    }
    return 0;
}