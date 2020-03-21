/*
   Copyright (C) Prikol Software 1996-1997
   Copyright (C) Aleksey Volynskov 1996-1997
   Copyright (C) <ARembo@gmail.com> 2011

   This file is part of the Doom2D:Rembo project.

   Doom2D:Rembo is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   Doom2D:Rembo is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/> or
   write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "glob.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "sound.h"
#include "music.h"
#include "view.h"
#include "monster.h"
#include "player.h"
#include "menu.h"
#include "files.h"
#include "render.h"
#include "error.h"
#include "input.h"
#include "my.h"

enum{NONE,BYTE,WORD,DWORD,STRING,SW_ON,SW_OFF,FILES,KEY};

typedef struct{
  char *par,*cfg;
  void *p;
  byte t,o;
}cfg_t;

byte cheat=0;
byte shot_vga=0;
char cd_path[128]="";
char cfg_file[128]="default.cfg";

static char buf[256];

static cfg_t cfg[]={
  {"file",NULL,NULL,FILES,0},
  {"cheat",NULL,&cheat,SW_ON,0},
  {"vga","screenshot",&shot_vga,SW_ON,0},
  {"sndvol","sound_volume",&snd_vol,WORD,0},
  {"musvol","music_volume",&mus_vol,WORD,0},
//  {"fullscr","fullscreen",&fullscreen,SW_ON,0},
//  {"window",NULL,&fullscreen,SW_OFF,0},
  {NULL,"sky",&w_horiz,SW_ON,0},
  {"mon",NULL,&nomon,SW_OFF,0},
//  {"gamma","gamma",&gammaa,DWORD,0},
  {"warp",NULL,&_warp,BYTE,0},
  {"width","screen_width",&SCRW,DWORD,0},
  {"height","screen_height",&SCRH,DWORD,0},
  {NULL,"music_random",&music_random,SW_ON,0},
  {NULL,"music_time",&music_time,DWORD,0},
  {NULL,"music_fade",&music_fade,DWORD,0},
  {NULL,"pl1_left", &pl1.kl,KEY,0},
  {NULL,"pl1_right",&pl1.kr,KEY,0},
  {NULL,"pl1_up",   &pl1.ku,KEY,0},
  {NULL,"pl1_down", &pl1.kd,KEY,0},
  {NULL,"pl1_jump", &pl1.kj,KEY,0},
  {NULL,"pl1_fire", &pl1.kf,KEY,0},
  {NULL,"pl1_next", &pl1.kwr,KEY,0},
  {NULL,"pl1_prev", &pl1.kwl,KEY,0},
  {NULL,"pl1_use",  &pl1.kp,KEY,0},
  {NULL,"pl2_left", &pl2.kl,KEY,0},
  {NULL,"pl2_right",&pl2.kr,KEY,0},
  {NULL,"pl2_up",   &pl2.ku,KEY,0},
  {NULL,"pl2_down", &pl2.kd,KEY,0},
  {NULL,"pl2_jump", &pl2.kj,KEY,0},
  {NULL,"pl2_fire", &pl2.kf,KEY,0},
  {NULL,"pl2_next", &pl2.kwr,KEY,0},
  {NULL,"pl2_prev", &pl2.kwl,KEY,0},
  {NULL,"pl2_use",  &pl2.kp,KEY,0},
  {"config",NULL,cfg_file,STRING,0},
  {NULL,NULL,NULL,NONE,0}
};

void CFG_args(int argc, char *argv[]) {
  int j;
  dword n;
  char *s;

  logo("CFG_args: проверка командной строки\n");

  int i;
  char *pbuf = buf;
  for (i=1;i<argc;i++){
     strcpy(pbuf,argv[i]);
     pbuf+=strlen(argv[i]);
     strcpy(pbuf," ");
     pbuf++;
  }

  for(s=strtok(buf," \r\n\t");s;s=strtok(NULL," \r\n\t")) {//for(s=strtok(getcmd(buf)," \r\n\t");s;s=strtok(NULL," \r\n\t")) {
next:
    if(*s=='/' || *s=='-') ++s;
    for(j=0;cfg[j].t;++j) if(cfg[j].par) if(strcasecmp(s,cfg[j].par)==0) {
	  switch(cfg[j].t) {
	case BYTE:
	  n=strtol(s=strtok(NULL," \r\n\t"),NULL,0);
	  *((byte *)cfg[j].p)=(byte)n;
	  break;
	case WORD:
	  n=strtol(s=strtok(NULL," \r\n\t"),NULL,0);
	  *((word *)cfg[j].p)=(word)n;
	  break;
	case DWORD:
	  n=strtol(s=strtok(NULL," \r\n\t"),NULL,0);
	  *((dword *)cfg[j].p)=n;
	  break;
	case STRING:
	  strcpy((char *)cfg[j].p,s=strtok(NULL," \r\n\t"));
	  break;
	case SW_ON:
	  *((byte *)cfg[j].p)=ON;
	  if(cfg[j+1].t==SW_OFF && cfg[j+1].p==cfg[j].p) cfg[j+1].o=1;
	  if(j>0) if(cfg[j-1].t==SW_OFF && cfg[j-1].p==cfg[j].p) cfg[j-1].o=1;
	  break;
	case SW_OFF:
	  *((byte *)cfg[j].p)=OFF;
	  if(cfg[j+1].t==SW_ON && cfg[j+1].p==cfg[j].p) cfg[j+1].o=1;
	  if(j>0) if(cfg[j-1].t==SW_ON && cfg[j-1].p==cfg[j].p) cfg[j-1].o=1;
	  break;
	case FILES:
	  for(s=strtok(NULL," \r\n\t");s;s=strtok(NULL," \r\n\t")) {
		if(*s=='/' || *s=='-') goto next;
#ifdef DEMO
		logo("  %s НЕ подключен!\n",s);
#else
		F_addwad(s);
#endif
	  }break;
	default:
	  ERR_failinit("!!! Неизвестный тип в cfg !!!");
	  }
	  cfg[j].o=1;break;
    }
  }
}

void CFG_load(void) {
  int j;
  FILE *h;
  dword n;
  char s[128];
  char *p1,*p2;

  char pc[50];
  char *e = getenv("HOME");
  strncpy(pc, e, 30);
  strcpy(&pc[strlen(pc)], "/default.cfg");
  if (!fexists(pc)) {
      strcpy(pc, "default.cfg");
      if (!fexists(pc)) {
          strcpy(pc, "/usr/share/doom2d-rembo/default.cfg");
          if (!fexists(pc)) {
              logo("default.cfg not found\n");
              return;
          }
      }
  }

  logo("CFG_load: загрузка конфигурации из %s\n",pc);
  if((h=fopen(pc,"rb"))==NULL) {
    perror("Cannot open file");return;
  }
  while(!feof(h)) {
    F_readstr(h,s,127);
    if(*s==';' || s[1]==';')
      continue; // comment
    if(!(p1=strtok(s,"\r\n\t=;")))
      continue; //if(!(p1=strtok(s,"\r\n\t =;"))) continue;
    if(!(p2=strtok(NULL,"\r\n\t=;")))
      continue;//if(!(p2=strtok(NULL,"\r\n\t =;"))) continue;
    for(j=0;cfg[j].t;++j) {
      if(cfg[j].cfg && !cfg[j].o) {
        if(strcasecmp(p1,cfg[j].cfg)==0) {
          switch(cfg[j].t) {
          case BYTE:
            n=strtol(p2,NULL,0);
            *((byte *)cfg[j].p)=(byte)n;
            break;
          case WORD:
            n=strtol(p2,NULL,0);
            *((word *)cfg[j].p)=(word)n;
            break;
          case DWORD:
            n=strtol(p2,NULL,0);
            *((dword *)cfg[j].p)=n;
            break;
          case STRING:
            strcpy((char *)cfg[j].p,p2);
            break;
          case SW_ON:
          case SW_OFF:
            if(strcasecmp(p2,"ON")==0) {
            *((byte *)cfg[j].p)=ON;
            break;
          }
          if(strcasecmp(p2,"OFF")==0) {
            *((byte *)cfg[j].p)=OFF;
            break;
          }
          *((byte *)cfg[j].p)=strtol(p2,NULL,0);
          break;
          case FILES:
            break;
          case KEY:
          {
              int k = I_string_to_key(p2);
              if (k != KEY_UNKNOWN) {
                *((int *)cfg[j].p)=k;
              } else {
                int i;
                logo("Unknown key in cfg: %s=%s\n",p1,p2);
                logo("List available key names:\n");
                for(i = 1; i <= KEY__LAST; i++) {
                  logo("  %s\n", I_key_to_string(i));
                }
              }
          }
          break;
          default:
            ERR_failinit("!!! Неизвестный тип в cfg !!!");
          } // switch
          break;
        } // if
      } // if
    } // for
  } // while
  fclose(h);
}

void CFG_save(void) {
/*
  char s[140],str[140];
  char *p;
  FILE *h,*oh;

  remove("CONFIG.ZZZ");
  if(rename(cfg_file,"CONFIG.ZZZ")) return;
  if(!(h=fopen("CONFIG.ZZZ","rt")))
    {rename("CONFIG.ZZZ",cfg_file);return;}
  if(!(oh=fopen(cfg_file,"wt")))
    {fclose(h);rename("CONFIG.ZZZ",cfg_file);return;}
  for(;;) {
    if(!fgets(s,128,h)) break;
    strcpy(str,s);
    if(!(p=strtok(str,"\r\n\t =;"))) {fprintf(oh,"%s",s);continue;}
    if(strcasecmp(p,"sound_volume")==0)
      sprintf(s,"sound_volume=%d\n",snd_vol);
    else if(strcasecmp(p,"music_volume")==0)
      sprintf(s,"music_volume=%d\n",mus_vol);
    else if(strcasecmp(p,"gamma")==0)
      sprintf(s,"gamma=%d\n",gammaa);
    else if(strcasecmp(p,"sound_interp")==0)
      sprintf(s,"sound_interp=%s\n",s_interp?"on":"off");
    fprintf(oh,"%s",s);
  }
  fclose(oh);fclose(h);
  remove("CONFIG.ZZZ");
*/
}
