#include "glob.h"
#include "sound.h"
#include "music.h"

short snd_vol;

short mus_vol;
char music_random;
int music_time;
int music_fade;

/* Music */

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

/* Sound */

snd_t *S_get (int id) {
  return NULL;
}

snd_t *S_load (const char name[8]) {
  return NULL;
}

void S_free (snd_t *s) {

}

void S_init (void) {

}

void S_done (void) {

}

short S_play (snd_t *s, short c, short v) {
  return 0;
}

void S_stop (short c) {
  
}

void S_volume (int v) {
  
}

void S_wait (void) {

}
