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
#define GM_QUERY  11 // menu
#define GM_GETTITLE 12 // menu
#define GM_GETENTRY 13 // entry
#define GM_GETCAPTION 14 // entry
#define GM_UP 15
#define GM_DOWN 16
#define GM_KEY 17

typedef struct menu_str_msg_t {
  byte type;
  char *s;
  int maxlen;
} menu_str_msg_t;

typedef struct menu_int_msg_t {
  byte type;
  int i, a, b, s;
} menu_int_msg_t;

typedef union menu_msg_t {
  byte type;
  menu_str_msg_t string;
  menu_int_msg_t integer;
} menu_msg_t;

typedef struct menu_t menu_t;
struct menu_t {
  int (*handler)(menu_msg_t *msg, const menu_t *m, int i);
};

typedef struct simple_menu_t {
  byte type;
  char *title;
  char *say;
  struct simple_entry_t {
    char *caption;
    const menu_t *submenu;
  } entries[];
} simple_menu_t;

extern byte _warp;

#define GM_MAX_INPUT 24
extern char ibuf[GM_MAX_INPUT];
extern byte input;
extern int icur;
extern int imax;

extern short lastkey;

#define GM_INIT_STRING(msg, str) GM_init_str(msg, str, sizeof str);
#define GM_CYCLE(i, m, n) ((i) < (m) ? (n) : (((i) > (n)) ? (m) : (i)))
int GM_init_int0 (menu_msg_t *msg, int i, int a, int b, int s);
int GM_init_int (menu_msg_t *msg, int i, int a, int b, int s);
int GM_init_str (menu_msg_t *msg, char *str, int maxlen);
int basic_menu_handler (menu_msg_t *msg, byte type, char *title, char *say, int n, int *cur);
int simple_menu_handler (menu_msg_t *msg, int i, int n, const simple_menu_t *m, int *cur);

int GM_push (const menu_t *m);
int GM_pop (void);
int GM_popall (void);
const menu_t *GM_get (void);
int GM_send_this (const menu_t *m, menu_msg_t *msg);
int GM_send (const menu_t *m, int i, menu_msg_t *msg);

void GM_key (int key, int down);
void GM_input (int ch);
void G_code (void);
int GM_act (void);
void GM_init (void);

#endif /* MENU_H_INCLUDED */
