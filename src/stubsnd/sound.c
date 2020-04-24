#include "glob.h"
#include "sound.h"
#include "music.h"

/* Music */

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

const cfg_t *S_args (void) {
  return NULL;
}

const cfg_t *S_conf (void) {
  return NULL;
}

const menu_t *S_menu (void) {
  return NULL;
}

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
