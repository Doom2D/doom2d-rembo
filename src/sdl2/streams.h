#ifndef D2D_SDL2_STREAMS_H_INCLUDED
#define D2D_SDL2_STREAMS_H_INCLUDED

#include <SDL2/SDL.h>
#include <common/streams.h>

typedef struct SDLRW_Stream {
  Stream base;
  SDL_RWops *io;
} SDLRW_Stream;

void SDLRW_Assign (SDLRW_Stream *s, SDL_RWops *io);
int  SDLRW_Open   (SDLRW_Stream *s, const char *name, const char *mode);
void SDLRW_Close  (SDLRW_Stream *s);

#endif /* D2D_SDL2_STREAMS_H_INCLUDED */
