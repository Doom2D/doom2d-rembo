#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include "glob.h"

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
