#ifndef D2D_KOS32_STREAMS_H_INCLUDED
#define D2D_KOS32_STREAMS_H_INCLUDED

#include <common/streams.h>

typedef struct KOS32_Stream {
  Stream base;
  char name[264];
  long pos;
} KOS32_Stream;

void KOS32_Assign (KOS32_Stream *s, const char *name, long pos);
int  KOS32_Open   (KOS32_Stream *s, const char *name);
int  KOS32_Create (KOS32_Stream *s, const char *name);
void KOS32_Close  (KOS32_Stream *s);

#endif /* D2D_KOS32_STREAMS_H_INCLUDED */
