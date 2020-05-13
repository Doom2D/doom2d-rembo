/* Copyright (C) 1996-1997 Aleksey Volynskov
 * Copyright (C) 2011 Rambo
 * Copyright (C) 2020 SovietPony
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

#ifndef GAME_H_INLUDED
#define GAME_H_INLUDED

#include "glob.h"
#include "view.h" // pos_t
#include "player.h" // player_t

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

void load_game (int n);
void G_start (void);
void G_init (void);
void G_act (void);
void G_respawn_player (player_t *p);

#endif /* GAME_H_INLUDED */
