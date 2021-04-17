#ifndef KOS32_H
#define KOS32_H

/* --- fn 0 (styles) --- */
#define KOS32_WIN_STYLE_EMPTY 0
#define KOS32_WIN_STYLE_SKIN  3
#define KOS32_WIN_STYLE_FIXED 4

/* --- fn 0 (flags) --- */
#define KOS32_WIN_FLAG_CAPTION  1
#define KOS32_WIN_FLAG_RELATIVE 2
#define KOS32_WIN_FLAG_NOFILL   4
#define KOS32_WIN_FLAG_GRADIENT 8
#define KOS32_WIN_FLAG_UNMOVABLE 256

/* --- fn 0 --- */
static inline void CreateWindow (int x, int y, int w, int h, int style, int flags, int color, int hcolor, const char *title) {
  int f = (title ? KOS32_WIN_FLAG_CAPTION : 0) | (flags & ~KOS32_WIN_FLAG_CAPTION);
  __asm__ __volatile__ (
    "int $0x40"
    :
    : "a" (0),
      "b" (x << 16 | ((w - 1) & 0xffff)),
      "c" (y << 16 | ((h - 1) & 0xffff)),
      "d" ((f << 28) | ((style & 0xf) << 24) | (color & 0xffffff)),
      "S" (((f >> 8) << 24) | (hcolor & 0xffffff)),
      "D" (title)
    : "memory"
  );
}

/* --- fn 12.1 --- */
static inline void BeginDraw (void) {
   __asm__ __volatile__ (
    "int $0x40"
    :
    : "a" (12),
      "b" (1)
  );
}

/* --- fn 12.2 --- */
static inline void EndDraw (void) {
   __asm__ __volatile__ (
    "int $0x40"
    :
    : "a" (12),
      "b" (2)
  );
}

/* --- fn 65 --- */
static inline void PutImageExt (const void *image, int w, int h, int x, int y, int bpp, const void *pal, int offset) {
  __asm__ __volatile__ (
    "push %%ebp; movl %6, %%ebp; int $0x40; pop %%ebp"
    :
    : "a" (65),
      "b" (image),
      "c" (w << 16 | (h & 0xffff)),
      "d" (x << 16 | (y & 0xffff)),
      "S" (bpp),
      "D" (pal),
      "m" (offset)
    : "memory", "ebp"
  );
}

/* --- fn 18.4 --- */
static inline int GetIdleCount (void) {
  int ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret)
    : "a" (18),
      "b" (4)
  );
  return ret;
}

/* --- fn 26.9 --- */
/* result is 1/100 sec */
static inline int GetTimeCount (void) {
  int ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret)
    : "a" (26),
      "b" (9)
  );
  return ret;
}

/* --- fn 26.10 --- */
/* result is 1/1000000 sec (nanosec) */
static inline long GetTimeCountPro (void) {
  union {
    struct { int lo, hi; };
    long pair;
  } ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret.lo),
      "=b" (ret.hi)
    : "a" (26),
      "b" (10)
  );
  return ret.pair;
}

/* --- fn 40 (flags) --- */
#define KOS32_EVENT_FLAG_REDRAW         (1 << 0)
#define KOS32_EVENT_FLAG_KEYBOARD       (1 << 1)
#define KOS32_EVENT_FLAG_BUTTON         (1 << 2)
#define KOS32_EVENT_FLAG_RESERVED       (1 << 3)
#define KOS32_EVENT_FLAG_REDRAW_BG      (1 << 4)
#define KOS32_EVENT_FLAG_MOUSE          (1 << 5)
#define KOS32_EVENT_FLAG_IPC            (1 << 6)
#define KOS32_EVENT_FLAG_NETWORK        (1 << 7)
#define KOS32_EVENT_FLAG_DEBUG          (1 << 8)
#define KOS32_EVENT_FLAG_IRQ0           (1 << 15)
#define KOS32_EVENT_FLAG_IRQ1           (1 << 16)
#define KOS32_EVENT_FLAG_IRQ2           (1 << 17)
#define KOS32_EVENT_FLAG_IRQ3           (1 << 18)
#define KOS32_EVENT_FLAG_IRQ4           (1 << 19)
#define KOS32_EVENT_FLAG_IRQ5           (1 << 20)
#define KOS32_EVENT_FLAG_IRQ6           (1 << 21)
#define KOS32_EVENT_FLAG_IRQ7           (1 << 22)
#define KOS32_EVENT_FLAG_IRQ8           (1 << 23)
#define KOS32_EVENT_FLAG_IRQ9           (1 << 24)
#define KOS32_EVENT_FLAG_IRQ10          (1 << 25)
#define KOS32_EVENT_FLAG_IRQ11          (1 << 26)
#define KOS32_EVENT_FLAG_IRQ12          (1 << 27)
#define KOS32_EVENT_FLAG_IRQ13          (1 << 28)
#define KOS32_EVENT_FLAG_IRQ14          (1 << 29)
#define KOS32_EVENT_FLAG_IRQ15          (1 << 30) /* ??? */
#define KOS32_EVENT_FLAG_MOUSE_NOFOCUS  (1 << 30)
#define KOS32_EVENT_FLAG_MOUSE_NOACTIVE (1 << 31)

/* --- fn 40 (events) --- */
#define KOS32_EVENT_NONE           0
#define KOS32_EVENT_REDRAW         1
#define KOS32_EVENT_KEYBOARD       2
#define KOS32_EVENT_BUTTON         3
#define KOS32_EVENT_RESERVED       4
#define KOS32_EVENT_REDRAW_BG      5
#define KOS32_EVENT_MOUSE          6
#define KOS32_EVENT_IPC            7
#define KOS32_EVENT_NETWORK        8
#define KOS32_EVENT_DEBUG          9
#define KOS32_EVENT_IRQ0           16
#define KOS32_EVENT_IRQ1           17
#define KOS32_EVENT_IRQ2           18
#define KOS32_EVENT_IRQ3           19
#define KOS32_EVENT_IRQ4           20
#define KOS32_EVENT_IRQ5           21
#define KOS32_EVENT_IRQ6           22
#define KOS32_EVENT_IRQ7           23
#define KOS32_EVENT_IRQ8           24
#define KOS32_EVENT_IRQ9           25
#define KOS32_EVENT_IRQ10          26
#define KOS32_EVENT_IRQ11          27
#define KOS32_EVENT_IRQ12          28
#define KOS32_EVENT_IRQ13          29
#define KOS32_EVENT_IRQ14          30
#define KOS32_EVENT_IRQ15          31

/* --- fn 40 --- */
static inline int SetEventsMask (int mask) {
  int ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret)
    : "a" (mask)
  );
  return ret;
}

/* --- fn 11 --- */
static inline int CheckEvent (void) {
  int ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret)
    : "a" (11)
  );
  return ret;
}

/* --- fn 23 --- */
static inline int WaitEventTimeout (int timeout) {
  int ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret)
    : "a" (23),
      "b" (timeout)
  );
  return ret;
}

/* --- fn 2 --- */
static inline int GetKey (void) {
  int ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret)
    : "a" (2)
  );
  return ret;
}

/* --- fn 17 --- */
static inline int GetButton (void) {
  int ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret)
    : "a" (17)
  );
  return ret;
}

#define KOS32_INPUT_MODE_ASCII 0
#define KOS32_INPUT_MODE_SCANCODE 1

/* --- fn 66.1 --- */
static inline void SetInputMode (int mode) {
  __asm__ __volatile__ (
    "int $0x40"
    :
    : "a" (66),
      "b" (1),
      "c" (mode)
  );
}

/* --- fn 66.2 --- */
static inline int GetInputMode (void) {
  int ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret)
    : "a" (66),
      "b" (2)
  );
  return ret;
}

/* --- fn 48.4 --- */
static inline int GetSkinHeight (void) {
  int ret;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret)
    : "a" (48),
      "b" (4)
  );
  return ret;
}

/* --- fn 68.1 --- */
static inline void SwitchTask (void) {
  __asm__ __volatile__ (
    "int $0x40"
    :
    : "a" (68),
      "b" (1)
  );
}

/* --- fn 5 --- */
static inline void Delay (int time) {
  __asm__ __volatile__ (
    "int $0x40"
    :
    : "a" (5),
      "b" (time)
  );
}

#define KOS32_SC_UNKNOWN 0x00
#define KOS32_SC_ESCAPE  0x01
#define KOS32_SC_1       0x02
#define KOS32_SC_2       0x03
#define KOS32_SC_3       0x04
#define KOS32_SC_4       0x05
#define KOS32_SC_5       0x06
#define KOS32_SC_6       0x07
#define KOS32_SC_7       0x08
#define KOS32_SC_8       0x09
#define KOS32_SC_9       0x0A
#define KOS32_SC_0       0x0B
#define KOS32_SC_MINUS   0x0C
#define KOS32_SC_EQUALS  0x0D
#define KOS32_SC_BACKSPACE 0x0E
#define KOS32_SC_TAB     0x0F
#define KOS32_SC_Q       0x10
#define KOS32_SC_W       0x11
#define KOS32_SC_E       0x12
#define KOS32_SC_R       0x13
#define KOS32_SC_T       0x14
#define KOS32_SC_Y       0x15
#define KOS32_SC_U       0x16
#define KOS32_SC_I       0x17
#define KOS32_SC_O       0x18
#define KOS32_SC_P       0x19
#define KOS32_SC_LEFTBRACKET 0x1A
#define KOS32_SC_RIGHTBRACKET 0x1B
#define KOS32_SC_RETURN  0x1C
#define KOS32_SC_LCTRL   0x1D
#define KOS32_SC_A       0x1E
#define KOS32_SC_S       0x1F
#define KOS32_SC_D       0x20
#define KOS32_SC_F       0x21
#define KOS32_SC_G       0x22
#define KOS32_SC_H       0x23
#define KOS32_SC_J       0x24
#define KOS32_SC_K       0x25
#define KOS32_SC_L       0x26
#define KOS32_SC_SEMICOLON 0x27
#define KOS32_SC_APOSTROPHE 0x28
#define KOS32_SC_GRAVE   0x29
#define KOS32_SC_LSHIFT  0x2A
#define KOS32_SC_BACKSLASH 0x2B
#define KOS32_SC_Z       0x2C
#define KOS32_SC_X       0x2D
#define KOS32_SC_C       0x2E
#define KOS32_SC_V       0x2F
#define KOS32_SC_B       0x30
#define KOS32_SC_N       0x31
#define KOS32_SC_M       0x32
#define KOS32_SC_COMMA   0x33
#define KOS32_SC_PERIOD  0x34
#define KOS32_SC_SLASH   0x35
#define KOS32_SC_RSHIFT  0x36
#define KOS32_SC_KP_MULTIPLY 0x37
#define KOS32_SC_LALT    0x38
#define KOS32_SC_SPACE   0x39
#define KOS32_SC_CAPSLOCK 0x3A
#define KOS32_SC_F1      0x3B
#define KOS32_SC_F2      0x3C
#define KOS32_SC_F3      0x3D
#define KOS32_SC_F4      0x3E
#define KOS32_SC_F5      0x3F
#define KOS32_SC_F6      0x40
#define KOS32_SC_F7      0x41
#define KOS32_SC_F8      0x42
#define KOS32_SC_F9      0x43
#define KOS32_SC_F10     0x44
#define KOS32_SC_NUMLOCK 0x45
#define KOS32_SC_SCROLLLOCK 0x46
#define KOS32_SC_KP_7    0x47
#define KOS32_SC_KP_8    0x48
#define KOS32_SC_KP_9    0x49
#define KOS32_SC_KP_MINUS 0x4A
#define KOS32_SC_KP_4    0x4B
#define KOS32_SC_KP_5    0x4C
#define KOS32_SC_KP_6    0x4D
#define KOS32_SC_KP_PLUS 0x4E
#define KOS32_SC_KP_1    0x4F
#define KOS32_SC_KP_2    0x50
#define KOS32_SC_KP_3    0x51
#define KOS32_SC_KP_0    0x52
#define KOS32_SC_KP_PERIOD 0x53
/* ... */
#define KOS32_SC_F11     0x57
#define KOS32_SC_F12     0x58

/* extended scancodes */
#define KOS32_SC_EXTENDED   0xE0 /* starts with this */
#define KOS32_SC_PREV_TRACK 0x10
#define KOS32_SC_NEXT_TRACK 0x19
#define KOS32_SC_KP_ENTER   0x1C
#define KOS32_SC_RCTRL      0x1D
#define KOS32_SC_MUTE       0x20
#define KOS32_SC_CALC       0x21
#define KOS32_SC_PLAY       0x22
#define KOS32_SC_STOP       0x24
#define KOS32_SC_VOLUME_DOWN 0x2E
#define KOS32_SC_VOLUME_UP  0x30
#define KOS32_SC_WWW        0x32
#define KOS32_SC_KP_DIVIDE  0x35
#define KOS32_SC_RALT       0x38
#define KOS32_SC_HOME       0x47
#define KOS32_SC_UP         0x48
#define KOS32_SC_PAGEUP     0x49
#define KOS32_SC_LEFT       0x4B
#define KOS32_SC_RIGHT      0x4D
#define KOS32_SC_END        0x4F
#define KOS32_SC_DOWN       0x50
#define KOS32_SC_PAGEDOWN   0x51
#define KOS32_SC_INSERT     0x52
#define KOS32_SC_DELETE     0x53
#define KOS32_SC_LSUPER     0x5B
#define KOS32_SC_RSUPER     0x5C
#define KOS32_SC_APP        0x5D
#define KOS32_SC_POWER      0x5E
#define KOS32_SC_SLEEP      0x5F
#define KOS32_SC_WAKE       0x63
#define KOS32_SC_WWW_SEARCH 0x65
#define KOS32_SC_WWW_FAVORITE 0x66
#define KOS32_SC_WWW_REFRESH 0x67
#define KOS32_SC_WWW_STOP   0x68
#define KOS32_SC_WWW_FORWARD 0x69
#define KOS32_SC_WWW_BACK   0x6A
#define KOS32_SC_MY_COMPUTER 0x6B
#define KOS32_SC_EMAIL      0x6C
#define KOS32_SC_SELECT     0x6D

#define KOS32_SC_EXTENDED_PAUSE 0xE1 /* pause key seq */

#pragma pack(push, 1)
struct FileExt {
  int fn;
  int a, b, c, d;
  int enc;
  const void *path;
};

struct FileTime {
  char s, m, h, reserved;
};

struct FileDate {
  char d, m;
  unsigned short y;
};

struct FileInfo {
  int attr;
  int enc;
  struct FileTime ctime;
  struct FileDate cdate;
  struct FileTime atime;
  struct FileDate cdate;
  struct FileTime mtime;
  struct FileDate mdate;
  long long size;
  union {
    char  cp866[264];
    short utf16[260];
    char  utf8[520];
  };
};
#pragma pack(pop)

#define KOS32_READ_FILE     0
#define KOS32_READ_FOLDER   1
#define KOS32_CREATE_FILE   2
#define KOS32_WRITE_FILE    3
#define KOS32_SET_END       4
#define KOS32_GET_INFO      5
#define KOS32_SET_INFO      6
#define KOS32_START_APP     7
#define KOS32_DELETE        8
#define KOS32_CREATE_FOLDER 9
#define KOS32_RENAME        10

#define KOS32_FILE_SUCCESS       0
#define KOS32_FILE_NOT_SUPPORTED 2
#define KOS32_FILE_UNKNOWN_FS    3
#define KOS32_FILE_NOT_FOUND     5
#define KOS32_FILE_EOF           6
#define KOS32_FILE_INVALID_PTR   7
#define KOS32_FILE_DISK_FULL     8
#define KOS32_FILE_FS_ERROR      9
#define KOS32_FILE_ACCESS_DENIED 10
#define KOS32_FILE_DEVICE_ERROR  11
#define KOS32_FILE_OUT_OF_MEMORY 12

#define KOS32_CP866 1
#define KOS32_UTF16 2
#define KOS32_UTF8  3

#define KOS32_ATTR_READONLY 0
#define KOS32_ATTR_HIDDEN   1
#define KOS32_ATTR_SYSTEM   2
#define KOS32_ATTR_VOLUME   3
#define KOS32_ATTR_FOLDER   4
#define KOS32_ATTR_ARCHIVED 5

#define KOS32_ATTR_MASK_READONLY (1 << KOS32_ATTR_READONLY)
#define KOS32_ATTR_MASK_HIDDEN   (1 << KOS32_ATTR_HIDDEN)
#define KOS32_ATTR_MASK_SYSTEM   (1 << KOS32_ATTR_SYSTEM)
#define KOS32_ATTR_MASK_VOLUME   (1 << KOS32_ATTR_VOLUME)
#define KOS32_ATTR_MASK_FOLDER   (1 << KOS32_ATTR_FOLDER)
#define KOS32_ATTR_MASK_ARCHIVED (1 << KOS32_ATTR_ARCHIVED)

static inline int FileExt (struct FileExt *info, int *ret) {
  int ret1, ret2;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (ret1),
      "=b" (ret2)
    : "a" (80),
      "b" (info)
    : "memory"
  );
  if (ret) *ret = ret2;
  return ret1;
}

/* --- fn 30.4 --- */
static inline void SetCurrentFolderEnc (char *path, int enc) {
  __asm__ __volatile__ (
    "int $0x40"
    :
    : "a" (30),
      "b" (4),
      "c" (path),
      "d" (enc)
    : "memory"
  );
}

/* --- fn 30.5 --- */
static inline int GetCurrentFolderEnc (char *buf, int maxlen, int enc) {
  int len;
  __asm__ __volatile__ (
    "int $0x40"
    : "=a" (len)
    : "a" (30),
      "b" (5),
      "c" (buf),
      "d" (maxlen),
      "S" (enc)
    : "memory"
  );
  return len;
}

#endif /* KOS32_H */
