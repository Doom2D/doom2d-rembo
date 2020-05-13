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

// Globals

#include <stdlib.h>

#ifndef GLOB_H_INCLUDED
#define GLOB_H_INCLUDED

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

#define __MAX_PATH 255
#define __MAX_DRIVE 50
#define __MAX_DIR 100
#define __MAX_FNAME 50
#define __MAX_EXT 50

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define myrand(a) (rand()%(a))

#define DELAY 50

#endif /* GLOB_H_INCLUDED */
