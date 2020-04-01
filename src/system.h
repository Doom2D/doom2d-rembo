#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include "glob.h"

#define SYSTEM_USE_OPENGL (1 << 0)
#define SYSTEM_USE_FULLSCREEN (1 << 1)

/* common video subsystem routines */
int Y_set_videomode (int w, int h, int flags);
void Y_get_videomode (int *w, int *h);
int Y_videomode_setted (void);
void Y_unset_videomode (void);
void Y_set_fullscreen (int yes);
int Y_get_fullscreen (void);

/* hardware specific rendering */
void Y_swap_buffers (void);

/* software specific rendering */
void Y_get_buffer (byte **buf, int *w, int *h, int *pitch);
void Y_set_vga_palette (byte *vgapal);
void Y_repaint_rect (int x, int y, int w, int h);
void Y_repaint (void);

#endif /* SYSTEM_H_INCLUDED */