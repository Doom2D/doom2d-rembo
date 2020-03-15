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

#include "glob.h"
#include "files.h"
#include "music.h"
#include "error.h"
#include "game.h"
#include <SDL.h>
#include <SDL_mixer.h>

short mus_vol = 50;
char music_random = ON;
int music_time = 3;
int music_fade = 5;

static Uint32 muscount;
static Mix_Music * muslo;
static int musdisabled = 1;
static int volsetcount = 0;

void S_initmusic (void) {
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            fprintf(stderr, "\nUnable to initialize audio:  %s\n", SDL_GetError());
            musdisabled=1;
            return;
        }

        if (Mix_OpenAudio(22050, AUDIO_S16, 1, 1000) < 0) {
            fprintf(stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
            musdisabled=1;
            return;
        }
    }

    muslo=NULL;

    muscount=0;

    musdisabled = (mus_vol==0);
    
   S_volumemusic(mus_vol);
}

void S_donemusic (void) {
    if (SDL_WasInit(SDL_INIT_AUDIO)) {
        F_freemus();
        Mix_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
}

void S_startmusic (int time) {
    if (musdisabled) return;
    Mix_PlayMusic(muslo, -1);
    Mix_VolumeMusic(mus_vol);
    muscount=time*60*1000/DELAY;
}

void S_stopmusic (void) {
    if (musdisabled) return;
    Mix_HaltMusic();
    muscount = 0;
}

void S_volumemusic (int v) {
    if (musdisabled) return;
    mus_vol = v;
    if (mus_vol>128) mus_vol=128;
    if (mus_vol<0) mus_vol=0;
    if (mus_vol==0 && Mix_PlayingMusic()) {
        S_stopmusic();
    }
    else if (mus_vol>0 && !Mix_PlayingMusic()) {
        S_startmusic(music_time);
    }
    else {
        Mix_VolumeMusic(v);
    }
}

static struct {
    Uint8 ascii;
    Uint8 asciilc;
    char *ch;
} atrans[] = {
    {0x80, 0xA0, "A"},//А
    {0x81, 0xA1, "B"},//Б
    {0x82, 0xA2, "V"},//В
    {0x83, 0xA3, "G"},//Г
    {0x84, 0xA4, "D"},//Д
    {0x85, 0xA5, "E"},//Е
    {0x86, 0xA6, "ZH"},//Ж
    {0x87, 0xA7, "Z"},//З
    {0x88, 0xA8, "I"},//И
    {0x89, 0xA9, "J"},//Й
    {0x8A, 0xAA, "K"},//К
    {0x8B, 0xAB, "L"},//Л
    {0x8C, 0xAC, "M"},//М
    {0x8D, 0xAD, "N"},//Н
    {0x8E, 0xAE, "O"},//О
    {0x8F, 0xAF, "P"},//П
    {0x90, 0xE0, "R"},//Р
    {0x91, 0xE1, "S"},//С
    {0x92, 0xE2, "T"},//Т
    {0x93, 0xE3, "U"},//У
    {0x94, 0xE4, "F"},//Ф
    {0x95, 0xE5, "H"},//Х
    {0x96, 0xE6, "C"},//Ц
    {0x97, 0xE7, "CH"},//Ч
    {0x98, 0xE8, "SH"},//Ш
    {0x99, 0xE9, "SCH"},//Щ
    {0x9A, 0xEA, "X"},//Ъ
    {0x9B, 0xEB, "Y"},//Ы
    {0x9C, 0xEC, "J"},//Ь
    {0x9D, 0xED, "E"},//Э
    {0x9E, 0xEE, "JU"},//Ю
    {0x9F, 0xEF, "JA"},//Я
    {0}
};

static char *get_trans_char (Uint8 c)
{
    int i = 0;
    while (atrans[i].ascii) {

        if (atrans[i].ascii == c || atrans[i].asciilc == c) {
            return atrans[i].ch;
        }
        i++;
    }
    return NULL;
}

static void trans_ascii_str (char *dest, char *src)
{
    char *p = dest;
    int i;
    for (i=0; i<strlen(src); i++) {
        char *ch = get_trans_char(src[i]);
        if (ch) {
            strcpy(p,ch);
            p+=strlen(ch);
        }
        else {
            strncpy(p,&src[i],1);
            p++;
        }
    }
    *p='\0';
}

void F_loadmus (char n[8]) {
    if (musdisabled) return;
    char f[50];
    char name[50];
#ifndef WIN32
    strcpy(f, "/usr/share/doom2d-rembo/music/");
#else
    strcpy(f, "music\\");
#endif
    int l = strlen(f);
    strncpy(&f[l], n, 8);
    f[l+8]='\0';
    trans_ascii_str(name, f);
    muslo = Mix_LoadMUS(name);
    if (muslo == NULL)
    {
#ifndef WIN32
        strcpy(f, "music/");
        int l = strlen(f);
        strncpy(&f[l], n, 8);
        f[l+8]='\0';
        trans_ascii_str(name, f);
        muslo = Mix_LoadMUS(name);
#endif
    }
    if (!muslo) logo("Music not found '%s'\n", name);

}

void F_freemus (void) {
    if (musdisabled) return;
    if (muslo) {
        Mix_HaltMusic();
        Mix_FreeMusic(muslo);
    }
    muslo = NULL;
}

void S_updatemusic (void) {
    if (musdisabled) return;
    
    //періодично встановлюю гучність музикі, так як вона сама підвищується до максимуму через певний час
    volsetcount++;
    if (volsetcount%(5*1000/DELAY)==0) {
        S_volumemusic(mus_vol);
    }

    if (muscount>0) {
        if (muscount < music_fade*1100/DELAY) {
            Mix_FadeOutMusic(music_fade*1000);
        }
        muscount--;
        if (muscount==0) {
            if (music_random) F_randmus(g_music);
            else F_nextmus(g_music);
            F_freemus();
            F_loadmus(g_music);
            S_startmusic(music_time);
        }
    }
}
