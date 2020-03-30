#include <SDL.h>
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

static int quit = 0;
static SDL_Window *window;
static SDL_GLContext context;

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
  puts("Спасибо за то, что вы играли в Операцию \"Смятка\"!");
  //F_loadres(F_getresid("ENDOOM"),p,0,4000);
  quit = 1;
}

/* --- system.h --- */

int Y_set_videomode (int w, int h, int flags) {
  assert(w > 0);
  assert(h > 0);
  Uint32 f = SDL_WINDOW_HIDDEN;
  if (flags & SYSTEM_USE_FULLSCREEN) {
    f = f | SDL_WINDOW_FULLSCREEN;
  }
  int res = 0;
  int x = SDL_WINDOWPOS_CENTERED;
  int y = SDL_WINDOWPOS_CENTERED;
  if (flags & SYSTEM_USE_OPENGL) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_Window *win = SDL_CreateWindow("Doom 2D (SDL2)", x, y, w, h, f | SDL_WINDOW_OPENGL);
    if (win != NULL) {
      SDL_GLContext ctx = SDL_GL_CreateContext(win);
      if (ctx != NULL) {
        Y_unset_videomode();
        window = win;
        context = ctx;
        SDL_GL_MakeCurrent(window, context);
        SDL_ShowWindow(window);
        res = 1;
      } else {
        SDL_DestroyWindow(win);
      }
    }
  } else {
    // TODO software
  }
  return res;
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
    SDL_DestroyWindow(window);
    window = NULL;
  }
}

int Y_set_fullscreen (int yes) {
  if (window != NULL) {
    SDL_SetWindowFullscreen(window, yes ? SDL_WINDOW_FULLSCREEN : 0);
  }
  return yes;
}

int Y_get_fullscreen (void) {
  return (window != NULL) && (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN);
}

void Y_swap_buffers (void) {
  assert(context != NULL);
  SDL_GL_SwapWindow(window);
}

void Y_get_buffer (byte **buf, int *w, int *h, int *pitch) {
  // TODO
}

void Y_set_vga_palette (byte *vgapal) {
  // TODO
}

void Y_repaint_rect (int x, int y, int w, int h) {
  // TODO
}

void Y_repaint (void) {
  // TODO
}

/* --- main --- */

static int sdl_to_key (int code) {
  switch (code) {
    case SDLK_0: return KEY_0;
    case SDLK_1: return KEY_1;
    case SDLK_2: return KEY_2;
    case SDLK_3: return KEY_3;
    case SDLK_4: return KEY_4;
    case SDLK_5: return KEY_5;
    case SDLK_6: return KEY_6;
    case SDLK_7: return KEY_7;
    case SDLK_8: return KEY_8;
    case SDLK_9: return KEY_9;
    case SDLK_a: return KEY_A;
    case SDLK_b: return KEY_B;
    case SDLK_c: return KEY_C;
    case SDLK_d: return KEY_D;
    case SDLK_e: return KEY_E;
    case SDLK_f: return KEY_F;
    case SDLK_g: return KEY_G;
    case SDLK_h: return KEY_H;
    case SDLK_i: return KEY_I;
    case SDLK_j: return KEY_J;
    case SDLK_k: return KEY_K;
    case SDLK_l: return KEY_L;
    case SDLK_m: return KEY_M;
    case SDLK_n: return KEY_N;
    case SDLK_o: return KEY_O;
    case SDLK_p: return KEY_P;
    case SDLK_q: return KEY_Q;
    case SDLK_r: return KEY_R;
    case SDLK_s: return KEY_S;
    case SDLK_t: return KEY_T;
    case SDLK_u: return KEY_U;
    case SDLK_v: return KEY_V;
    case SDLK_w: return KEY_W;
    case SDLK_x: return KEY_X;
    case SDLK_y: return KEY_Y;
    case SDLK_z: return KEY_Z;
    case SDLK_RETURN: return KEY_RETURN;
    case SDLK_ESCAPE: return KEY_ESCAPE;
    case SDLK_BACKSPACE: return KEY_BACKSPACE;
    case SDLK_TAB: return KEY_TAB;
    case SDLK_SPACE: return KEY_SPACE;
    case SDLK_MINUS: return KEY_MINUS;
    case SDLK_EQUALS: return KEY_EQUALS;
    case SDLK_LEFTBRACKET: return KEY_LEFTBRACKET;
    case SDLK_RIGHTBRACKET: return KEY_RIGHTBRACKET;
    case SDLK_BACKSLASH: return KEY_BACKSLASH;
    case SDLK_SEMICOLON: return KEY_SEMICOLON;
    case SDLK_QUOTE: return KEY_APOSTROPHE;
    case SDLK_BACKQUOTE: return KEY_GRAVE;
    case SDLK_COMMA: return KEY_COMMA;
    case SDLK_PERIOD: return KEY_PERIOD;
    case SDLK_SLASH: return KEY_SLASH;
    case SDLK_CAPSLOCK: return KEY_CAPSLOCK;
    case SDLK_F1: return KEY_F1;
    case SDLK_F2: return KEY_F2;
    case SDLK_F3: return KEY_F3;
    case SDLK_F4: return KEY_F4;
    case SDLK_F5: return KEY_F5;
    case SDLK_F6: return KEY_F6;
    case SDLK_F7: return KEY_F7;
    case SDLK_F8: return KEY_F8;
    case SDLK_F9: return KEY_F9;
    case SDLK_F10: return KEY_F10;
    case SDLK_F11: return KEY_F11;
    case SDLK_F12: return KEY_F12;
    case SDLK_PRINTSCREEN: return KEY_PRINTSCREEN;
    case SDLK_SCROLLLOCK: return KEY_SCROLLLOCK;
    case SDLK_PAUSE: return KEY_PAUSE;
    case SDLK_INSERT: return KEY_INSERT;
    case SDLK_HOME: return KEY_HOME;
    case SDLK_PAGEUP: return KEY_PAGEUP;
    case SDLK_DELETE: return KEY_DELETE;
    case SDLK_END: return KEY_END;
    case SDLK_PAGEDOWN: return KEY_PAGEDOWN;
    case SDLK_RIGHT: return KEY_RIGHT;
    case SDLK_LEFT: return KEY_LEFT;
    case SDLK_DOWN: return KEY_DOWN;
    case SDLK_UP: return KEY_UP;
    //case SDLK_NUMLOCK: return KEY_NUMLOCK;
    case SDLK_KP_DIVIDE: return KEY_KP_DIVIDE;
    case SDLK_KP_MULTIPLY: return KEY_KP_MULTIPLY;
    case SDLK_KP_MINUS: return KEY_KP_MINUS;
    case SDLK_KP_PLUS: return KEY_KP_PLUS;
    case SDLK_KP_ENTER: return KEY_KP_ENTER;
    case SDLK_KP_0: return KEY_KP_0;
    case SDLK_KP_1: return KEY_KP_1;
    case SDLK_KP_2: return KEY_KP_2;
    case SDLK_KP_3: return KEY_KP_3;
    case SDLK_KP_4: return KEY_KP_4;
    case SDLK_KP_5: return KEY_KP_5;
    case SDLK_KP_6: return KEY_KP_6;
    case SDLK_KP_7: return KEY_KP_7;
    case SDLK_KP_8: return KEY_KP_8;
    case SDLK_KP_9: return KEY_KP_9;
    case SDLK_KP_PERIOD: return KEY_KP_PERIOD;
    case SDLK_SYSREQ: return KEY_SYSREQ;
    case SDLK_LCTRL: return KEY_LCTRL;
    case SDLK_LSHIFT: return KEY_LSHIFT;
    case SDLK_LALT: return KEY_LALT;
    //case SDLK_LSUPER: return KEY_LSUPER;
    case SDLK_RCTRL: return KEY_RCTRL;
    case SDLK_RSHIFT: return KEY_RSHIFT;
    case SDLK_RALT: return KEY_RALT;
    //case SDLK_RSUPER: return KEY_RSUPER;
    default: return KEY_UNKNOWN;
  }
}

static void poll_events (void (*h)(int key, int down)) {
  int key;
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    switch (ev.type) {
      case SDL_QUIT:
        ERR_quit();
        break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        key = sdl_to_key(ev.key.keysym.sym);
        I_press(key, ev.type == SDL_KEYDOWN);
        if (h != NULL) {
          (*h)(key, ev.type == SDL_KEYDOWN);
        }
        break;
    }
  }
}

int main (int argc, char **argv) {
  char *pw;
  Uint32 t, ticks;
  logo("system: initialize SDL2\n");
  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS) == -1) {
    logo("system: failed to init SDL2: %s\n", SDL_GetError());
    return 1;
  }
  //SDL_WM_SetCaption("Doom 2D v1.351", "Doom 2D");
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
  while (!quit) {
    poll_events(&G_keyf);
    S_updatemusic();
    t = SDL_GetTicks();
    if (t - ticks > DELAY) {
      ticks = t;
      G_act();
    }
    R_draw();
  }
  CFG_save();
  R_done();
  S_donemusic();
  S_done();
  M_shutdown();
  SDL_Quit();
  return 0;
}
