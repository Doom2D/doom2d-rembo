/* Copyright (C) 2020 SovietPony
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
#include "sound.h"
#include "music.h"
#include "memory.h" // M_lock M_unlock
#include "files.h" // F_findres
#include "error.h"

#include "common/endianness.h"

#include "SDL.h"
#include "SDL_mixer.h"
#include <assert.h>
#include <string.h>

#define TAG_MIX1 0x4d495831

#pragma pack(1)
typedef struct dmi {
  word len;    // length [bytes]
  word rate;   // freq [Hz]
  word lstart; // loop start offset [bytes]
  word llen;   // loop length [bytes]
  byte data[];  // sound data
} dmi;
#pragma pack()

typedef struct sdlmixer_snd {
  snd_t base;
  Mix_Chunk *c;
} sdlmixer_snd;

static short snd_vol;

static int devfreq = MIX_DEFAULT_FREQUENCY;
static Uint32 devformat = AUDIO_S16SYS; // MIX_DEFAULT_FORMAT
static int devchannels = 1; // MIX_DEFAULT_CHANNELS;
static int devchunksize = 1024;
static int devchunkchannels = 8;
static int devinit;

/* music */

const cfg_t *MUS_args (void) {
  return NULL;
}

const cfg_t *MUS_conf (void) {
  return NULL;
}

const menu_t *MUS_menu (void) {
  return NULL;
}

void MUS_init (void) {

}

void MUS_done (void) {

}

void MUS_start (int time) {

}

void MUS_stop (void) {

}

void MUS_volume (int v) {

}

void MUS_load (char n[8]) {

}

void MUS_free (void) {

}

void MUS_update (void) {

}

/* Sound */

static int sound_menu_handler (menu_msg_t *msg, const menu_t *m, int i) {
  static int cur;
  enum { VOLUME, __NUM__ };
  static const simple_menu_t sm = {
    GM_BIG, "Sound", NULL,
    {
      { "Volume", NULL },
    }
  };
  if (i == VOLUME) {
    switch (msg->type) {
      case GM_GETENTRY: return GM_init_int0(msg, GM_SCROLLER, 0, 0, 0);
      case GM_GETINT: return GM_init_int(msg, snd_vol, 0, 128, 8);
      case GM_SETINT: S_volume(msg->integer.i); return 1;
    }
  }
  return simple_menu_handler(msg, i, __NUM__, &sm, &cur);
}

const menu_t *S_menu (void) {
  static const menu_t m = { sound_menu_handler };
  return &m;
}

const cfg_t *S_args (void) {
  static const cfg_t args[] = {
    { "sndvol", &snd_vol, Y_WORD },
    { NULL, NULL, 0 }
  };
  return args;
}

const cfg_t *S_conf (void) {
  static const cfg_t conf[] = {
    { "sound_volume", &snd_vol, Y_WORD },
    { NULL, NULL, 0 }
  };
  return conf;
}

void S_init (void) {
  assert(devinit == 0);
  logo("S_init: initialize sound\n");
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0) {
    if (Mix_OpenAudio(devfreq, devformat, devchannels, devchunksize) == 0) {
      Mix_AllocateChannels(devchunkchannels);
      devinit = 1;
    } else {
      logo("S_init: Mix_OpenAudio: %s\n", Mix_GetError());
      SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
  } else {
    logo("S_init: SDL_InitSubSytem: %s\n", SDL_GetError());
  }
}

static Mix_Chunk *convert_this (int rate, int sign, const Uint8 *buf, int len) {
  SDL_AudioCVT cvt;
  Mix_Chunk *c = NULL;
  if (SDL_BuildAudioCVT(&cvt, sign ? AUDIO_S8 : AUDIO_U8, 1, rate, devformat, devchannels, devfreq) != -1) {
    int maxlen = len * cvt.len_mult;
    Uint8 *maxbuf = malloc(maxlen);
    if (maxbuf != NULL) {
      memcpy(maxbuf, buf, len);
      cvt.buf = maxbuf;
      cvt.len = len;
      if (SDL_ConvertAudio(&cvt) == 0) {
        c = malloc(sizeof(Mix_Chunk));
        if (c != NULL) {
          c->allocated = 0;
          c->abuf = maxbuf;
          c->alen = len * cvt.len_ratio;
          c->volume = MIX_MAX_VOLUME;
        } else {
          free(maxbuf);
        }
      } else {
        free(maxbuf);
      }
    }
  }
  return c;
}

static sdlmixer_snd *new_sdlmixer_snd (const void *data, dword len, dword rate, dword lstart, dword llen, int sign) {
  Mix_Chunk *c = NULL;
  sdlmixer_snd *snd = NULL;
  c = convert_this(rate, sign, data, len);
  if (c != NULL) {
    snd = malloc(sizeof(sdlmixer_snd));
    if (snd != NULL) {
      snd->base.tag = TAG_MIX1;
      snd->c = c;
    } else {
      free(c->abuf);
      free(c);
    }
  }
  return snd;
}

snd_t *S_get (int id) {
  void *handle;
  sdlmixer_snd *snd = NULL;
  if (devinit) {
    handle = M_lock(id);
    if (handle != NULL) {
      void *data = handle;
      word len = F_getreslen(id);
      word rate = 11025;
      word lstart = 0;
      word llen = 0;
      int sign = 0;
      if (len > 8) {
        dmi *hdr = handle;
        word hdr_len = short2host(hdr->len);
        word hdr_rate = short2host(hdr->rate);
        word hdr_lstart = short2host(hdr->lstart);
        word hdr_llen = short2host(hdr->llen);
        if (hdr_len <= len - 4 && hdr_lstart + hdr_llen <= len - 8) {
          data = hdr->data;
          len = hdr_len;
          rate = hdr_rate;
          lstart = hdr_lstart;
          llen = hdr_llen;
          sign = 1;
        }
      }
      snd = new_sdlmixer_snd(data, len, rate, lstart, llen, sign);
      M_unlock(handle);
    }
  }
  return (snd_t*)snd;
}

snd_t *S_load (const char name[8]) {
  int id = F_findres(name);
  return S_get(id);
}

void S_free (snd_t *s) {
  int i;
  sdlmixer_snd *snd = (sdlmixer_snd*)s;
  if (snd != NULL) {
    assert(snd->base.tag == TAG_MIX1);
    if (devinit) {
      for (i = 0; i < devchunkchannels; i++) {
        if (Mix_GetChunk(i) == snd->c) {
          Mix_HaltChannel(i);
        }
      }
    }
    free(snd->c->abuf);
    free(snd->c);
    free(snd);
  }
}

short S_play (snd_t *s, short c, short v) {
  short channel = 0;
  sdlmixer_snd *snd = (sdlmixer_snd*)s;
  assert(c >= 0 && c <= 8);
  assert(v >= 0 && v <= 255);
  if (devinit && snd != NULL) {
    assert(snd->base.tag == TAG_MIX1);
    // TODO care about global volume level
    snd->c->volume = v * MIX_MAX_VOLUME / 255;
    channel = Mix_PlayChannel(c <= 0 ? -1 : c - 1, snd->c, 0);
    channel = channel == -1 ? 0 : channel + 1;
  }
  return channel;
}

void S_stop (short c) {
  assert(c >= 0 && c <= 8);
  if (devinit && c > 0) {
    Mix_HaltChannel(c - 1);
  }
}

void S_volume (int v) {
  snd_vol = min(max(v, 0), 128);
  if (devinit) {
    // TODO change relativelly for every channel
    Mix_Volume(-1, v * MIX_MAX_VOLUME / 128);
  }
}

void S_wait (void) {
  if (devinit) {
    while (Mix_Playing(-1) > 0) {
      SDL_Delay(10);
    }
  }
}

void S_done (void) {
  if (devinit) {
    // TODO free memory
    Mix_AllocateChannels(0);
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }
}
