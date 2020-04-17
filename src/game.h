#ifndef GAME_H_INLUDED
#define GAME_H_INLUDED

#include "glob.h"
#include "view.h" // pos_t
#include "player.h" // player_t
#include <stdio.h> // FILE

extern byte transdraw;
extern byte _2pl;
extern byte g_dm;
extern byte g_st;
extern byte g_exit;
extern byte g_map;
extern char g_music[8];
extern dword g_time;
extern int dm_pnum;
extern int dm_pl1p;
extern int dm_pl2p;
extern pos_t dm_pos[100];
extern byte cheat;

extern int lt_time;
extern int lt_type;
extern int lt_side;
extern int lt_ypos;

extern int g_trans;

void G_savegame (FILE* h);
void G_loadgame (FILE* h);
int G_load (FILE *h);
void load_game (int n);
void G_start (void);
void G_init (void);
void G_act (void);
void G_respawn_player (player_t *p);

#endif /* GAME_H_INLUDED */
