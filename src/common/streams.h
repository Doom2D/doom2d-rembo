#ifndef COMMON_STREAMS_H_INCLUDED
#define COMMON_STREAMS_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

typedef struct Stream Stream;

struct Stream {
  long (*getpos)(Stream *rw);
  void (*setpos)(Stream *rw, long pos);
  long (*getlen)(Stream *rw);
  void (*read)(Stream *r, void *data, size_t size, size_t n);
  void (*write)(Stream *w, const void *data, size_t size, size_t n);
};

long stream_getpos (Stream *s);
void stream_setpos (Stream *s, long pos);

long stream_getlen (Stream *s);

void stream_read (void *data, size_t size, size_t n, Stream *r);
int8_t stream_read8 (Stream *r);
int16_t stream_read16 (Stream *r);
int32_t stream_read32 (Stream *r);

void stream_write (const void *data, size_t size, size_t n, Stream *w);
void stream_write8 (int8_t x, Stream *w);
void stream_write16 (int16_t x, Stream *w);
void stream_write32 (int32_t x, Stream *w);

#endif /* COMMON_STREAMS_H_INCLUDED */