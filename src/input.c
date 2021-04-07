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

#include "input.h"
#include "common/cp866.h"

#include <assert.h>

static char keystate[KEY__LAST + 1];

const char *I_key_to_string (int key) {
  switch (key) {
    case KEY_0: return "0";
    case KEY_1: return "1";
    case KEY_2: return "2";
    case KEY_3: return "3";
    case KEY_4: return "4";
    case KEY_5: return "5";
    case KEY_6: return "6";
    case KEY_7: return "7";
    case KEY_8: return "8";
    case KEY_9: return "9";
    case KEY_A: return "A";
    case KEY_B: return "B";
    case KEY_C: return "C";
    case KEY_D: return "D";
    case KEY_E: return "E";
    case KEY_F: return "F";
    case KEY_G: return "G";
    case KEY_H: return "H";
    case KEY_I: return "I";
    case KEY_J: return "J";
    case KEY_K: return "K";
    case KEY_L: return "L";
    case KEY_M: return "M";
    case KEY_N: return "N";
    case KEY_O: return "O";
    case KEY_P: return "P";
    case KEY_Q: return "Q";
    case KEY_R: return "R";
    case KEY_S: return "S";
    case KEY_T: return "T";
    case KEY_U: return "U";
    case KEY_V: return "V";
    case KEY_W: return "W";
    case KEY_X: return "X";
    case KEY_Y: return "Y";
    case KEY_Z: return "Z";
    case KEY_RETURN: return "RETURN";
    case KEY_ESCAPE: return "ESCAPE";
    case KEY_BACKSPACE: return "BACKSPACE";
    case KEY_TAB: return "TAB";
    case KEY_SPACE: return "SPACE";
    case KEY_MINUS: return "MINUS";
    case KEY_EQUALS: return "EQUALS";
    case KEY_LEFTBRACKET: return "LEFTBRACKET";
    case KEY_RIGHTBRACKET: return "RIGHTBRACKET";
    case KEY_BACKSLASH: return "BACKSLASH";
    case KEY_SEMICOLON: return "SEMICOLON";
    case KEY_APOSTROPHE: return "APOSTROPHE";
    case KEY_GRAVE: return "GRAVE";
    case KEY_COMMA: return "COMMA";
    case KEY_PERIOD: return "PERIOD";
    case KEY_SLASH: return "SLASH";
    case KEY_CAPSLOCK: return "CAPSLOCK";
    case KEY_F1: return "F1";
    case KEY_F2: return "F2";
    case KEY_F3: return "F3";
    case KEY_F4: return "F4";
    case KEY_F5: return "F5";
    case KEY_F6: return "F6";
    case KEY_F7: return "F7";
    case KEY_F8: return "F8";
    case KEY_F9: return "F9";
    case KEY_F10: return "F10";
    case KEY_F11: return "F11";
    case KEY_F12: return "F12";
    case KEY_PRINTSCREEN: return "PRINTSCREEN";
    case KEY_SCROLLLOCK: return "SCROLLLOCK";
    case KEY_PAUSE: return "PAUSE";
    case KEY_INSERT: return "INSERT";
    case KEY_HOME: return "HOME";
    case KEY_PAGEUP: return "PAGEUP";
    case KEY_DELETE: return "DELETE";
    case KEY_END: return "END";
    case KEY_PAGEDOWN: return "PAGEDOWN";
    case KEY_RIGHT: return "RIGHT";
    case KEY_LEFT: return "LEFT";
    case KEY_DOWN: return "DOWN";
    case KEY_UP: return "UP";
    case KEY_NUMLOCK: return "NUMLOCK";
    case KEY_KP_DIVIDE: return "KP_DIVIDE";
    case KEY_KP_MULTIPLY: return "KP_MULTIPLY";
    case KEY_KP_MINUS: return "KP_MINUS";
    case KEY_KP_PLUS: return "KP_PLUS";
    case KEY_KP_ENTER: return "KP_ENTER";
    case KEY_KP_0: return "KP_0";
    case KEY_KP_1: return "KP_1";
    case KEY_KP_2: return "KP_2";
    case KEY_KP_3: return "KP_3";
    case KEY_KP_4: return "KP_4";
    case KEY_KP_5: return "KP_5";
    case KEY_KP_6: return "KP_6";
    case KEY_KP_7: return "KP_7";
    case KEY_KP_8: return "KP_8";
    case KEY_KP_9: return "KP_9";
    case KEY_KP_PERIOD: return "KP_PERIOD";
    case KEY_SYSREQ: return "SYSREQ";
    case KEY_LCTRL: return "LCTRL";
    case KEY_LSHIFT: return "LSHIFT";
    case KEY_LALT: return "LALT";
    case KEY_LSUPER: return "LSUPER";
    case KEY_RCTRL: return "RCTRL";
    case KEY_RSHIFT: return "RSHIFT";
    case KEY_RALT: return "RALT";
    case KEY_RSUPER: return "RSUPER";
    default: return "UNKNOWN";
  }
}

int I_string_to_key (const char *str) {
  if (cp866_strcasecmp(str, "0") == 0) return KEY_0;
  else if (cp866_strcasecmp(str, "1") == 0) return KEY_1;
  else if (cp866_strcasecmp(str, "2") == 0) return KEY_2;
  else if (cp866_strcasecmp(str, "3") == 0) return KEY_3;
  else if (cp866_strcasecmp(str, "4") == 0) return KEY_4;
  else if (cp866_strcasecmp(str, "5") == 0) return KEY_5;
  else if (cp866_strcasecmp(str, "6") == 0) return KEY_6;
  else if (cp866_strcasecmp(str, "7") == 0) return KEY_7;
  else if (cp866_strcasecmp(str, "8") == 0) return KEY_8;
  else if (cp866_strcasecmp(str, "9") == 0) return KEY_9;
  else if (cp866_strcasecmp(str, "A") == 0) return KEY_A;
  else if (cp866_strcasecmp(str, "B") == 0) return KEY_B;
  else if (cp866_strcasecmp(str, "C") == 0) return KEY_C;
  else if (cp866_strcasecmp(str, "D") == 0) return KEY_D;
  else if (cp866_strcasecmp(str, "E") == 0) return KEY_E;
  else if (cp866_strcasecmp(str, "F") == 0) return KEY_F;
  else if (cp866_strcasecmp(str, "G") == 0) return KEY_G;
  else if (cp866_strcasecmp(str, "H") == 0) return KEY_H;
  else if (cp866_strcasecmp(str, "I") == 0) return KEY_I;
  else if (cp866_strcasecmp(str, "J") == 0) return KEY_J;
  else if (cp866_strcasecmp(str, "K") == 0) return KEY_K;
  else if (cp866_strcasecmp(str, "L") == 0) return KEY_L;
  else if (cp866_strcasecmp(str, "M") == 0) return KEY_M;
  else if (cp866_strcasecmp(str, "N") == 0) return KEY_N;
  else if (cp866_strcasecmp(str, "O") == 0) return KEY_O;
  else if (cp866_strcasecmp(str, "P") == 0) return KEY_P;
  else if (cp866_strcasecmp(str, "Q") == 0) return KEY_Q;
  else if (cp866_strcasecmp(str, "R") == 0) return KEY_R;
  else if (cp866_strcasecmp(str, "S") == 0) return KEY_S;
  else if (cp866_strcasecmp(str, "T") == 0) return KEY_T;
  else if (cp866_strcasecmp(str, "U") == 0) return KEY_U;
  else if (cp866_strcasecmp(str, "V") == 0) return KEY_V;
  else if (cp866_strcasecmp(str, "W") == 0) return KEY_W;
  else if (cp866_strcasecmp(str, "X") == 0) return KEY_X;
  else if (cp866_strcasecmp(str, "Y") == 0) return KEY_Y;
  else if (cp866_strcasecmp(str, "Z") == 0) return KEY_Z;
  else if (cp866_strcasecmp(str, "RETURN") == 0) return KEY_RETURN;
  else if (cp866_strcasecmp(str, "ESCAPE") == 0) return KEY_ESCAPE;
  else if (cp866_strcasecmp(str, "BACKSPACE") == 0) return KEY_BACKSPACE;
  else if (cp866_strcasecmp(str, "TAB") == 0) return KEY_TAB;
  else if (cp866_strcasecmp(str, "SPACE") == 0) return KEY_SPACE;
  else if (cp866_strcasecmp(str, "MINUS") == 0) return KEY_MINUS;
  else if (cp866_strcasecmp(str, "EQUALS") == 0) return KEY_EQUALS;
  else if (cp866_strcasecmp(str, "LEFTBRACKET") == 0) return KEY_LEFTBRACKET;
  else if (cp866_strcasecmp(str, "RIGHTBRACKET") == 0) return KEY_RIGHTBRACKET;
  else if (cp866_strcasecmp(str, "BACKSLASH") == 0) return KEY_BACKSLASH;
  else if (cp866_strcasecmp(str, "SEMICOLON") == 0) return KEY_SEMICOLON;
  else if (cp866_strcasecmp(str, "APOSTROPHE") == 0) return KEY_APOSTROPHE;
  else if (cp866_strcasecmp(str, "GRAVE") == 0) return KEY_GRAVE;
  else if (cp866_strcasecmp(str, "COMMA") == 0) return KEY_COMMA;
  else if (cp866_strcasecmp(str, "PERIOD") == 0) return KEY_PERIOD;
  else if (cp866_strcasecmp(str, "SLASH") == 0) return KEY_SLASH;
  else if (cp866_strcasecmp(str, "CAPSLOCK") == 0) return KEY_CAPSLOCK;
  else if (cp866_strcasecmp(str, "F1") == 0) return KEY_F1;
  else if (cp866_strcasecmp(str, "F2") == 0) return KEY_F2;
  else if (cp866_strcasecmp(str, "F3") == 0) return KEY_F3;
  else if (cp866_strcasecmp(str, "F4") == 0) return KEY_F4;
  else if (cp866_strcasecmp(str, "F5") == 0) return KEY_F5;
  else if (cp866_strcasecmp(str, "F6") == 0) return KEY_F6;
  else if (cp866_strcasecmp(str, "F7") == 0) return KEY_F7;
  else if (cp866_strcasecmp(str, "F8") == 0) return KEY_F8;
  else if (cp866_strcasecmp(str, "F9") == 0) return KEY_F9;
  else if (cp866_strcasecmp(str, "F10") == 0) return KEY_F10;
  else if (cp866_strcasecmp(str, "F11") == 0) return KEY_F11;
  else if (cp866_strcasecmp(str, "F12") == 0) return KEY_F12;
  else if (cp866_strcasecmp(str, "PRINTSCREEN") == 0) return KEY_PRINTSCREEN;
  else if (cp866_strcasecmp(str, "SCROLLLOCK") == 0) return KEY_SCROLLLOCK;
  else if (cp866_strcasecmp(str, "PAUSE") == 0) return KEY_PAUSE;
  else if (cp866_strcasecmp(str, "INSERT") == 0) return KEY_INSERT;
  else if (cp866_strcasecmp(str, "HOME") == 0) return KEY_HOME;
  else if (cp866_strcasecmp(str, "PAGEUP") == 0) return KEY_PAGEUP;
  else if (cp866_strcasecmp(str, "DELETE") == 0) return KEY_DELETE;
  else if (cp866_strcasecmp(str, "END") == 0) return KEY_END;
  else if (cp866_strcasecmp(str, "PAGEDOWN") == 0) return KEY_PAGEDOWN;
  else if (cp866_strcasecmp(str, "RIGHT") == 0) return KEY_RIGHT;
  else if (cp866_strcasecmp(str, "LEFT") == 0) return KEY_LEFT;
  else if (cp866_strcasecmp(str, "DOWN") == 0) return KEY_DOWN;
  else if (cp866_strcasecmp(str, "UP") == 0) return KEY_UP;
  else if (cp866_strcasecmp(str, "NUMLOCK") == 0) return KEY_NUMLOCK;
  else if (cp866_strcasecmp(str, "KP_DIVIDE") == 0) return KEY_KP_DIVIDE;
  else if (cp866_strcasecmp(str, "KP_MULTIPLY") == 0) return KEY_KP_MULTIPLY;
  else if (cp866_strcasecmp(str, "KP_MINUS") == 0) return KEY_KP_MINUS;
  else if (cp866_strcasecmp(str, "KP_PLUS") == 0) return KEY_KP_PLUS;
  else if (cp866_strcasecmp(str, "KP_ENTER") == 0) return KEY_KP_ENTER;
  else if (cp866_strcasecmp(str, "KP_0") == 0) return KEY_KP_0;
  else if (cp866_strcasecmp(str, "KP_1") == 0) return KEY_KP_1;
  else if (cp866_strcasecmp(str, "KP_2") == 0) return KEY_KP_2;
  else if (cp866_strcasecmp(str, "KP_3") == 0) return KEY_KP_3;
  else if (cp866_strcasecmp(str, "KP_4") == 0) return KEY_KP_4;
  else if (cp866_strcasecmp(str, "KP_5") == 0) return KEY_KP_5;
  else if (cp866_strcasecmp(str, "KP_6") == 0) return KEY_KP_6;
  else if (cp866_strcasecmp(str, "KP_7") == 0) return KEY_KP_7;
  else if (cp866_strcasecmp(str, "KP_8") == 0) return KEY_KP_8;
  else if (cp866_strcasecmp(str, "KP_9") == 0) return KEY_KP_9;
  else if (cp866_strcasecmp(str, "KP_PERIOD") == 0) return KEY_KP_PERIOD;
  else if (cp866_strcasecmp(str, "SYSREQ") == 0) return KEY_SYSREQ;
  else if (cp866_strcasecmp(str, "LCTRL") == 0) return KEY_LCTRL;
  else if (cp866_strcasecmp(str, "LSHIFT") == 0) return KEY_LSHIFT;
  else if (cp866_strcasecmp(str, "LALT") == 0) return KEY_LALT;
  else if (cp866_strcasecmp(str, "LSUPER") == 0) return KEY_LSUPER;
  else if (cp866_strcasecmp(str, "RCTRL") == 0) return KEY_RCTRL;
  else if (cp866_strcasecmp(str, "RSHIFT") == 0) return KEY_RSHIFT;
  else if (cp866_strcasecmp(str, "RALT") == 0) return KEY_RALT;
  else if (cp866_strcasecmp(str, "RSUPER") == 0) return KEY_RSUPER;
  else return KEY_UNKNOWN;
}

int I_pressed (int key) {
  assert(key >= 0 && key <= KEY__LAST);
  return keystate[key];
}

void I_press (int key, int down) {
  assert(key >= 0 && key <= KEY__LAST);
  keystate[key] = down != 0;
}
