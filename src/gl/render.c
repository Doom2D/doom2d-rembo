#include "glob.h"
#include "render.h"
#include "files.h"
#include "memory.h"
#include "misc.h"
#include "error.h"

#include "menu.h"
#include "game.h"
#include "dots.h"
#include "items.h"

#include "sound.h" // snd_vol
#include "music.h" // mus_vol

#include "fx.h"
#include "player.h"
#include "monster.h"
#include "weapons.h"
#include "smoke.h"
#include "view.h"
#include "switch.h" // sw_secrets

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif
#include <stdlib.h> // malloc free abs
#include <assert.h> // assert
#include <SDL.h>

#define VGA_TRANSPARENT_COLOR 0
#define DEFAULT_SKY_COLOR 0x97
#define MANCOLOR 0xD0
#define PLAYER_COLOR_OFFSET 7
#define MAXAIR 1091
#define ANIT 5
#define PL_FLASH 90

#pragma pack(1)
typedef struct vgaimg {
  word w, h;
  short x, y;
  byte data[];
} vgaimg;

typedef struct rgb {
  byte r, g, b;
} rgb;

typedef struct rgba {
  byte r, g, b, a;
} rgba;
#pragma pack()

typedef struct node {
  struct cache *base;
  struct node *left, *right;
  struct node *up;
  int l, t, r, b;
  int leaf;
} node;

typedef struct cache {
  struct cache *next;
  struct node root;
  GLuint id;
} cache;

typedef struct image {
  node *n;
  GLint x, y;
  GLuint w, h;
  int res;
} image;

/* Render Specific */
int SCRW = 320; // public
int SCRH = 200; // public
static int gamma;
static int fullscreen;
static SDL_Surface *surf;
static rgb playpal[256];
static byte bright[256];
static GLuint lastTexture;
static cache *root;

/* Game */
static image scrnh[3]; // TITLEPIC INTERPIC ENDPIC
static image ltn[2][2];

/* Smoke */
static image smk_spr[SMSN];
static image smk_fspr[FLSN];

/* Effects */
static image fx_spr[15];
static char fx_sprd[15];

/* Weapons */
static image wp_spr[49*2];
static char wp_sprd[49*2];

/* Items */
static image item_spr[58];
static char item_sprd[58];

/* Player */
static image plr_spr[27*2];
static image plr_msk[27*2];
static char plr_sprd[27*2];
static image plr_wpn[11][6];

/* Monsters */
static image pl_spr[2];
static image pl_msk[2];
static image mn_spr[MN_TN][29*2];
static image mn_man_msk[29*2];
static char mn_sprd[MN_TN][29*2];
static image mn_fspr[8];
static image mn_sgun[2];

/* Misc */
static image sth[22];
static image bfh[160 - '!'];
static image sfh[160 - '!'];
static image stone;
static image stone2;
static image keys[3];
static int prx = 0;
static int pry = 0;

/* Menu */
static int gm_tm;
static image msklh[2];
static image mbarl;
static image mbarm;
static image mbarr;
static image mbaro;
static image mslotl;
static image mslotm;
static image mslotr;

/* Map */
static const char *anm[ANIT - 1][5] = {
  {"WALL22_1", "WALL23_1", "WALL23_2", NULL,    NULL},
  {"WALL58_1", "WALL58_2", "WALL58_3", NULL,    NULL},
  {"W73A_1",   "W73A_2",   NULL,       NULL,    NULL},
  {"RP2_1",    "RP2_2",    "RP2_3",    "RP2_4", NULL}
};
static int max_wall_width;
static int max_wall_height;
static int max_textures;
static image walp[256];
static byte walani[256];
static image anip[ANIT][5];
static byte anic[ANIT];
static image horiz;

/* Texture cache */

// https://blackpawn.com/texts/lightmaps/
static node *R_node_alloc (node *p, int w, int h) {
  assert(p);
  assert(w > 0);
  assert(h > 0);
  if (p->left) {
    assert(p->right);
    node *n = R_node_alloc(p->left, w, h);
    return n ? n : R_node_alloc(p->right, w, h);
  } else {
    int pw = p->r - p->l + 1;
    int ph = p->b - p->t + 1;
    if (p->leaf || pw < w || ph < h) {
      return NULL;
    } else if (pw == w && ph == h) {
      p->leaf = 1;
      return p;
    } else {
      p->left = malloc(sizeof(node));
      p->right = malloc(sizeof(node));
      if (pw - w > ph - h) {
        *p->left = (node) {
          .up = p,
          .l = p->l,
          .t = p->t,
          .r = p->l + w - 1,
          .b = p->b
        };
        *p->right = (node) {
          .up = p,
          .l = p->l + w,
          .t = p->t,
          .r = p->r,
          .b = p->b
        };
      } else {
        *p->left = (node) {
          .up = p,
          .l = p->l,
          .t = p->t,
          .r = p->r,
          .b = p->t + h - 1
        };
        *p->right = (node) {
          .up = p,
          .l = p->l,
          .t = p->t + h,
          .r = p->r,
          .b = p->b
        };
      }
      return R_node_alloc(p->left, w, h);
    }
  }
}

static int R_node_have_leaf (node *n) {
  return n && (n->leaf || R_node_have_leaf(n->left) || R_node_have_leaf(n->right));
}

static void R_node_free_recursive (node *n) {
  if (n) {
    R_node_free_recursive(n->left);
    R_node_free_recursive(n->right);
    free(n);
  }
}

static void R_node_free (node *n) {
  if (n) {
    //logo("free node %p {%i:%i:%i:%i}\n", n, n->l, n->t, n->r, n->b);
    assert(n->leaf);
    assert(n->left == NULL);
    assert(n->right == NULL);
    n->leaf = 0;
    n->base = NULL;
    node *p = n->up;
    while (p != NULL) {
      assert(p->leaf == 0);
      assert(p->left);
      assert(p->right);
      if (R_node_have_leaf(p) == 0) {
        R_node_free_recursive(p->left);
        p->left = NULL;
        R_node_free_recursive(p->right);
        p->right = NULL;
        p = p->up;
      } else {
        p = NULL;
      }
    }
  }
}

static void R_gl_bind_texture (GLuint id) {
  if (id != lastTexture) {
    glBindTexture(GL_TEXTURE_2D, id);
  }
}

static cache *R_cache_new (void) {
  GLuint id = 0;
  GLint size = 0;
  cache *c = NULL;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
  size = size < 512 ? size : 512; // more can be buggy on older hardware
  if (size) {
    glGenTextures(1, &id);
    if (id) {
      R_gl_bind_texture(id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      c = malloc(sizeof(cache));
      if (c != NULL) {
        *c = (cache) {
          .id = id,
          .root.r = size - 1,
          .root.b = size - 1
        };
      } else {
        glDeleteTextures(1, &id);
      }
    }
  }
  //logo("new cache %p\n", c);
  return c;
}

static void R_cache_free (cache *root, int freetexture) {
  cache *next;
  cache *c = root;
  while (c != NULL) {
    next = c->next;
    R_node_free_recursive(c->root.left);
    R_node_free_recursive(c->root.right);
    if (freetexture && c->id != 0) {
      glDeleteTextures(1, &c->id);
    }
    free(c);
    c = next;
  }
}

static node *R_cache_alloc (cache *root, int w, int h) {
  assert(root);
  assert(w > 0 && h > 0);
  node *n = NULL;
  cache *p = NULL;
  cache *c = root;
  // TODO return null if required size greater than maximum
  while (c && !n) {
    n = R_node_alloc(&c->root, w, h);
    if (n) {
      assert(n->leaf);
      n->base = c;
    }
    p = c;
    c = c->next;
  }
  if (!n) {
    c = R_cache_new();
    if (c) {
      p->next = c;
      n = R_node_alloc(&c->root, w, h);
      if (n) {
        assert(n->leaf);
        n->base = c;
      }
    }
  }
  if (n) {
    //logo("new node %p {%i:%i:%i:%i}\n", n, n->l, n->t, n->r, n->b);
  } else {
    logo("new node failed\n");
  }
  return n;
}

static void R_cache_update (node *n, const void *data, int w, int h) {
  assert(n);
  assert(n->leaf);
  assert(n->base);
  assert(data);
  int nw = n->r - n->l + 1;
  int nh = n->b - n->t + 1;
  assert(w == nw);
  assert(h == nh);
  R_gl_bind_texture(n->base->id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, n->l, n->t, nw, nh, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

/* Generic helpers */

static void R_init_playpal (void) {
  int i;
  byte *vgapal = M_lock(F_getresid("PLAYPAL"));
  for (i = 0; i < 256; i++) {
    playpal[i] = (rgb) {
      .r = vgapal[i * 3 + 0] * 255 / 63,
      .g = vgapal[i * 3 + 1] * 255 / 63,
      .b = vgapal[i * 3 + 2] * 255 / 63,
    };
    bright[i] = ((int)vgapal[i * 3 + 0] + vgapal[i * 3 + 1] + vgapal[i * 3 + 2]) * 8 / (63 * 3);
  }
  M_unlock(vgapal);
}

static vgaimg *R_getvga (int id) {
  int loaded = M_was_locked(id);
  vgaimg *v = M_lock(id);
  if (v != NULL && !loaded) {
    v->w = short2host(v->w);
    v->h = short2host(v->h);
    v->x = short2host(v->x);
    v->y = short2host(v->y);
  }
  return v;
}

static rgba *R_extract_flame_spr (vgaimg *v) {
  static const byte flametab[16] = {
    0xBC, 0xBA, 0xB8, 0xB6, 0xB4, 0xB2, 0xB0, 0xD5,
    0xD6, 0xD7, 0xA1, 0xA0, 0xE3, 0xE2, 0xE1, 0xE0
  };
  int i, j;
  rgba *s = malloc(v->w * v->h * sizeof(rgba));
  if (s != NULL) {
    for (j = 0; j < v->h; j++) {
      for (i = 0; i < v->w; i++) {
        int k = j * v->w + i;
        byte c = v->data[k] + bright[DEFAULT_SKY_COLOR];
        s[k] = (rgba) {
          .r = playpal[flametab[c]].r,
          .g = playpal[flametab[c]].g,
          .b = playpal[flametab[c]].b,
          .a = v->data[k] == VGA_TRANSPARENT_COLOR ? 0x00 : 0xFF,
        };
      }
    }
  }
  return s;
}

static rgba *R_extract_smoke_spr (vgaimg *v) {
  int i, j;
  rgba *s = malloc(v->w * v->h * sizeof(rgba));
  if (s != NULL) {
    for (j = 0; j < v->h; j++) {
      for (i = 0; i < v->w; i++) {
        int k = j * v->w + i;
        byte c = ((v->data[k] + bright[DEFAULT_SKY_COLOR]) + 0x60) ^ 0x0F;
        byte a = 0xFF - ((int)playpal[c].r + playpal[c].g + playpal[c].b) / 3;
        s[k] = (rgba) {
          .r = playpal[c].r,
          .g = playpal[c].g,
          .b = playpal[c].b,
          .a = v->data[k] == VGA_TRANSPARENT_COLOR ? 0x00 : a,
        };
      }
    }
  }
  return s;
}

static rgba *R_extract_mask_spr (vgaimg *v) {
  int i, j;
  rgba *s = malloc(v->w * v->h * sizeof(rgba));
  if (s != NULL) {
    for (j = 0; j < v->h; j++) {
      for (i = 0; i < v->w; i++) {
        int k = j * v->w + i;
        byte c = v->data[k];
        if (c >= 0x70 && c <= 0x7F) {
          byte mask = c - 0x70;
          mask = 0xFF - ((mask << 4) | mask);
          s[k] = (rgba) {
            .r = mask,
            .g = mask,
            .b = mask,
            .a = 0xFF,
          };
        } else {
          s[k] = (rgba) {
            .r = 0,
            .g = 0,
            .b = 0,
            .a = 0,
          };
        }
      }
    }
  }
  return s;
}

static rgba *R_extract_rgba_spr (vgaimg *v) {
  int i, j;
  rgba *s = malloc(v->w * v->h * sizeof(rgba));
  if (s != NULL) {
    for (j = 0; j < v->h; j++) {
      for (i = 0; i < v->w; i++) {
        int k = j * v->w + i;
        byte c = v->data[k];
        s[k] = (rgba) {
          .r = playpal[c].r,
          .g = playpal[c].g,
          .b = playpal[c].b,
          .a = c == VGA_TRANSPARENT_COLOR ? 0x00 : 0xFF,
        };
      }
    }
  }
  return s;
}

/* OpenGL helpers */

static image R_gl_create_image (const rgba *buf, int w, int h) {
  node *n = R_cache_alloc(root, w, h);
  if (n) {
    R_cache_update(n, buf, w, h);
  }
  return (image) {
    .n = n,
    .w = w,
    .h = h,
    .res = -1
  };
}

static image R_gl_get_special_image (int id, rgba *(*fn)(vgaimg*)) {
  image img;
  vgaimg *v = R_getvga(id);
  if (v != NULL) {
    rgba *buf = (*fn)(v);
    img = R_gl_create_image(buf, v->w, v->h);
    img.x = v->x;
    img.y = v->y;
    img.res = id;
    M_unlock(v);
    free(buf);
  } else {
    img = (image) {
      .res = id
    };
  }
  return img;
}

static image R_gl_getimage (int id) {
  return R_gl_get_special_image(id, &R_extract_rgba_spr);
}

static image R_gl_loadimage (const char name[8]) {
  return R_gl_getimage(F_getresid(name));
}

static image R_gl_get_special_spr (const char n[4], int s, int d, rgba *(*fn)(vgaimg*)) {
  return R_gl_get_special_image(F_getsprid(n, s, d), fn);
}

static void R_gl_free_image (image *img) {
  if (img->n != NULL && img->res >= 0) {
    R_node_free(img->n);
  }
  img->n = NULL;
  img->res = -1;
}

static void R_gl_draw_quad (int x, int y, int w, int h) {
  glBegin(GL_QUADS);
  glVertex2i(x + w, y);
  glVertex2i(x,     y);
  glVertex2i(x,     y + h);
  glVertex2i(x + w, y + h);
  glEnd();
}

static void R_gl_draw_textured (image *img, int x, int y, int w, int h, int flip) {
  if (img->n) {
    GLfloat nw = img->n->base->root.r + 1;
    GLfloat nh = img->n->base->root.b + 1;
    GLfloat ax = (flip ? img->n->l : img->n->r + 1) / nw;
    GLfloat bx = (flip ? img->n->r + 1 : img->n->l) / nh;
    GLfloat ay = (img->n->t) / nw;
    GLfloat by = (img->n->b + 1) / nh;
    R_gl_bind_texture(img->n->base->id);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(ax, ay); glVertex2i(x + w, y);
    glTexCoord2f(bx, ay); glVertex2i(x,     y);
    glTexCoord2f(bx, by); glVertex2i(x,     y + h);
    glTexCoord2f(ax, by); glVertex2i(x + w, y + h);
    glEnd();
  } else {
    glColor3ub(255, 0, 0);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    R_gl_draw_quad(x, y, w, h);
  }
}

/* fit image into rectangle without applying offset and transparency */
static void R_gl_draw_image_ext (image *img, int x, int y, int w, int h) {
  glDisable(GL_BLEND);
  glColor3ub(255, 255, 255);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  R_gl_draw_textured(img, x, y, w, h, 0);
}

/* draw sprite with offset and coloring */
static void R_gl_draw_image_color (image *img, int x, int y, int flip) {
  int xx = flip ? x - img->w + img->x : x - img->x;
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  R_gl_draw_textured(img, xx, y - img->y, img->w, img->h, flip);
}

/* draw sprite with offset */
static void R_gl_draw_image (image *img, int x, int y, int flip) {
  glColor3ub(255, 255, 255);
  R_gl_draw_image_color(img, x, y, flip);
}

static void R_gl_set_color (byte c) {
  glColor3ub(playpal[c].r, playpal[c].g, playpal[c].b);
}

static void R_gl_setclip (int x, int y, int w, int h) {
  glScissor(x, SCRH - h - y, w, h);
}

static void R_gl_setmatrix (void) {
  glScissor(0, 0, SCRW, SCRH);
  glViewport(0, 0, SCRW, SCRH);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, SCRW, SCRH, 0, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/* --- Misc --- */

static image Z_getspr (const char n[4], int s, int d, char *dir) {
  int h = F_getsprid(n, s, d);
  if (dir != NULL) {
    *dir = (h & 0x8000) ? 1 : 0;
  }
  return R_gl_getimage(h);
}

static void Z_putch_generic (image img[], int off, int ch) {
  image *p = NULL;
  if (ch > 32 && ch < 160) {
    p = &img[ch - '!'];
  }
  if (p != NULL) {
    R_gl_draw_image(p, prx, pry, 0);
    prx += p->w - 1;
  } else {
    prx += off;
  }
}

static void Z_printf_generic (image img[], int off, const char *fmt, va_list ap) {
  int i;
  char buf[80];
  vsprintf(buf, fmt, ap);
  for (i = 0; buf[i]; ++i) {
    switch (buf[i]) {
      case '\n':
        pry += off + 1;
      case '\r':
        prx = 0;
        break;
      default:
        Z_putch_generic(img, off, (byte)buf[i]);
    }
  }
}

static void Z_gotoxy (int x, int y) {
  prx = x;
  pry = y;
}

static void Z_printbf (const char *fmt, ...) {
  va_list a;
  va_start(a, fmt);
  Z_printf_generic(bfh, 12, fmt, a);
  va_end(a);
}

static void Z_printsf (const char *fmt, ...) {
  va_list a;
  va_start(a, fmt);
  Z_printf_generic(sfh, 7, fmt, a);
  va_end(a);
}

static void Z_printhf (const char *fmt, ...) {
  int i, c;
  char buf[80];
  va_list a;
  va_start(a, fmt);
  vsprintf(buf, fmt, a);
  va_end(a);
  for (i = 0; buf[i]; ++i) {
    switch (buf[i]) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        c = buf[i] - '0';
        break;
      case '-':
        c = 10;
        break;
      case '%':
        c = 11;
        break;
      case '\n':
        pry += 19;
      case '\r':
        c = -1;
        prx = 0;
        break;
      default:
        c = -1;
        break;
    }
    if (c >= 0) {
      R_gl_draw_image(&sth[c], prx, pry, 0);
    }
    prx += 14;
  }
}

/* --- Menu --- */

static image *PL_getspr (int s, int d, int msk) {
  int i = (s - 'A') * 2 + d;
  return msk ? &plr_msk[i] : &plr_spr[i];
}

static void GM_draw (void) {
  enum {MENU, MSG}; // copypasted from menu.c!
  enum {
    CANCEL, NEWGAME, LOADGAME, SAVEGAME, OPTIONS, QUITGAME, QUIT, ENDGAME, ENDGM,
    PLR1, PLR2, COOP, DM, VOLUME, GAMMA, LOAD, SAVE, PLCOLOR, PLCEND, MUSIC, INTERP,
    SVOLM, SVOLP, MVOLM, MVOLP, GAMMAM, GAMMAP, PL1CM, PL1CP, PL2CM, PL2CP
  }; // copypasted from menu.c!
  int i, j, k, x, y, cx, cy;
  image *img;
  gm_tm += 1;
  if (mnu != NULL) {
    cx = SCRW / 2;
    cy = SCRH / 2;
    if (mnu->type == MENU) {
      y = cy - (mnu->n * 16 - 20) / 2;
      Z_gotoxy(cx - mnu->x, y - 10); Z_printbf("%s", mnu->ttl);
      for (i = 0; i < mnu->n; i++) {
        if (mnu->t[i] == LOAD || mnu->t[i] == SAVE) {
          j = y + i * 16 + 29;
          R_gl_draw_image(&mslotl, cx - mnu->x, j, 0);
          for (k = 8; k < 184; k += 8) {
            R_gl_draw_image(&mslotm, cx - mnu->x + k, j, 0);
          }
          R_gl_draw_image(&mslotr, cx - mnu->x + 184, j, 0);
          Z_gotoxy(cx - mnu->x + 4, j - 8);
          if (input && i == save_mnu.cur) {
            Z_printsf("%s_", ibuf);
          } else {
            Z_printsf("%s", savname[i]);
          }
        } else {
          x = mnu->t[i] >= SVOLM ? (mnu->t[i] >= PL1CM ? 50 : 152) : 0;
          Z_gotoxy(cx - mnu->x + x, y + i * 16 + 20);
          Z_printbf("%s", mnu->m[i]);
          switch (mnu->t[i]) {
            case MUSIC:
              Z_printbf(" '%s'", g_music);
              break;
            case INTERP:
              Z_printbf("%s", fullscreen ? "ON" : "OFF");
              break;
            case PL1CM:
            case PL1CP:
            case PL2CM:
            case PL2CP:
              img = PL_getspr(*panimp, 0, 0);
              R_gl_draw_image(img, cx - mnu->x + (mnu->t[i] == PL1CM ? 15 : 35), y + i * 16 + 20 + 14, 0);
              img = PL_getspr(*panimp, 0, 1);
              R_gl_set_color(pcolortab[(mnu->t[i] == PL1CM) ? p1color : p2color] + PLAYER_COLOR_OFFSET);
              R_gl_draw_image_color(img, cx - mnu->x + (mnu->t[i] == PL1CM ? 15 : 35), y + i * 16 + 20 + 14, 0);
              break;
            case SVOLM:
            case SVOLP:
            case MVOLM:
            case MVOLP:
            case GAMMAM:
            case GAMMAP:
              j = y + i * 16 + 20;
              R_gl_draw_image(&mbarl, cx - mnu->x, j, 0);
              for (k = 8; k < 144; k += 8) {
                R_gl_draw_image(&mbarm, cx - mnu->x + k, j, 0);
              }
              R_gl_draw_image(&mbarr, cx - mnu->x + 144, j, 0);
              switch (mnu->t[i]) {
                case SVOLM:
                  k = snd_vol;
                  break;
                case MVOLM:
                  k = mus_vol;
                  break;
                case GAMMAM:
                  k = gamma << 5;
                  break;
                default:
                  k = 0;
                  break;
              }
              R_gl_draw_image(&mbaro, cx - mnu->x + 8 + k, j, 0);
              break;
          }
        }
      }
      R_gl_draw_image(&msklh[(gm_tm / 6) & 1], cx - mnu->x - 25, y + mnu->cur * 16 + 20 - 8, 0);
    } else if (mnu->type == MSG) {
      Z_gotoxy(cx - strlen(mnu->ttl) * 7 / 2, cy - 10); Z_printsf(mnu->ttl);
      Z_gotoxy(cx - 24, SCRH / 2); Z_printsf("(Y/N)");
    } else {
      ERR_fatal("Unknown menu type %i\n", mnu->type);
    }
  }
}

/* --- View --- */

static void R_draw_fld (byte *fld, int minx, int miny, int maxx, int maxy, int fg) {
  int i, j;
  assert(minx >= 0 && minx <= FLDW);
  assert(miny >= 0 && miny <= FLDH);
  assert(maxx >= 0 && maxx <= FLDW);
  assert(maxy >= 0 && maxy <= FLDH);
  for (j = miny; j < maxy; j++) {
    for (i = minx; i < maxx; i++) {
      byte id = fld[j * FLDW + i];
      if (id != 0) {
        if (walp[id].res < 0) {
          if (fg) {
            switch (R_get_special_id(id)) {
              case 1:
                glColor4ub(0, 0, 255, 127);
                break;
              case 2:
                glColor4ub(0, 127, 0, 127);
                break;
              case 3:
                glColor4ub(127, 0, 0, 127);
                break;
              default:
                glColor4ub(0, 0, 0, 127);
                break;
            }
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
            glDisable(GL_TEXTURE_2D);
            R_gl_draw_quad(i * CELW, j * CELW, CELW, CELH);
          }
        } else {
          R_gl_draw_image(&walp[id], i * CELW, j * CELH, 0);
        }
      }
    }
  }
}

static void R_draw_dots (void) {
  int i;
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_POINTS);
  for (i = 0; i < MAXDOT; i++) {
    if (dot[i].t != 0) {
      R_gl_set_color(dot[i].c);
      glVertex2i(dot[i].o.x, dot[i].o.y + 1);
    }
  }
  glEnd();
}

static void R_draw_items (void) {
  int i, s;
  for (i = 0; i < MAXITEM; ++i) {
    s = -1;
    if (it[i].t && it[i].s >= 0) {
      switch (it[i].t & 0x7FFF) {
        case I_ARM1:
          s = it[i].s / 9 + 18;
          break;
        case I_ARM2:
          s = it[i].s / 9 + 20;
          break;
        case I_MEGA:
          s = it[i].s / 2 + 22;
          break;
        case I_INVL:
          s = it[i].s / 2 + 26;
          break;
        case I_SUPER:
        case I_RTORCH:
        case I_GTORCH:
        case I_BTORCH:
          s = it[i].s / 2 + (it[i].t - I_SUPER) * 4 + 35;
          break;
        case I_GOR1: case I_FCAN:
          s = it[i].s / 2 + (it[i].t - I_GOR1) * 3 + 51;
          break;
        case I_AQUA:
          s = 30;
          break;
        case I_SUIT:
          s = 34;
          break;
        case I_KEYR:
        case I_KEYG:
        case I_KEYB:
          s = (it[i].t & 0x7FFF) - I_KEYR + 31;
          break;
        case I_GUN2:
          s = 57;
          break;
        default:
          s = (it[i].t & 0x7FFF) - 1;
      }
    }
    if (s >= 0) {
      R_gl_draw_image(&item_spr[s], it[i].o.x, it[i].o.y, item_sprd[s]);
    }
  }
}

static int standspr (player_t *p) {
  if (p->f & PLF_UP) {
    return 'X';
  } else if (p->f & PLF_DOWN) {
    return 'Z';
  } else {
    return 'E';
  }
}

static int wpnspr (player_t *p) {
  if (p->f & PLF_UP) {
    return 'C';
  } else if(p->f & PLF_DOWN) {
    return 'E';
  } else {
    return 'A';
  }
}

static void R_draw_player (player_t *p) {
  enum {STAND, GO, DIE, SLOP, DEAD, MESS, OUT, FALL}; // copypasted from player.c!
  static const int wytab[] = {-1, -2, -1, 0};
  int s = 'A';
  int w = 0;
  int wx = 0;
  int wy = 0;
  switch (p->st) {
    case STAND:
      if (p->f & PLF_FIRE) {
        s = standspr(p) + 1;
        w = wpnspr(p) + 1;
      } else if (p->pain) {
        s = 'G';
        w = 'A';
        wx = p->d ? 2 : -2;
        wy = 1;
      } else {
        s = standspr(p);
        w = wpnspr(p);
      }
      break;
    case DEAD:
      s = 'N';
      break;
    case MESS:
      s = 'W';
      break;
    case GO:
      if (p->pain) {
        s = 'G';
        w = 'A';
        wx = p->d ? 2 : -2;
        wy = 1;
      } else {
        s = plr_goanim[p->s / 8];
        w = (p->f & PLF_FIRE) ? 'B' : 'A';
        wx = p->d ? 2 : -2;
        wy = 1 + wytab[s - 'A'];
      }
      break;
    case DIE:
      s = plr_dieanim[p->s];
      break;
    case SLOP:
      s = plr_slopanim[p->s];
      break;
    case OUT:
      s = 0;
      break;
  }
  if (p->wpn == 0) {
    w = 0;
  }
  if (w) {
    R_gl_draw_image(&plr_wpn[(int)p->wpn][w -'A'], p->o.x + wx, p->o.y + wy, p->d);
  }
  if (s) {
    R_gl_draw_image(&plr_spr[(s - 'A') * 2 + p->d], p->o.x, p->o.y, plr_sprd[(s - 'A') * 2 + p->d]);
    R_gl_set_color(p->color + PLAYER_COLOR_OFFSET);
    R_gl_draw_image_color(&plr_msk[(s - 'A') * 2 + p->d], p->o.x, p->o.y, plr_sprd[(s - 'A') * 2 + p->d]);
  }
}

static void R_draw_monsters (void) {
  enum {SLEEP, GO, RUN, CLIMB, DIE, DEAD, ATTACK, SHOOT, PAIN, WAIT, REVIVE, RUNOUT}; // copypasted from monster.c!
  int i;
  for (i = 0; i < MAXMN; i++) {
    if (mn[i].t != MN_NONE) {
      int x = mn[i].o.x;
      int y = mn[i].o.y;
      if (mn[i].t < MN__LAST) {
        if ((mn[i].t != MN_SOUL && mn[i].t != MN_PAIN) || mn[i].st != DEAD) {
          int ap = mn[i].ap[mn[i].ac];
          int d = (ap - 'A') * 2 + mn[i].d;
          int dir = mn_sprd[mn[i].t - 1][d];
          if (mn[i].t == MN_MAN && (ap == 'E' || ap == 'F')) {
            R_gl_draw_image(&mn_sgun[ap - 'E'], x, y, mn[i].d);
          }
          R_gl_draw_image(&mn_spr[mn[i].t - 1][d], x, y, dir);
          if (mn[i].t == MN_MAN) {
            R_gl_set_color(MANCOLOR + PLAYER_COLOR_OFFSET);
            R_gl_draw_image_color(&mn_man_msk[d], x, y, dir);
          }
        }
        if (mn[i].t == MN_VILE && mn[i].st == SHOOT) {
          R_gl_draw_image(&mn_fspr[mn[i].ac / 3], mn[i].tx, mn[i].ty, 0);
        }
      } else if (mn[i].t == MN_PL_DEAD || mn[i].t == MN_PL_MESS) {
        int type = mn[i].t - MN_PL_DEAD;
        R_gl_draw_image(&pl_spr[type], x, y, 0);
        R_gl_set_color(mn[i].d);
        R_gl_draw_image_color(&pl_msk[type], x, y, 0);
      }
    }
  }
}

static void R_draw_weapons (void) {
  enum {NONE, ROCKET, PLASMA, APLASMA, BALL1, BALL2, BALL7, BFGBALL, BFGHIT, MANF, REVF, FIRE}; // copypasted from weapons.c!
  int i, s, d, x, y;
  for (i = 0; i < MAXWPN; ++i) {
    s = -1;
    d = 0;
    switch (wp[i].t) {
      case REVF:
      case ROCKET:
        d = wp[i].s;
        if (d < 2) {
          d = wp[i].o.xv > 0 ? 1 : 0;
          x = abs(wp[i].o.xv);
          y = wp[i].o.yv;
          s = 0;
          if (y < 0) {
            if (-y >= x) {
              s = 30;
            }
          } else if (y > 0) {
            if (y >= x / 2) {
              s = 31;
            }
          }
        } else {
          s = (d - 2) / 2 + 1;
          d = 0;
        }
        break;
      case MANF:
        s=wp[i].s;
        if (s >= 2) {
          s /= 2;
          break;
        }
      case PLASMA:
      case APLASMA:
      case BALL1:
      case BALL7:
      case BALL2:
        s = wp[i].s;
        if (s >= 2) {
          s = s / 2 + 1;
        }
        switch (wp[i].t) {
          case PLASMA:
            s += 4;
            break;
          case APLASMA:
            s += 11;
            break;
          case BALL1:
            s += 32;
            break;
          case BALL2:
            s += 42;
            break;
          case BALL7:
            s += 37;
            d = wp[i].o.xv >= 0 ? 1 : 0;
            break;
          case MANF:
            s += 47;
            d= wp[i].o.xv>=0 ? 1 : 0;
            break;
        }
        break;
      case BFGBALL:
        s = wp[i].s;
        if (s >= 2) {
          s = s / 2 + 1;
        }
        s += 18;
        break;
      case BFGHIT:
        s = wp[i].s / 2 + 26;
        break;
    }
    if (s >= 0) {
      R_gl_draw_image(&wp_spr[s * 2 + d], wp[i].o.x, wp[i].o.y, wp_sprd[s * 2 + d]);
    }
  }
}

static void R_draw_smoke (void) {
  int i, s;
  for (i = 0; i < MAXSMOK; ++i) {
    if (sm[i].t) {
      switch (sm[i].s) {
        case 0:
          s = sm[i].t;
          if (s >= (SMSN - 1) * 3) {
            s = 0;
          } else {
            s = SMSN - 1 - s / 3;
          }
          R_gl_draw_image(&smk_spr[s], sm[i].x >> 8, (sm[i].y >> 8) + 1, 0);
          break;
        case 1:
          s = sm[i].t;
          if (s >= FLSN - 1) {
            s = 0;
          } else {
            s = FLSN - 1 - s;
          }
          R_gl_draw_image(&smk_fspr[s], sm[i].x >> 8, (sm[i].y >> 8) + 1, 0);
          break;
      }
    }
  }
}

static void R_draw_effects (void) {
  enum {NONE, TFOG, IFOG, BUBL}; // copypasted from fx.c
  int i, s;
  for (i = 0; i < MAXFX; ++i) {
    switch (fx[i].t) {
      case TFOG:
        s = fx[i].s / 2;
        R_gl_draw_image(&fx_spr[s], fx[i].x, fx[i].y, fx_sprd[s]);
        break;
      case IFOG:
        s = fx[i].s / 2 + 10;
        R_gl_draw_image(&fx_spr[s], fx[i].x, fx[i].y, fx_sprd[s]);
        break;
      case BUBL:
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_POINTS);
        R_gl_set_color(0xC0 + fx[i].s);
        glVertex2i(fx[i].x >> 8, (fx[i].y >> 8) + 1);
        glEnd();
        break;
    }
  }
}

static int get_pu_st (int t) {
  if (t >= PL_FLASH) {
    return 1;
  } else if((t / 9) & 1) {
    return 0;
  } else {
    return 1;
  }
}

static void R_draw_view (int x, int y, int w, int h, int camx, int camy) {
  glPushMatrix();
  R_gl_setclip(x, y, w, h);
  glTranslatef(x, y, 0);
  if (w_horiz && horiz.n != NULL) {
    R_gl_draw_image_ext(&horiz, 0, 0, w, h);
    if (sky_type == 2 && lt_time < 0) {
      image *tanderbolt = &ltn[lt_type][lt_time < -5 ? 0 : 1];
      if (!lt_side) {
        R_gl_draw_image(tanderbolt, 0, lt_ypos, 0);
      } else {
        R_gl_draw_image(tanderbolt, w - 1, lt_ypos, 1);
      }
    }
  } else {
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    R_gl_set_color(DEFAULT_SKY_COLOR);
    R_gl_draw_quad(0, 0, w, h);
  }
  int maxx = min((camx + w) / CELW + 1, FLDW);
  int maxy = min((camy + h) / CELH + 1, FLDH);
  int minx = max((camx - max_wall_width) / CELW, 0);
  int miny = max((camy - max_wall_height) / CELH, 0);
  glTranslatef(-camx, -camy, 0);
  R_draw_fld((byte*)fldb, minx, miny, maxx, maxy, 0);
  R_draw_dots();
  R_draw_items();
  R_draw_player(&pl1);
  if (_2pl) {
    R_draw_player(&pl2);
  }
  R_draw_monsters();
  R_draw_weapons();
  R_draw_smoke();
  R_draw_effects();
  R_draw_fld((byte*)fldf, minx, miny, maxx, maxy, 1);
  glTranslatef(camx, camy, 0);
  if (sky_type == 2 && (lt_time == -4 || lt_time == -2)) {
    glColor4ub(255, 255, 255, 255);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
    glDisable(GL_TEXTURE_2D);
    R_gl_draw_quad(0, 0, w, h);
  }
  glPopMatrix();
}

static void R_draw_player_view (player_t *p, int x, int y, int w, int h) {
  p->looky = min(max(p->looky, -SCRH / 4), SCRH / 4); // TODO remove writeback
  int st = stone.w;
  int cw = w - st;
  int cx = min(max(p->o.x, cw / 2), FLDW * CELW - cw / 2);
  int cy = min(max(p->o.y - 12 + p->looky, h / 2), FLDH * CELH - h / 2);
  int camx = max(cx - cw / 2, 0);
  int camy = max(cy - h / 2, 0);
  glPushMatrix();
  R_draw_view(x, y + 1, cw, h - 2, camx, camy);
  glTranslatef(x, y, 0);
  if (p->invl) {
    if (get_pu_st(p->invl)) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
      glDisable(GL_TEXTURE_2D);
      glColor4ub(191, 191, 191, 255);
      R_gl_draw_quad(0, 0, cw, h);
    }
  } else {
    if (p->suit && get_pu_st(p->suit)) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_TEXTURE_2D);
      glColor4ub(0, 255, 0, 192);
      R_gl_draw_quad(0, 0, cw, h);
    }
    int f = min(max(p->pain * 3, 0), 255);
    if (f > 0) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_TEXTURE_2D);
      glColor4ub(255, 0, 0, f);
      R_gl_draw_quad(0, 0, cw, h);
    }
  }
  R_gl_setclip(x, y, w, h);
  glTranslatef(-x + cw, 0, 0);
  R_gl_draw_image(&stone, 0, 0, 0);
  int i = stone.h;
  while (i < h) {
    R_gl_draw_image(&stone2, 0, i, 0);
    i += stone2.h;
  }
  if (p->drawst & PL_DRAWAIR) {
    if (p->air < PL_AIR) {
      int a = min(max(p->air, 0), MAXAIR) * 100 / MAXAIR;
      glDisable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_TEXTURE_2D);
      R_gl_set_color(0xC8);
      R_gl_draw_quad(10, 49, a, 2);
    }
  }
  if (p->drawst & PL_DRAWLIFE) {
    Z_gotoxy(10, 7);
    Z_printhf("%3d%%", p->life);
  }
  if (p->drawst & PL_DRAWARMOR) {
    Z_gotoxy(10, 7 + 19);
    Z_printhf("%3d%%", p->armor);
  }
  if (p->drawst & PL_DRAWWPN) {
    switch(p->wpn) {
      case 2:
      case 5:
        i = p->ammo;
        break;
      case 3:
      case 4:
      case 9:
        i = p->shel;
        break;
      case 6:
        i = p->rock;
        break;
      case 7:
      case 8:
        i = p->cell;
        break;
      case 10:
        i = p->fuel;
        break;
      default:
        i = -1;
        break;
    }
    // weapon
    if (p->wpn >= 0) {
      R_gl_draw_image(&sth[12 + p->wpn], st - 88, 58 + 19, 0);
    }
    // ammo
    if (p->wpn >= 2) {
      Z_gotoxy(st - 10 - 5 * 14, 58 + 2);
      Z_printhf("%5d", i);
    }
  }
  if (p->drawst & PL_DRAWFRAG && g_dm) {
    Z_gotoxy(st - 5 - 5 * 14, 77 + 5);
    Z_printhf("%5d", p->frag);
  }
  if (p->drawst & PL_DRAWKEYS) {
    int x, k, n;
    for (k = p->keys >> 4, n = 0, x = st - 75; n < 3; n++, k >>= 1, x += 9) {
      if (k & 1) {
        R_gl_draw_image(&keys[n], x, 91, 0);
      }
    }
  }
  if (p->drawst & PL_DRAWLIVES && !_2pl) {
    Z_gotoxy(st - 35, 17);
    Z_printhf("%d", p->lives);
  }
  glPopMatrix();
}

/* --- Game --- */

static void pl_info (player_t *p, int x, int y) {
  dword t = p->kills * 10920 / g_time;
  Z_gotoxy(x + 25, y); Z_printbf("KILLS");
  Z_gotoxy(x + 25, y + 15); Z_printbf("KPM");
  Z_gotoxy(x + 25, y + 30); Z_printbf("SECRETS %u / %u", p->secrets, sw_secrets);
  Z_gotoxy(x + 255, y); Z_printbf("%u", p->kills);
  Z_gotoxy(x + 255, y + 15); Z_printbf("%u.%u", t / 10, t % 10);
}

static void R_draw_intermission (void) {
  int cx = SCRW / 2;
  word hr, mn, sc, h;
  Z_gotoxy(cx - 14*12/2, 20);
  Z_printbf("LEVEL COMPLETE");
  Z_calc_time(g_time, &hr, &mn, &sc);
  Z_gotoxy(cx - 12*12/2, 40);
  Z_printbf("TIME %u:%02u:%02u", hr, mn, sc);
  h = 40 + SCRH / 10;
  if (_2pl) {
    Z_gotoxy(cx - 10*12/2, h);
    Z_printbf("PLAYER ONE");
    h += 20;
  }
  pl_info(&pl1, cx - 160, h);
  if (_2pl) {
    h += 30 + SCRH / 10;
    Z_gotoxy(cx - 10*12/2, h);
    Z_printbf("PLAYER TWO");
    h += 20;
    pl_info(&pl2, cx - 160, h);
  }
}

static void W_act (void) {
  int i, a;
  if (g_time % 3 == 0) {
    for (i = 1; i < max_textures; i++) {
      a = walani[i];
      if (a != 0) {
        anic[a]++;
        if (anip[a][anic[a]].res == -1) {
          anic[a] = 0;
        }
        walp[i] = anip[a][anic[a]];
      }
    }
  }
}

void R_draw (void) {
  W_act();
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_SCISSOR_TEST);
  R_gl_setmatrix();
  switch (g_st) {
    case GS_ENDANIM:
    case GS_END2ANIM:
    case GS_DARKEN:
    case GS_BVIDEO:
    case GS_EVIDEO:
    case GS_END3ANIM:
      break;
    case GS_TITLE:
      R_gl_draw_image_ext(&scrnh[0], 0, 0, SCRW, SCRH);
      break;
    case GS_INTER:
      R_gl_draw_image_ext(&scrnh[1], 0, 0, SCRW, SCRH);
      R_draw_intermission();
      break;
    case GS_ENDSCR:
      R_gl_draw_image_ext(&scrnh[2], 0, 0, SCRW, SCRH);
      break;
    case GS_GAME:
      if (_2pl) {
        R_draw_player_view(&pl1, 0, 0,        SCRW, SCRH / 2);
        R_draw_player_view(&pl2, 0, SCRH / 2, SCRW, SCRH / 2);
      } else {
        R_draw_player_view(&pl1, 0, 0, SCRW, SCRH);
      }
      R_gl_setclip(0, 0, SCRW, SCRH);
      break;
  }
  GM_draw();
  SDL_GL_SwapBuffers();
}

void R_alloc (void) {
  char s[10];
  int i, j, n;
  logo("R_alloc: load graphics\n");
  /* Game */
  scrnh[0] = R_gl_loadimage("TITLEPIC");
  assert(scrnh[0].n);
  scrnh[1] = R_gl_loadimage("INTERPIC");
  scrnh[2] = R_gl_loadimage("ENDPIC");
  for (i = 0; i < 2; i++) {
    sprintf(s, "LTN%c", '1' + i);
    for (j = 0; j < 2; j++) {
      ltn[i][j] = Z_getspr(s, j, 0, NULL);
    }
  }
  /* Smoke */
  for (i = 0; i < SMSN; i++) {
    smk_spr[i] = R_gl_get_special_spr("SMOK", i, 0, &R_extract_smoke_spr);
  }
  for (i = 0; i < FLSN; i++) {
    smk_fspr[i] = R_gl_get_special_spr("SMOK", i, 0, &R_extract_flame_spr);
  }
  /* Effects */
  for (i = 0; i < 10; i++) {
    fx_spr[i] = Z_getspr("TFOG", i, 0, fx_sprd + i);
  }
  for (; i < 15; i++) {
    fx_spr[i] = Z_getspr("IFOG", i - 10, 0, fx_sprd + i);
  }
  /* Weapons */
  for (i = 0; i < 4; i++) {
    wp_spr[i * 2] = Z_getspr("MISL", i, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("MISL", i, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 6; i++) {
    wp_spr[i * 2] = Z_getspr("PLSS", i - 4, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("PLSS", i - 4, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 11; i++) {
    wp_spr[i * 2] = Z_getspr("PLSE", i - 6, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("PLSE", i - 6, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 13; i++) {
    wp_spr[i * 2] = Z_getspr("APLS", i - 11, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("APLS", i - 11, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 18; i++) {
    wp_spr[i * 2] = Z_getspr("APBX", i - 13, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("APBX", i - 13, 2, wp_sprd + i * 2 + 1);
  }
  for(; i < 20; i++) {
    wp_spr[i * 2] = Z_getspr("BFS1", i - 18, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BFS1", i - 18, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 26; i++) {
    wp_spr[i * 2] = Z_getspr("BFE1", i - 20, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BFE1", i - 20, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 30; i++) {
    wp_spr[i * 2] = Z_getspr("BFE2", i - 26, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BFE2", i - 26, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 32; i++) {
    wp_spr[i * 2] = Z_getspr("MISL", i - 30 + 4, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("MISL", i - 30 + 4, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 37; i++) {
    wp_spr[i * 2] = Z_getspr("BAL1", i - 32, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BAL1", i - 32, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 42; i++) {
    wp_spr[i * 2] = Z_getspr("BAL7", i - 37, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BAL7", i - 37, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 47; i++) {
    wp_spr[i * 2] = Z_getspr("BAL2", i - 42, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("BAL2", i - 42, 2, wp_sprd + i * 2 + 1);
  }
  for (; i < 49; i++) {
    wp_spr[i * 2] = Z_getspr("MANF", i - 47, 1, wp_sprd + i * 2);
    wp_spr[i * 2 + 1] = Z_getspr("MANF", i - 47, 2, wp_sprd + i * 2 + 1);
  }
  /* Items */
  static const char snm[18][4] = {
    "CLIP", "SHEL", "ROCK", "CELL", "AMMO", "SBOX", "BROK", "CELP",
    "STIM", "MEDI", "BPAK",
    "CSAW", "SHOT", "SGN2", "MGUN", "LAUN", "PLAS", "BFUG"
  };
  static const char n4[4][4] = {
    "SOUL", "SMRT", "SMGT", "SMBT"
  };
  static const char n3[2][4] = {
    "GOR1", "FCAN"
  };
  for (i = 0; i < 18; i++) {
    item_spr[i] = Z_getspr(snm[i], 0, 0, item_sprd + i);
  }
  for (; i < 20; i++) {
    item_spr[i] = Z_getspr("ARM1", i - 18, 0, item_sprd + i);
    item_spr[i + 2] = Z_getspr("ARM2", i - 18, 0, item_sprd + i);
  }
  i+=2;
  for (; i < 26; i++) {
    item_spr[i] = Z_getspr("MEGA", i - 22, 0, item_sprd + i);
  }
  for (; i < 30; i++) {
    item_spr[i] = Z_getspr("PINV", i - 26, 0, item_sprd + i);
  }
  item_spr[30] = Z_getspr("AQUA", 0, 0, item_sprd + 30);
  item_spr[31] = Z_getspr("KEYR", 0, 0, item_sprd + 31);
  item_spr[32] = Z_getspr("KEYG", 0, 0, item_sprd + 32);
  item_spr[33] = Z_getspr("KEYB", 0, 0, item_sprd + 33);
  item_spr[34] = Z_getspr("SUIT", 0, 0, item_sprd + 34);
  for (n = 35, j = 0; j < 4; j++) {
    for (i = 0; i < 4; i++, n++) {
      item_spr[n] = Z_getspr(n4[j], i, 0, item_sprd + n);
    }
  }
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 3; i++, n++) {
      item_spr[n] = Z_getspr(n3[j], i, 0, item_sprd + n);
    }
  }
  item_spr[57] = Z_getspr("GUN2", 0, 0, item_sprd + 57);
  /* Player */
  for (i = 0; i < 27; i++) {
    plr_spr[i * 2] = Z_getspr("PLAY", i, 1, plr_sprd + i * 2);
    plr_msk[i * 2] = R_gl_get_special_spr("PLAY", i, 1, &R_extract_mask_spr);
    plr_spr[i * 2 + 1] = Z_getspr("PLAY", i, 2, plr_sprd + i * 2 + 1);
    plr_msk[i * 2 + 1] = R_gl_get_special_spr("PLAY", i, 2, &R_extract_mask_spr);
  }
  strncpy(s, "PWPx", 4);
  for (i = 1; i < 11; i++) {
    s[3] = (i < 10 ? '0' : 'A' - 10) + i;
    for (j = 0; j < 6; j++) {
      plr_wpn[i][j] = Z_getspr(s, j, 1, NULL);
    }
  }
  /* Monsters */
  static const char msn[MN_TN][4] = {
    "SARG", "TROO", "POSS", "SPOS", "CYBR", "CPOS", "BOSS", "BOS2", "HEAD", "SKUL",
    "PAIN", "SPID", "BSPI", "FATT", "SKEL", "VILE", "FISH", "BAR1", "ROBO", "PLAY"
  };
  static const int mms[MN_TN] = {
    14*2, 21*2, 21*2, 21*2, 16*2, 20*2, 15*2, 15*2, 12*2, 11*2,
    13*2, 19*2, 16*2, 20*2, 17*2, 29*2, 6*2, 2*2, 17*2, 23*2
  };
  mn_sgun[0] = Z_getspr("PWP4", 0, 1, NULL);
  mn_sgun[1] = Z_getspr("PWP4", 1, 1, NULL);
  for (j = 0; j < MN_TN; j++) {
    for (i = 0; i < mms[j]; i++) {
      mn_spr[j][i] = Z_getspr(msn[j], i / 2, (i & 1) + 1, &mn_sprd[j][i]);
      if (j == MN_MAN - 1) {
        mn_man_msk[i] = R_gl_get_special_spr(msn[j], i / 2, (i & 1) + 1, &R_extract_mask_spr);
      }
    }
    if (j == MN_BARREL - 1) {
      for (i = 4; i < 14; i++) {
        mn_spr[j][i] = Z_getspr("BEXP", i / 2 - 2, (i & 1) + 1, &mn_sprd[j][i]);
      }
    }
  }
  for (i = 0; i < 8; i++) {
    mn_fspr[i] = Z_getspr("FIRE", i, 0, NULL);
  }
  pl_spr[0] = Z_getspr("PLAY", 'N' - 'A', 0, NULL);
  pl_msk[0] = R_gl_get_special_spr("PLAY", 'N' - 'A', 0, &R_extract_mask_spr);
  pl_spr[1] = Z_getspr("PLAY", 'W' - 'A', 0, NULL);
  pl_msk[1] = R_gl_get_special_spr("PLAY", 'W' - 'A', 0, &R_extract_mask_spr);
  /* Misc */
  static const char mnm[22][8]={
    "STTNUM0", "STTNUM1", "STTNUM2", "STTNUM3", "STTNUM4",
    "STTNUM5", "STTNUM6", "STTNUM7", "STTNUM8", "STTNUM9",
    "STTMINUS", "STTPRCNT",
    "FISTA0", "CSAWA0", "PISTA0", "SHOTA0", "SGN2A0", "MGUNA0", "LAUNA0",
    "PLASA0", "BFUGA0", "GUN2A0"
  };
  stone = R_gl_loadimage("STONE");
  stone2 = R_gl_loadimage("STONE2");
  keys[0] = R_gl_loadimage("KEYRA0");
  keys[1] = R_gl_loadimage("KEYGA0");
  keys[2] = R_gl_loadimage("KEYBA0");
  for (i = 0; i < 22; i++) {
    sth[i] = R_gl_loadimage(mnm[i]);
  }
  strcpy(s, "STBF_*");
  for (i = '!'; i < 160; i++) {
    s[5] = i;
    bfh[i - '!'] = R_gl_getimage(F_findres(s));
  }
  for (i = '!'; i < 160; i++) {
    sprintf(s, "STCFN%03d", i);
    sfh[i - '!'] = R_gl_getimage(F_findres(s));
  }
  strcpy(s, "WINUM*");
  for (i = '0'; i <= '9'; i++) {
    s[5] = i;
    bfh[i - '!'] = R_gl_loadimage(s);
  }
  bfh[':' - '!'] = R_gl_loadimage("WICOLON");
  // menu
  msklh[0] = R_gl_loadimage("M_SKULL1");
  msklh[1] = R_gl_loadimage("M_SKULL2");
  mbarl = R_gl_loadimage("M_THERML");
  mbarm = R_gl_loadimage("M_THERMM");
  mbarr = R_gl_loadimage("M_THERMR");
  mbaro = R_gl_loadimage("M_THERMO");
  mslotl = R_gl_loadimage("M_LSLEFT");
  mslotm = R_gl_loadimage("M_LSCNTR");
  mslotr = R_gl_loadimage("M_LSRGHT");
  // walls
  for (i = 1; i < ANIT; i++) {
    for (j = 0; anm[i - 1][j]; j++) {
      anip[i][j] = R_gl_loadimage(anm[i - 1][j]);
    }
    for(; j < 5; j++) {
      anip[i][j] = (image) {
        .n = NULL,
        .w = 8,
        .h = 8,
        .res = -1,
      };
    }
  }
}

void R_init (void) {
  Uint32 flags = SDL_OPENGL;
  if (fullscreen) {
    flags = flags | SDL_FULLSCREEN;
  }
  if (SCRW <= 0 || SCRH <= 0) {
    ERR_failinit("Invalid screen size %ix%i\n", SCRW, SCRH);
  }
  if (surf == NULL) {
    R_init_playpal(); // only onece
  }
  surf = SDL_SetVideoMode(SCRW, SCRH, 0, flags);
  if (surf == NULL) {
    ERR_failinit("Unable to set video mode: %s\n", SDL_GetError());
  }
  root = R_cache_new();
  assert(root);
  R_alloc();
}

void R_done (void) {
  R_cache_free(root, 1);
}

void R_setgamma (int g) {
  gamma = g < 0 ? 0 : (g > 4 ? 4 : g);
}

int R_getgamma (void) {
  return gamma;
}

void R_toggle_fullscreen (void) {
  fullscreen = !fullscreen;
  if (surf) {
    R_init(); // recreate window
  }
}

void R_get_name (int n, char s[8]) {
  assert(n >= 0 && n < 256);
  if (walp[n].res == -1) {
    memset(s, 0, 8);
  } else if (walp[n].res == -2) {
    memcpy(s, "_WATER_", 8);
    s[7] = '0' + (intptr_t)walp[n].n - 1;
  } else if (walani[n] > 0) {
    memcpy(s, anm[walani[n] - 1][0], 8);
  } else {
    F_getresname(s, walp[n].res & 0x7FFF);
  }
}

static short getani (char n[8]) {
  short i = 0;
  while (i < ANIT - 1 && strncasecmp(n, anm[i][0], 8) != 0) {
    i++;
  }
  return i < ANIT - 1 ? i + 1 : 0;
}

int R_get_special_id (int n) {
  assert(n >= 0 && n <= 256);
  return walp[n].res == -2 ? (intptr_t)walp[n].n : -1;
}

void R_begin_load (void) {
  int i;
  for (i = 0; i < 256; i++) {
    if (walp[i].n != NULL && walp[i].res >= 0 && walani[i] == 0) {
      R_gl_free_image(&walp[i]);
    }
    memset(&walp[i], 0, sizeof(image));
    walp[i].res = -1;
    walswp[i] = i;
    walani[i] = 0;
  }
  memset(anic, 0, sizeof(anic));
  max_wall_width = 0;
  max_wall_height = 0;
  max_textures = 1;
}

void R_load (char s[8], int f) {
  assert(max_textures < 256);
  if (!s[0]) {
    walp[max_textures] = (image) {
      .n = NULL,
      .x = 0,
      .y = 0,
      .w = 0,
      .h = 0,
      .res = -1,
    };
  } else if (strncasecmp(s, "_WATER_", 7) == 0) {
    walp[max_textures] = (image) {
      .n = (void*)((intptr_t)s[7] - '0' + 1),
      .x = 0,
      .y = 0,
      .w = 8,
      .h = 8,
      .res = -2,
    };
  } else {
    walp[max_textures] = R_gl_loadimage(s);
    if (f) {
      walp[max_textures].res |= 0x8000;
    }
    if (s[0] == 'S' && s[1] == 'W' && s[4] == '_') {
      walswp[max_textures] = 0;
    }
    walani[max_textures] = getani(s);
  }
  max_wall_width = max(max_wall_width, walp[max_textures].w);
  max_wall_height = max(max_wall_height, walp[max_textures].h);
  max_textures++;
}

void R_end_load (void) {
  int i, j, k, g;
  char s[8];
  j = max_textures;
  for (i = 1; i < 256 && j < 256; i++) {
    if (walswp[i] == 0) {
      R_get_name(i, s);
      s[5] ^= 1;
      g = F_getresid(s) | (walp[i].res & 0x8000);
      k = 1;
      while (k < 256 && walp[k].res != g) {
        k += 1;
      }
      if (k >= 256) {
        k = j;
        j += 1;
        walp[k] = R_gl_getimage(g);
        walf[k] = g & 0x8000 ? 1 : 0;
      }
      walswp[i] = k;
      walswp[k] = i;
    }
  }
}

void R_loadsky (int sky) {
  char s[6];
  strcpy(s, "RSKYx");
  s[4] = '0' + sky;
  R_gl_free_image(&horiz);
  horiz = R_gl_loadimage(s);
}