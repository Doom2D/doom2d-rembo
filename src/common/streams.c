#include "streams.h"
#include "endianness.h"

#include <stddef.h>
#include <stdint.h>

long stream_getpos (Stream *s) {
  return s->getpos(s);
}

void stream_setpos (Stream *s, long pos) {
  s->setpos(s, pos);
}

long stream_getlen (Stream *s) {
  return s->getlen(s);
}

void stream_read (void *data, size_t size, size_t n, Stream *r) {
  r->read(r, data, size, n);
}

int8_t stream_read8 (Stream *r) {
  int8_t x;
  r->read(r, &x, 1, 1);
  return x;
}

int16_t stream_read16 (Stream *r) {
  int16_t x;
  r->read(r, &x, 2, 1);
  return short2host(x);
}

int32_t stream_read32 (Stream *r) {
  int32_t x;
  r->read(r, &x, 4, 1);
  return int2host(x);
}

void stream_write (const void *data, size_t size, size_t n, Stream *w) {
  w->write(w, data, size, n);
}

void stream_write8 (int8_t x, Stream *w) {
  w->write(w, &x, 1, 1);
}

void stream_write16 (int16_t x, Stream *w) {
  int16_t y = short2host(x);
  w->write(w, &y, 2, 1);
}

void stream_write32 (int32_t x, Stream *w) {
  int32_t y = int2host(x);
  w->write(w, &y, 4, 1);
}
