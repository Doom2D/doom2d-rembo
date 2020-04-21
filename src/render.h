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
