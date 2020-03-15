#ifndef MUSIC_H_INCLUDED
#define MUSIC_H_INCLUDED

extern short mus_vol;
extern char music_random;
extern int music_time;
extern int music_fade;

void S_initmusic (void);
void S_donemusic (void);
void S_startmusic (int time);
void S_stopmusic (void);
void S_volumemusic (int v);
void F_loadmus (char n[8]);
void F_freemus (void);
void S_updatemusic (void);

#endif /* MUSIC_H_INCLUDED */
