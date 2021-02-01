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
#include "error.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

void mysplitpath(const char* path, char* drv, char* dir, char* name, char* ext) {
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
#if 1
  myfreadc(ptr, size, n, f);
#else
  if (myfreadc(ptr, size, n, f) != n) {
    ERR_fatal("File reading error (readed %u, required %u)\n", m, n);
  }
#endif
}

int8_t myfread8 (FILE *f) {
  int8_t x;
  myfread(&x, 1, 1, f);
  return x;
}

int16_t myfread16 (FILE *f) {
  int16_t x;
  myfread(&x, 2, 1, f);
  return short2host(x);
}

int32_t myfread32 (FILE *f) {
  int32_t x;
  myfread(&x, 4, 1, f);
  return int2host(x);
}

void myfwrite (void *ptr, size_t size, size_t n, FILE *f) {
  assert(fwrite(ptr, size, n, f) == n);
}

void myfwrite8 (int8_t x, FILE *f) {
  myfwrite(&x, 1, 1, f);
}

void myfwrite16 (int16_t x, FILE *f) {
  x = short2host(x);
  myfwrite(&x, 2, 1, f);
}

void myfwrite32 (int32_t x, FILE *f) {
  x = int2host(x);
  myfwrite(&x, 4, 1, f);
}

int fexists (char *filename) {
    FILE *f;
    if ((f = fopen(filename, "r")))
    {
        fclose(f);
        return 1;
    }
    return 0;
}
