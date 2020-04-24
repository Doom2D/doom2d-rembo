#ifndef MUSIC_H_INCLUDED
#define MUSIC_H_INCLUDED

#include "menu.h"
#include "system.h"

const cfg_t *MUS_args (void);
const cfg_t *MUS_conf (void);
const menu_t *MUS_menu (void);

void MUS_init (void);
void MUS_done (void);
void MUS_start (int time);
void MUS_stop (void);
void MUS_volume (int v);
void MUS_load (char n[8]);
void MUS_free (void);
void MUS_update (void);

#endif /* MUSIC_H_INCLUDED */
