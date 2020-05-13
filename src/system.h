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

#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include "glob.h"

enum { Y_BYTE = 1, Y_WORD, Y_DWORD, Y_STRING, Y_SW_ON, Y_SW_OFF, Y_FILES, Y_KEY };

typedef struct cfg_t {
  const char *cfg;
  void *p;
  byte t;
} cfg_t;

typedef struct videomode_size_t {
  int w, h, r;
} videomode_size_t;

typedef struct videomode_t {
  int n;
  videomode_size_t *modes;
} videomode_t;

/* common video subsystem routines */
void Y_get_videomode (int *w, int *h);
int Y_videomode_setted (void);
void Y_unset_videomode (void);
void Y_set_fullscreen (int yes);
int Y_get_fullscreen (void);

/* hardware specific rendering */
int Y_set_videomode_opengl (int w, int h, int fullscreen);
const videomode_t *Y_get_videomode_list_opengl (int fullscreen);
void Y_swap_buffers (void);

/* software specific rendering */
int Y_set_videomode_software (int w, int h, int fullscreen);
const videomode_t *Y_get_videomode_list_software (int fullscreen);
void Y_get_buffer (byte **buf, int *w, int *h, int *pitch);
void Y_set_vga_palette (byte *vgapal);
void Y_repaint_rect (int x, int y, int w, int h);
void Y_repaint (void);

/* input */
void Y_enable_text_input (void);
void Y_disable_text_input (void);

#endif /* SYSTEM_H_INCLUDED */
