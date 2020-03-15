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

// заголовок инструмента (DMI)
#pragma pack(1)
typedef struct {
  unsigned int len;    // длина в байтах
  unsigned int rate;   // частота в Гц.
  unsigned int lstart; // начало повтора в байтах от начала данных
  unsigned int llen;   // длина повтора в байтах
} snd_t;
#pragma pack()

// громкость звука и музыки (0-128)
extern short snd_vol;

void S_init (void);
void S_done (void);

// проиграть звук s на канале c (1-8), частоте r и громкости v (0-255)
// возвращает номер канала, на котором играется звук
// если c==0, то звук попадет в любой свободный канал
// r - это относительная частота (обычно 1024)
short S_play (snd_t *s, short c, unsigned r, short v);

// остановить звук на канале c (1-8)
void S_stop (short c);

void S_volume (int v);
void free_chunks (void);
void S_wait (void);

#endif /* SOUND_H_INCLUDED */
