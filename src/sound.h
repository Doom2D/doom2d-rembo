/*
   Управление звуком и музыкой для DOS4GW
   Версия 1.0
   Copyright (C) Алексей Волынсков, 1996

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

#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED

typedef struct {
  int tag;
} snd_t;

// Sound volume 0..128
extern short snd_vol;

// Get sound handle for resource <id>
snd_t *S_get (int id);

// Get sound handle for resource with name
snd_t *S_load (const char name[8]);

// Initialize sound subsystem
void S_init (void);

// Deinitialize sound subsystem
void S_done (void);

// Play sound <s> on channel <c> with volume <v>.
// <s> = sound handle (NULL is ignored)
// <c> = 1..8 or 0 for any free
// <v> = 0..255
// return used channel or zero on error
short S_play (snd_t *s, short c, short v);

// Stop sound on channel <c>
// <c> = 0..8 (0 ignored)
void S_stop (short c);

// Set sound volume to <v>, also affects variable snd_vol
// <v> = 0..128
void S_volume (int v);

// Wait before all sounds end playing
void S_wait (void);

#endif /* SOUND_H_INCLUDED */
