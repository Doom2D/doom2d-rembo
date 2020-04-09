/*
   Copyright (C) Prikol Software 1996-1997
   Copyright (C) Aleksey Volynskov 1996-1997

   This file is part of the Doom2D:Rembo project.

   Doom2D:Rembo is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   Doom2D:Rembo is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/> or
   write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include "glob.h"

/* menu types */
#define GM_BIG 1
#define GM_SMALL 2

/* variant types */
#define GM_BUTTON 1
#define GM_SCROLLER 2
#define GM_TEXTFIELD 3
#define GM_SMALL_BUTTON 4
#define GM_TEXTFIELD_BUTTON 5

/* menu messages */
#define GM_GETINT 1
#define GM_SETINT 2
#define GM_GETSTR 3
#define GM_SETSTR 4
#define GM_SELECT 5 // buttons
#define GM_ENTER  6 // menu
#define GM_LEAVE  7 // menu
#define GM_BEGIN  8 // textfield
#define GM_END    9 // textfield
#define GM_CANCEL 10 // textfield

typedef struct new_str_msg_t {
  byte type;
  char *s;
  int maxlen;
} new_str_msg_t;

typedef struct new_int_msg_t {
  byte type;
  int i, a, b, s;
} new_int_msg_t;

typedef union new_msg_t {
  byte type;
  new_str_msg_t string;
  new_int_msg_t integer;
} new_msg_t;

typedef struct new_var_t new_var_t;
typedef struct new_menu_t new_menu_t;

struct new_var_t {
  byte type;
  char *caption;
  void *data;
  int (*handler)(new_msg_t *msg, const new_menu_t *m, void *data);
  const new_menu_t *submenu;
};

struct new_menu_t {
  byte type;
  char *title;
  void *data;
  int (*handler)(new_msg_t *msg, const new_menu_t *m, void *data);
  new_var_t entries[];
};

extern byte _warp;

//#define PCOLORN 10
//extern byte pcolortab[PCOLORN];
//extern int p1color;
//extern int p2color;

#define GM_MAX_INPUT 24
extern char ibuf[GM_MAX_INPUT];
extern byte input;
extern int icur;

extern short lastkey;

void GM_push (const new_menu_t *m);
void GM_pop (void);
void GM_popall (void);
const new_menu_t *GM_get (void);
int GM_geti (void);
int GM_send_this (const new_menu_t *m, new_msg_t *msg);
int GM_send (const new_menu_t *m, int i, new_msg_t *msg);

void GM_key (int key, int down);
void GM_input (int ch);
void G_code (void);
int GM_act (void);
void GM_init (void);

#endif /* MENU_H_INCLUDED */
