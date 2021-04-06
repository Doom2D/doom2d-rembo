#ifndef COMMON_STREAMS_H_INCLUDED
#define COMMON_STREAMS_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

typedef struct Reader Reader;
typedef struct Writer Writer;

struct Reader {
  void (*setpos)(Reader *r, int pos);
  void (*read)(Reader *r, void *data, size_t size, size_t n);
};

struct Writer {
  void (*setpos)(Writer *w, int pos);
  void (*write)(Writer *w, const void *data, size_t size, size_t n);
};

void stream_read (void *data, size_t size, size_t n, Reader *f);
int8_t stream_read8 (Reader *f);
int16_t stream_read16 (Reader *f);
int32_t stream_read32 (Reader *f);

void stream_write (const void *data, size_t size, size_t n, Writer *w);
void stream_write8 (int8_t x, Writer *w);
void stream_write16 (int16_t x, Writer *w);
void stream_write32 (int32_t x, Writer *w);

#endif /* COMMON_STREAMS_H_INCLUDED */