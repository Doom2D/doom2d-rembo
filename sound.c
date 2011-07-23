#include "glob.h"
#include "files.h"
#include "sound.h"
#include <SDL.h>
#include <SDL_mixer.h>

#define NUM_CHANNELS 16
#define NUM_CHUNKS 300

short snd_vol = 50;

int snddisabled = 1;
int ggg;
struct {
    snd_t *s;
    Mix_Chunk *c;
} chunks[NUM_CHUNKS];

void S_init(void)
{
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            fprintf(stderr, "\nUnable to initialize audio:  %s\n", SDL_GetError());
            snddisabled=1;
            return;
        }

        if (Mix_OpenAudio(22050, AUDIO_S16, 1, 1000) < 0) {
            fprintf(stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
            snddisabled=1;
            return;
        }

        if (Mix_AllocateChannels(NUM_CHANNELS)!=NUM_CHANNELS) {
            fprintf(stderr, "Error allocation channels: %s\n", Mix_GetError());
            snddisabled=1;
            return;
        }
    }

    int i;
    for (i=0; i<NUM_CHUNKS; i++) {
        chunks[i].s = NULL;
        chunks[i].c = NULL;
    }

    snddisabled = (snd_vol==0);

   S_volume(snd_vol);
}

void S_done(void)
{
    free_chunks();
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

Mix_Chunk * get_chunk(snd_t *s, int r, int v)
{
    int i, fi = -1;
    for(i=0; i<NUM_CHUNKS; i++) {
        if (chunks[i].s == s) return chunks[i].c;
        if (chunks[i].s == NULL && fi==-1) fi = i;
    }

    if (fi==-1) return NULL;

  Uint8 *data = (Uint8*)s+sizeof(snd_t);
  Uint32 dlen = s->len;
  SDL_AudioCVT cvt;
  SDL_BuildAudioCVT(&cvt, AUDIO_S8, 1, s->rate, AUDIO_S16, 1, 22050);
  if (!(cvt.buf = malloc(dlen*cvt.len_mult))) ERR_fatal("Out of memory\n");;
  memcpy(cvt.buf, data, dlen);
  cvt.len = dlen;
  SDL_ConvertAudio(&cvt);

    Mix_Chunk *chunk;
    if (!(chunk = malloc(sizeof(Mix_Chunk)))) ERR_fatal("Out of memory\n");;
    chunk->abuf=cvt.buf;
    chunk->alen=cvt.len_cvt;
    chunk->allocated=0;
    chunk->volume=(float)v/255*SDL_MIX_MAXVOLUME;

    chunks[fi].s = s;
    chunks[fi].c = chunk;

    return chunk;
}

void free_chunks()
{
    if (snddisabled) return;
    Mix_HaltChannel(-1);
    int i;
    for (i=0; i<NUM_CHUNKS; i++) {
        if (chunks[i].c) {
            free(chunks[i].c->abuf);
            free(chunks[i].c);
            chunks[i].c = NULL;
            chunks[i].s = NULL;
        }
    }
}

short S_play(snd_t *s,short c,unsigned r,short v)
{
    if (snddisabled) return 0;
    Mix_Chunk *chunk = get_chunk(s,r,v);
    if (chunk==NULL) return 0;
    return Mix_PlayChannel(c, chunk, 0);
}

void S_stop(short c)
{
    Mix_HaltChannel(c);
}

void S_volume(int v)
{
    if (snddisabled) return;
    snd_vol=v;
    if (snd_vol>128) snd_vol=128;
    if (snd_vol<0) snd_vol=0;
    Mix_Volume(-1, snd_vol);
}

void S_wait()
{
    if (snddisabled) return;
    while (Mix_Playing(-1)) {
        SDL_Delay(10);
    }
}
