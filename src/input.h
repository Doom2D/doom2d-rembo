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

#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

enum {
  KEY_UNKNOWN,
  KEY_0,
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,
  KEY_A,
  KEY_B,
  KEY_C,
  KEY_D,
  KEY_E,
  KEY_F,
  KEY_G,
  KEY_H,
  KEY_I,
  KEY_J,
  KEY_K,
  KEY_L,
  KEY_M,
  KEY_N,
  KEY_O,
  KEY_P,
  KEY_Q,
  KEY_R,
  KEY_S,
  KEY_T,
  KEY_U,
  KEY_V,
  KEY_W,
  KEY_X,
  KEY_Y,
  KEY_Z,
  KEY_RETURN,
  KEY_ESCAPE,
  KEY_BACKSPACE,
  KEY_TAB,
  KEY_SPACE,
  KEY_MINUS,
  KEY_EQUALS,
  KEY_LEFTBRACKET,
  KEY_RIGHTBRACKET,
  KEY_BACKSLASH,
  KEY_SEMICOLON,
  KEY_APOSTROPHE,
  KEY_GRAVE,
  KEY_COMMA,
  KEY_PERIOD,
  KEY_SLASH,
  KEY_CAPSLOCK,
  KEY_F1,
  KEY_F2,
  KEY_F3,
  KEY_F4,
  KEY_F5,
  KEY_F6,
  KEY_F7,
  KEY_F8,
  KEY_F9,
  KEY_F10,
  KEY_F11,
  KEY_F12,
  KEY_PRINTSCREEN,
  KEY_SCROLLLOCK,
  KEY_PAUSE,
  KEY_INSERT,
  KEY_HOME,
  KEY_PAGEUP,
  KEY_DELETE,
  KEY_END,
  KEY_PAGEDOWN,
  KEY_RIGHT,
  KEY_LEFT,
  KEY_DOWN,
  KEY_UP,
  KEY_NUMLOCK,
  KEY_KP_DIVIDE,
  KEY_KP_MULTIPLY,
  KEY_KP_MINUS,
  KEY_KP_PLUS,
  KEY_KP_ENTER,
  KEY_KP_0,
  KEY_KP_1,
  KEY_KP_2,
  KEY_KP_3,
  KEY_KP_4,
  KEY_KP_5,
  KEY_KP_6,
  KEY_KP_7,
  KEY_KP_8,
  KEY_KP_9,
  KEY_KP_PERIOD,
  KEY_SYSREQ,
  KEY_LCTRL,
  KEY_LSHIFT,
  KEY_LALT,
  KEY_LSUPER,
  KEY_RCTRL,
  KEY_RSHIFT,
  KEY_RALT,
  KEY_RSUPER,
  KEY__LAST = KEY_RSUPER
};

const char *I_key_to_string (int key);
int I_string_to_key (const char *str);
int I_pressed (int key);
void I_press (int key, int down);

#endif /* INPUT_H_INCLUDED */
