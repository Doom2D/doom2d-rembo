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
