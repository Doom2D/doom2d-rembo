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

#ifdef __cplusplus
extern "C" {
#endif

// заголовок инструмента (DMI)
#pragma pack(1)
typedef struct{
  unsigned int   len,		// длина в байтах
                 rate,		// частота в Гц.
                 lstart,	// начало повтора в байтах от начала данных
                 llen;		// длина повтора в байтах
}snd_t;
#pragma pack()

void S_init(void);

void S_done(void);

// проиграть звук s на канале c (1-8), частоте r и громкости v (0-255)
// возвращает номер канала, на котором играется звук
// если c==0, то звук попадет в любой свободный канал
// r - это относительная частота (обычно 1024)
short S_play(snd_t *s,short c,unsigned r,short v);

// остановить звук на канале c (1-8)
void S_stop(short c);

void S_startmusic(int);

void S_stopmusic(void);

void S_updatemusic(void);

// громкость звука и музыки (0-128)
extern short snd_vol,mus_vol;

void S_volumemusic(int v);
void S_volume(int v);
void free_chunks();
void S_wait();

extern char music_random;
extern int music_time;
extern int music_fade;

void F_freemus(void);
void S_initmusic(void);
void S_donemusic(void);

#ifdef __cplusplus
}
#endif
