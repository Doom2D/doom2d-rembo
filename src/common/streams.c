#include "streams.h"

#include <stddef.h>
#include <stdint.h>

#include "misc.h" // endianness conversion

void stream_read (void *data, size_t size, size_t n, Reader *r) {
  r->read(r, data, size, n);
}

int8_t stream_read8 (Reader *r) {
  int8_t x;
  r->read(r, &x, 1, 1);
  return x;
}

int16_t stream_read16 (Reader *r) {
  int16_t x;
  r->read(r, &x, 2, 1);
  return short2host(x);
}

int32_t stream_read32 (Reader *r) {
  int32_t x;
  r->read(r, &x, 4, 1);
  return int2host(x);
}

void stream_write (const void *data, size_t size, size_t n, Writer *w) {
  w->write(w, data, size, n);
}

void stream_write8 (int8_t x, Writer *w) {
  w->write(w, &x, 1, 1);
}

void stream_write16 (int16_t x, Writer *w) {
  int16_t y = short2host(x);
  w->write(w, &y, 2, 1);
}

void stream_write32 (int32_t x, Writer *w) {
  int32_t y = int2host(x);
  w->write(w, &y, 4, 1);
}
