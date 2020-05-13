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

#include "render.h"

const cfg_t *R_args (void) {
  return NULL;
}

const cfg_t *R_conf (void) {
  return NULL;
}

const menu_t *R_menu (void) {
  return NULL;
}

void R_init (void) {
  // stub
}

void R_draw (void) {
  // stub
}

void R_done (void) {
  // stub
}

void R_set_videomode (int w, int h, int fullscreen) {
  // stub
}

void R_switch_texture (int x, int y) {
  // stub
}

void R_get_name (int n, char s[8]) {
  s[0] = 0;
}

int R_get_special_id (int n) {
  return -1;
}

int R_get_swp (int n) {
  return 0;
}

void R_begin_load (void) {
  // stub
}

void R_load (char s[8]) {
  // stub
}

void R_end_load (void) {
  // stub
}

void R_loadsky (int sky) {
  // stub
}
