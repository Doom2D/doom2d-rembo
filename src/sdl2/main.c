#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h> // srand exit
#include <string.h> // strcasecmp
#include <assert.h>
#include "system.h"
#include "input.h"

#include "my.h" // fexists
#include "player.h" // pl1 pl2
#include "menu.h" // G_keyf
#include "error.h" // logo

#include "files.h" // F_startup F_addwad F_initwads F_allocres
#include "config.h" // CFG_args CFG_load CFG_save
#include "memory.h" // M_startup
#include "game.h" // G_init G_act
#include "sound.h" // S_init S_done
#include "music.h" // S_initmusic S_updatemusic S_donemusic
#include "render.h" // R_init R_draw R_done

#define TITLE_STR "Doom 2D (SDL2)"

static Uint32 ticks;
static int quit = 0;
static SDL_Window *window;
static SDL_GLContext context;
static SDL_Surface *surf;

/* --- error.h --- */

void logo (const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  vprintf(s, ap);
  va_end(ap);
  fflush(stdout);
}

void logo_gas (int cur, int all) {
  // stub
}

void ERR_failinit (char *s, ...) {
  va_list ap;
  va_start(ap, s);
  vprintf(s, ap);
  va_end(ap);
  puts("");
  exit(1);
}

void ERR_fatal (char *s, ...) {
  va_list ap;
  R_done();
  S_done();
  S_donemusic();
  M_shutdown();
  SDL_Quit();
  puts("\nКРИТИЧЕСКАЯ ОШИБКА:");
  va_start(ap, s);
  vprintf(s, ap);
  va_end(ap);
  puts("");
  exit(1);
}

void ERR_quit (void) {
  quit = 1;
}

/* --- system.h --- */

static int Y_resize_window (int w, int h, int fullscreen) {
  assert(w > 0);
  assert(h > 0);
  assert(window != NULL);
  if (surf != NULL) {
    if (surf->w != w || surf->h != h) {
      SDL_Surface *s = SDL_CreateRGBSurface(0, w, h, 8, 0, 0, 0, 0);
      if (s != NULL) {
        SDL_SetPaletteColors(s->format->palette, surf->format->palette->colors, 0, surf->format->palette->ncolors);
        SDL_FreeSurface(surf);
        surf = s;
      }
    }
  }
  SDL_SetWindowSize(window, w, h);
  Y_set_fullscreen(fullscreen);
  return 1;
}

int Y_set_videomode_opengl (int w, int h, int fullscreen) {
  assert(w > 0);
  assert(h > 0);
  Uint32 flags;
  SDL_Window *win;
  SDL_GLContext ctx;
  if (window != NULL && context != NULL) {
    Y_resize_window(w, h, fullscreen);
    win = window;
  } else {
    flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
    if (fullscreen) {
      flags = flags | SDL_WINDOW_FULLSCREEN;
    }
    // TODO set context version and type
#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    win = SDL_CreateWindow(TITLE_STR, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
    if (win != NULL) {
      ctx = SDL_GL_CreateContext(win);
      if (ctx != NULL) {
        Y_unset_videomode();
        window = win;
        context = ctx;
        SDL_GL_MakeCurrent(window, context);
      } else {
        SDL_DestroyWindow(win);
        win = NULL;
      }
    }
  }
  if (win == NULL) {
    logo("Y_set_videomode_opengl: error: %s\n", SDL_GetError());
  }
  return win != NULL;
}

int Y_set_videomode_software (int w, int h, int fullscreen) {
  assert(w > 0);
  assert(h > 0);
  Uint32 flags;
  SDL_Surface *s;
  SDL_Window *win;
  if (window != NULL && surf != NULL) {
    Y_resize_window(w, h, fullscreen);
    win = window;
  } else {
    flags = SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
      flags = flags | SDL_WINDOW_FULLSCREEN;
    }
    win = SDL_CreateWindow(TITLE_STR, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
    if (win != NULL) {
      s = SDL_CreateRGBSurface(0, w, h, 8, 0, 0, 0, 0);
      if (s != NULL) {
        Y_unset_videomode();
        window = win;
        surf = s;
      } else {
        SDL_DestroyWindow(win);
        win = NULL;
      }
    }
  }
  if (win == NULL) {
    logo("Y_set_videomode_software: error: %s\n", SDL_GetError());
  }
  return win != NULL;
}

void Y_get_videomode (int *w, int *h) {
  if (window != NULL) {
    SDL_GetWindowSize(window, w, h);
  } else {
    *w = 0;
    *h = 0;
  }
}

int Y_videomode_setted (void) {
  return window != NULL;
}

void Y_unset_videomode (void) {
  if (window != NULL) {
    if (context != NULL) {
      SDL_GL_MakeCurrent(window, NULL);
      SDL_GL_DeleteContext(context);
      context = NULL;
    }
    if (surf != NULL) {
      SDL_FreeSurface(surf);
      surf = NULL;
    }
    SDL_DestroyWindow(window);
    window = NULL;
  }
}

void Y_set_fullscreen (int yes) {
  if (window != NULL) {
    SDL_SetWindowFullscreen(window, yes ? SDL_WINDOW_FULLSCREEN : 0);
  }
}

int Y_get_fullscreen (void) {
  return (window != NULL) && (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN);
}

void Y_swap_buffers (void) {
  assert(window != NULL);
  assert(context != NULL);
  SDL_GL_SwapWindow(window);
}

void Y_get_buffer (byte **buf, int *w, int *h, int *pitch) {
  assert(window != NULL);
  assert(surf != NULL);
  *buf = surf->pixels;
  *w = surf->w;
  *h = surf->h;
  *pitch = surf->pitch;
}

void Y_set_vga_palette (byte *vgapal) {
  assert(window != NULL);
  assert(surf != NULL);
  int i;
  byte *p = vgapal;
  SDL_Color colors[256];
  for (i = 0; i < 256; i++) {
    colors[i] = (SDL_Color) {
      .r = p[0] * 255 / 63,
      .g = p[1] * 255 / 63,
      .b = p[2] * 255 / 63
    };
    p += 3;
  }
  SDL_SetPaletteColors(surf->format->palette, colors, 0, 256);
}

void Y_repaint_rect (int x, int y, int w, int h) {
  assert(window != NULL);
  assert(surf != NULL);
  SDL_Surface *s = SDL_GetWindowSurface(window);
  SDL_Rect r = (SDL_Rect) {
    .x = x,
    .y = y,
    .w = w,
    .h = h
  };
  SDL_BlitSurface(surf, &r, s, &r);
  SDL_UpdateWindowSurfaceRects(window, &r, 1);
}

void Y_repaint (void) {
  Y_repaint_rect(0, 0, surf->w, surf->h);
}

void Y_enable_text_input (void) {
  SDL_StartTextInput();
}

void Y_disable_text_input (void) {
  SDL_StopTextInput();
}

/* --- main --- */

static int sdl_to_key (int code) {
  switch (code) {
    case SDL_SCANCODE_0: return KEY_0;
    case SDL_SCANCODE_1: return KEY_1;
    case SDL_SCANCODE_2: return KEY_2;
    case SDL_SCANCODE_3: return KEY_3;
    case SDL_SCANCODE_4: return KEY_4;
    case SDL_SCANCODE_5: return KEY_5;
    case SDL_SCANCODE_6: return KEY_6;
    case SDL_SCANCODE_7: return KEY_7;
    case SDL_SCANCODE_8: return KEY_8;
    case SDL_SCANCODE_9: return KEY_9;
    case SDL_SCANCODE_A: return KEY_A;
    case SDL_SCANCODE_B: return KEY_B;
    case SDL_SCANCODE_C: return KEY_C;
    case SDL_SCANCODE_D: return KEY_D;
    case SDL_SCANCODE_E: return KEY_E;
    case SDL_SCANCODE_F: return KEY_F;
    case SDL_SCANCODE_G: return KEY_G;
    case SDL_SCANCODE_H: return KEY_H;
    case SDL_SCANCODE_I: return KEY_I;
    case SDL_SCANCODE_J: return KEY_J;
    case SDL_SCANCODE_K: return KEY_K;
    case SDL_SCANCODE_L: return KEY_L;
    case SDL_SCANCODE_M: return KEY_M;
    case SDL_SCANCODE_N: return KEY_N;
    case SDL_SCANCODE_O: return KEY_O;
    case SDL_SCANCODE_P: return KEY_P;
    case SDL_SCANCODE_Q: return KEY_Q;
    case SDL_SCANCODE_R: return KEY_R;
    case SDL_SCANCODE_S: return KEY_S;
    case SDL_SCANCODE_T: return KEY_T;
    case SDL_SCANCODE_U: return KEY_U;
    case SDL_SCANCODE_V: return KEY_V;
    case SDL_SCANCODE_W: return KEY_W;
    case SDL_SCANCODE_X: return KEY_X;
    case SDL_SCANCODE_Y: return KEY_Y;
    case SDL_SCANCODE_Z: return KEY_Z;
    case SDL_SCANCODE_RETURN: return KEY_RETURN;
    case SDL_SCANCODE_ESCAPE: return KEY_ESCAPE;
    case SDL_SCANCODE_BACKSPACE: return KEY_BACKSPACE;
    case SDL_SCANCODE_TAB: return KEY_TAB;
    case SDL_SCANCODE_SPACE: return KEY_SPACE;
    case SDL_SCANCODE_MINUS: return KEY_MINUS;
    case SDL_SCANCODE_EQUALS: return KEY_EQUALS;
    case SDL_SCANCODE_LEFTBRACKET: return KEY_LEFTBRACKET;
    case SDL_SCANCODE_RIGHTBRACKET: return KEY_RIGHTBRACKET;
    case SDL_SCANCODE_BACKSLASH: return KEY_BACKSLASH;
    case SDL_SCANCODE_SEMICOLON: return KEY_SEMICOLON;
    case SDL_SCANCODE_APOSTROPHE: return KEY_APOSTROPHE;
    case SDL_SCANCODE_GRAVE: return KEY_GRAVE;
    case SDL_SCANCODE_COMMA: return KEY_COMMA;
    case SDL_SCANCODE_PERIOD: return KEY_PERIOD;
    case SDL_SCANCODE_SLASH: return KEY_SLASH;
    case SDL_SCANCODE_CAPSLOCK: return KEY_CAPSLOCK;
    case SDL_SCANCODE_F1: return KEY_F1;
    case SDL_SCANCODE_F2: return KEY_F2;
    case SDL_SCANCODE_F3: return KEY_F3;
    case SDL_SCANCODE_F4: return KEY_F4;
    case SDL_SCANCODE_F5: return KEY_F5;
    case SDL_SCANCODE_F6: return KEY_F6;
    case SDL_SCANCODE_F7: return KEY_F7;
    case SDL_SCANCODE_F8: return KEY_F8;
    case SDL_SCANCODE_F9: return KEY_F9;
    case SDL_SCANCODE_F10: return KEY_F10;
    case SDL_SCANCODE_F11: return KEY_F11;
    case SDL_SCANCODE_F12: return KEY_F12;
    case SDL_SCANCODE_PRINTSCREEN: return KEY_PRINTSCREEN;
    case SDL_SCANCODE_SCROLLLOCK: return KEY_SCROLLLOCK;
    case SDL_SCANCODE_PAUSE: return KEY_PAUSE;
    case SDL_SCANCODE_INSERT: return KEY_INSERT;
    case SDL_SCANCODE_HOME: return KEY_HOME;
    case SDL_SCANCODE_PAGEUP: return KEY_PAGEUP;
    case SDL_SCANCODE_DELETE: return KEY_DELETE;
    case SDL_SCANCODE_END: return KEY_END;
    case SDL_SCANCODE_PAGEDOWN: return KEY_PAGEDOWN;
    case SDL_SCANCODE_RIGHT: return KEY_RIGHT;
    case SDL_SCANCODE_LEFT: return KEY_LEFT;
    case SDL_SCANCODE_DOWN: return KEY_DOWN;
    case SDL_SCANCODE_UP: return KEY_UP;
    case SDL_SCANCODE_NUMLOCKCLEAR: return KEY_NUMLOCK;
    case SDL_SCANCODE_KP_DIVIDE: return KEY_KP_DIVIDE;
    case SDL_SCANCODE_KP_MULTIPLY: return KEY_KP_MULTIPLY;
    case SDL_SCANCODE_KP_MINUS: return KEY_KP_MINUS;
    case SDL_SCANCODE_KP_PLUS: return KEY_KP_PLUS;
    case SDL_SCANCODE_KP_ENTER: return KEY_KP_ENTER;
    case SDL_SCANCODE_KP_0: return KEY_KP_0;
    case SDL_SCANCODE_KP_1: return KEY_KP_1;
    case SDL_SCANCODE_KP_2: return KEY_KP_2;
    case SDL_SCANCODE_KP_3: return KEY_KP_3;
    case SDL_SCANCODE_KP_4: return KEY_KP_4;
    case SDL_SCANCODE_KP_5: return KEY_KP_5;
    case SDL_SCANCODE_KP_6: return KEY_KP_6;
    case SDL_SCANCODE_KP_7: return KEY_KP_7;
    case SDL_SCANCODE_KP_8: return KEY_KP_8;
    case SDL_SCANCODE_KP_9: return KEY_KP_9;
    case SDL_SCANCODE_KP_PERIOD: return KEY_KP_PERIOD;
    case SDL_SCANCODE_SYSREQ: return KEY_SYSREQ;
    case SDL_SCANCODE_LCTRL: return KEY_LCTRL;
    case SDL_SCANCODE_LSHIFT: return KEY_LSHIFT;
    case SDL_SCANCODE_LALT: return KEY_LALT;
    case SDL_SCANCODE_LGUI: return KEY_LSUPER;
    case SDL_SCANCODE_RCTRL: return KEY_RCTRL;
    case SDL_SCANCODE_RSHIFT: return KEY_RSHIFT;
    case SDL_SCANCODE_RALT: return KEY_RALT;
    case SDL_SCANCODE_RGUI: return KEY_RSUPER;
    default: return KEY_UNKNOWN;
  }
}

static void window_event_handler (SDL_WindowEvent *ev) {
  switch (ev->event) {
    case SDL_WINDOWEVENT_RESIZED:
      R_set_videomode(ev->data1, ev->data2, Y_get_fullscreen());
      break;
    case SDL_WINDOWEVENT_CLOSE:
      ERR_quit();
      break;
  }
}

static int utf8_to_wchar (char *x) {
  int i = 0;
  byte *s = (byte*)x;
  if (s[0] < 0x80) {
    return s[0];
  } else if (s[0] < 0xE0) {
    if (s[0] - 192 >= 0 && s[1] >= 0x80 && s[1] < 0xE0) {
      i = (s[0] - 192) * 64 + s[1] - 128;
    }
  } else if (s[0] < 0xF0) {
    if (s[1] >= 0x80 && s[1] < 0xE0 && s[2] >= 0x80 && s[2] < 0xE0) {
      i = ((s[0] - 224) * 64 + s[1] - 128) * 64 + s[2] - 128;
    }
  }
  return i;
}

static int wchar_to_cp866 (int uch) {
  if (uch <= 0x7f) {
    return uch;
  } else if (uch >= 0x410 && uch <= 0x43f) {
    return uch - 0x410 + 0x80;
  } else if (uch >= 0x440 && uch <= 0x44f) {
    return uch - 0x440 + 0xe0;
  } else {
    switch (uch) {
      // TODO graphics from 0xb0..0xdf
      case 0x401: return 0xf0;
      case 0x451: return 0xf1;
      case 0x404: return 0xf2;
      case 0x454: return 0xf3;
      case 0x407: return 0xf4;
      case 0x457: return 0xf5;
      case 0x40e: return 0xf6;
      case 0x45e: return 0xf7;
      case 0xb0: return 0xf8;
      case 0x2219: return 0xf9;
      case 0xb7: return 0xfa;
      case 0x221a: return 0xfb;
      case 0x2116: return 0xfc;
      case 0xa4: return 0xfd;
      case 0x25a0: return 0xfe;
      case 0xa0: return 0xff;
      default: return 0; // unknown
    }
  }
}

static void poll_events (void) {
  int key, down, uch, ch;
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    switch (ev.type) {
      case SDL_QUIT:
        ERR_quit();
        break;
      case SDL_WINDOWEVENT:
        if (ev.window.windowID == SDL_GetWindowID(window)) {
          window_event_handler(&ev.window);
        }
        break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        down = ev.type == SDL_KEYDOWN;
        key = sdl_to_key(ev.key.keysym.scancode);
        I_press(key, down);
        GM_key(key, down);
        break;
      case SDL_TEXTINPUT:
        uch = utf8_to_wchar(ev.text.text);
        ch = wchar_to_cp866(uch);
        GM_input(ch);
        break;
    }
  }
}

static void step (void) {
  poll_events();
  S_updatemusic();
  Uint32 t = SDL_GetTicks();
  if (t - ticks > DELAY) {
    ticks = t;
    G_act();
  }
  R_draw();
}

int main (int argc, char **argv) {
  char *pw;
  logo("system: initialize SDL2\n");
  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS) == -1) {
    logo("system: failed to init SDL2: %s\n", SDL_GetError());
    return 1;
  }
  // Player 1 defaults
  pl1.ku = KEY_KP_8;
  pl1.kd = KEY_KP_5;
  pl1.kl = KEY_KP_4;
  pl1.kr = KEY_KP_6;
  pl1.kf = KEY_PAGEDOWN;
  pl1.kj = KEY_DELETE;
  pl1.kwl = KEY_HOME;
  pl1.kwr = KEY_END;
  pl1.kp = KEY_KP_8;
  // Player 2 defaults
  pl2.ku = KEY_E;
  pl2.kd = KEY_D;
  pl2.kl = KEY_S;
  pl2.kr = KEY_F;
  pl2.kf = KEY_A;
  pl2.kj = KEY_Q;
  pl2.kwl = KEY_1;
  pl2.kwr = KEY_2;
  pl2.kp = KEY_E;
  srand(SDL_GetTicks());
  F_startup();
#ifndef WIN32
  pw = "/usr/share/doom2d-rembo/doom2d.wad";
#else
  pw = "doom2d.wad";
#endif
  if (fexists(pw)) {
    F_addwad(pw);
  } else {
    F_addwad("doom2d.wad");
  }
  CFG_args(argc, argv);
  CFG_load();
  F_initwads();
  M_startup();
  F_allocres();
  S_init();
  S_initmusic();
  R_init();
  G_init();
  ticks = SDL_GetTicks();
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(step, 0, 1);
#else
  while (!quit) {
    step();
  }
#endif
  CFG_save();
  R_done();
  S_donemusic();
  S_done();
  M_shutdown();
  SDL_Quit();
  return 0;
}