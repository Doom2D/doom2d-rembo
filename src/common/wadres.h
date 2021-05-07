#ifndef COMMON_WADRES_H_INCLUDED
#define COMMON_WADRES_H_INCLUDED

#include "streams.h"

#define MAX_WADS 20
#define MAX_RESOURCES 2000

int WADRES_addwad (Stream *r);
int WADRES_rehash (void);

int WADRES_find (const char name[8]);
int WADRES_maxids (void);

// Get sprite resource id.
// Sprite name has following format:
//  (nnnn)('A'+s)('0'+d)[('A'+s)('0'+d)]
//  Letter means animation frame
//    A for first, B for second...
//  Number means direction
//    0 = front
//    1 = left
//    2 = right
//  Optional part means that this file can be used for differnt frame/direction.
//  Note that if found FRONT direction for this frame than it UNCONDITIONALLY used.
//  Note that search performed between markers S_START and S_END in order as paced in wad.
//  int n[4]  -- sprite name
//  int s     -- sprite frame
//  int d     -- sprite direction
//  char *dir -- out flag "alternative used"
int WADRES_findsprite (const char n[4], int s, int d, char *dir);

Stream *WADRES_getbasereader (int id);
long WADRES_getoffset (int id);
long WADRES_getsize (int id);
void WADRES_getname (int id, char *name);
void WADRES_getdata (int id, void *data);

void *WADRES_lock (int id);
void  WADRES_unlock (void *data);
int   WADRES_locked (int id);
int   WADRES_was_locked (int id);

#endif /* COMMON_WADRES_H_INCLUDED */