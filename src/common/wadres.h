#ifndef COMMON_WADRES_H_INCLUDED
#define COMMON_WADRES_H_INCLUDED

#include "common/streams.h"

#define MAX_WADS 20
#define MAX_RESOURCES 2000

int WADRES_addwad (Reader *r);
int WADRES_rehash (void);

int WADRES_find (const char name[8]);
int WADRES_maxids (void);

Reader *WADRES_getbasereader (int id);
long WADRES_getoffset (int id);
long WADRES_getsize (int id);
void WADRES_getname (int id, char *name);
void WADRES_getdata (int id, void *data);

#endif /* COMMON_WADRES_H_INCLUDED */