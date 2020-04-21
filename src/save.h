#ifndef SAVE_H_INCLUDED
#define SAVE_H_INCLUDED

#include "glob.h"

#define SAVE_MAX 7
#define SAVE_MAXLEN 24

extern char savname[SAVE_MAX][SAVE_MAXLEN];
extern char savok[SAVE_MAX];

void F_getsavnames (void);
void F_savegame (int n, char *s);
void F_loadgame (int n);

#endif /* SAVE_H_INCLUDED */
