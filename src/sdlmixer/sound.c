#include "glob.h"
#include "sound.h"
#include "music.h"
#include "misc.h" // int2host
#include "memory.h" // M_lock M_unlock
#include "files.h" // F_findres
#include "error.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <assert.h>

#define TAG_MIX1 0x4d495831

#pragma pack(1)
struct dmi {
  Uint32 len;    // length [bytes]
  Uint32 rate;   // freq [Hz]
  Uint32 lstart; // loop start offset [bytes]
  Uint32 llen;   // loop length [bytes]
  Uint8 data[];  // sound data
};
#pragma pack()

struct sound {
  snd_t base;
  Mix_Chunk *c;
};

short snd_vol; // public 0..128

static int devfreq = MIX_DEFAULT_FREQUENCY;
static Uint32 devformat = AUDIO_S16SYS; // MIX_DEFAULT_FORMAT
static int devchannels = 1; // MIX_DEFAULT_CHANNELS;
static int devchunksize = 1024;
static int devchunkchannels = 8;
static int devinit;

/* music */

short mus_vol;
char music_random;
int music_time;
int music_fade;

void S_initmusic (void) {

}

void S_donemusic (void) {

}

void S_startmusic (int time) {

}

void S_stopmusic (void) {

}

void S_volumemusic (int v) {

}

void F_loadmus (char n[8]) {

}

void F_freemus (void) {

}

void S_updatemusic (void) {

}

/* sound */

void S_init (void) {
  assert(devinit == 0);
  logo("S_init: initialize sound\n");
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0) {
    if (Mix_OpenAudio(devfreq, devformat, devchannels, devchunksize) == 0) {
      Mix_AllocateChannels(devchunkchannels);
      devinit = 1;
    } else {
      logo("S_init: Mix_OpenAudio: %s\n", Mix_GetError());
    }
  } else {
    logo("S_init: SDL_InitSubSytem: %s\n", SDL_GetError());
  }
}

static Mix_Chunk *convert_this (int rate, const Uint8 *buf, int len) {
  SDL_AudioCVT cvt;
  Mix_Chunk *c = NULL;
  if (SDL_BuildAudioCVT(&cvt, AUDIO_S8, 1, rate, devformat, devchannels, devfreq) != -1) {
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

// TODO load raw sounds with voices

snd_t *S_get (int id) {
  if (!devinit) {
    return NULL;
  }
  struct dmi *snd = M_lock(id);
  struct sound *res = NULL;
  if (snd != NULL) {
    int rlen = F_getreslen(id);
    if (rlen > 16) {
      Uint32 len = int2host(snd->len);
      Uint32 rate = int2host(snd->rate);
      Uint32 lstart = int2host(snd->lstart);
      Uint32 llen = int2host(snd->llen);
      if (len <= rlen - 8 && lstart + llen <= rlen - 16) {
        Mix_Chunk *c = convert_this(rate, snd->data, len);
        if (c != NULL) {
          res = malloc(sizeof(struct sound));
          if (res != NULL) {
            res->base.tag = TAG_MIX1;
            res->c = c;
            // TODO loops
          } else {
            free(c);
          }
        }
      } else {
        logo("S_get(%i): invalid header {len=%u:rate=%u:lstart=%u:llen=%u:rlen=%i}\n", id, len, rate, lstart, llen, rlen);
      }
    } else {
      logo("S_load(%i): too short\n", id);
    }
    M_unlock(snd);
  } else {
    logo("S_load(%i): not found\n", id);
  }
  return (snd_t*)res;
}

snd_t *S_load (const char name[8]) {
  int id = F_findres(name);
  return S_get(id);
}

short S_play (snd_t *s, short c, short v) {
  assert(c >= 0 && c <= 8);
  assert(v >= 0 && v <= 255);
  short channel = 0;
  if (devinit) {
    if (s) {
      struct sound *snd = (struct sound *)s;
      assert(snd->base.tag == TAG_MIX1);
      // TODO care about global volume level
      snd->c->volume = v * MIX_MAX_VOLUME / 255;
      channel = Mix_PlayChannel(c <= 0 ? -1 : c - 1, snd->c, 0);
      channel = channel == -1 ? 0 : channel + 1;
    }
  }
  return channel;
}

void S_stop (short c) {
  assert(c >= 0 && c <= 8);
  if (devinit) {
    if (c > 0) {
      Mix_HaltChannel(c - 1);
    }
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
