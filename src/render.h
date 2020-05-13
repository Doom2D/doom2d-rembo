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

#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include "system.h"
#include "menu.h"

const cfg_t *R_args (void);
const cfg_t *R_conf (void);
const menu_t *R_menu (void);

void R_init (void);
void R_draw (void);
void R_done (void);

void R_set_videomode (int w, int h, int fullscreen);

void R_switch_texture (int x, int y);

void R_get_name (int n, char s[8]);
int R_get_special_id (int n);
int R_get_swp (int n);
void R_begin_load (void);
void R_load (char s[8]);
void R_end_load (void);
void R_loadsky (int sky);

#endif /* RENDER_H_INCLUDED */
