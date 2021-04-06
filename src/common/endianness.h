#ifndef COMMON_ENDIANNESS_H_INCLUDED
#define COMMON_ENDIANNESS_H_INCLUDED

#include <stdint.h>

#define SWAP_VAR(a, b) do { unsigned char t = a; a = b; b = t; } while(0)

static inline int16_t short2swap (int16_t x) {
  union {
    uint8_t a[2];
    int16_t x;
  } y;
  y.x = x;
  SWAP_VAR(y.a[0], y.a[1]);
  return y.x;
}

static inline int32_t int2swap (int32_t x) {
  union {
    uint8_t a[4];
    int32_t x;
  } y;
  y.x = x;
  SWAP_VAR(y.a[0], y.a[3]);
  SWAP_VAR(y.a[1], y.a[2]);
  return y.x;
}

#undef SWAP_VAR

static inline int16_t short2host (int16_t x) {
#if __BIG_ENDIAN__
  return short2swap(x);
#else
  return x;
#endif
}

static inline int32_t int2host (int32_t x) {
#if __BIG_ENDIAN__
  return int2swap(x);
#else
  return x;
#endif
}

#endif /* COMMON_ENDIANNESS_H_INCLUDED */