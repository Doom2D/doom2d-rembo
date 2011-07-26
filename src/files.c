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
//#include <conio.h>
#include <malloc.h>
//#include <dos.h>
#include <string.h>
#include <stdlib.h>
//#include <sys\stat.h>
#include "vga.h"
#include "error.h"
#include "sound.h"
//#include "snddrv.h"
#include "memory.h"
#include "view.h"
#include "items.h"
#include "switch.h"
#include "files.h"
#include "map.h"

#include <sys/stat.h>

char *S_getinfo(void);

extern void *snd_drv;

typedef struct{
  byte n,i,v,d;
}dmv;

byte seq[255],seqn;
dmv *pat=NULL;
unsigned *patp;
void **dmi;

static int inum=0;

void G_savegame(FILE*);
void W_savegame(FILE*);
void DOT_savegame(FILE*);
void SMK_savegame(FILE*);
void FX_savegame(FILE*);
void IT_savegame(FILE*);
void MN_savegame(FILE*);
void PL_savegame(FILE*);
void SW_savegame(FILE*);
void WP_savegame(FILE*);

void G_loadgame(FILE*);
void W_loadgame(FILE*);
void DOT_loadgame(FILE*);
void SMK_loadgame(FILE*);
void FX_loadgame(FILE*);
void IT_loadgame(FILE*);
void MN_loadgame(FILE*);
void PL_loadgame(FILE*);
void SW_loadgame(FILE*);
void WP_loadgame(FILE*);

byte savname[7][24],savok[7];

int d_start,d_end,m_start,m_end,s_start,s_end,wad_num;
mwad_t wad[MAX_WAD];

char wads[MAX_WADS][__MAX_PATH];
static FILE* wadh[MAX_WADS];

char f_drive[__MAX_DRIVE],f_dir[__MAX_DIR],f_name[__MAX_FNAME],f_ext[__MAX_EXT],
  f_path[__MAX_PATH];

void F_startup(void) {
  logo("F_startup: настройка файловой системы\n");
  memset(wads,0,sizeof(wads));
}

char *getsavfpname(int n, int ro)
{
  static char fn[]="savgame0.dat";
  fn[7]=n+'0';
#ifndef WIN32
  static char p[100];
  char *e = getenv("HOME");
  strncpy(p,e,60);
  strcat(p,"/.doom2d-rembo");
  if (!ro) mkdir(p, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  strcat(p,"/");
  strcat(p,fn);
#else
  strcpy(p,fn);
#endif
  return p;
}

void F_getsavnames(void) {

  int i; FILE *h;
  short ver;
  char *p;

  for(i=0;i<7;++i) {
    p = getsavfpname(i,1);
    memset(savname[i],0,24);savok[i]=0;
    if((h=fopen(p,"rb"))==NULL) continue; //if((h=open(n,O_RDONLY|O_BINARY))==-1) continue;
    myfread(savname[i],1,24,h);ver=-1;myfread(&ver,1,2,h);
    fclose(h);savname[i][23]=0;savok[i]=(ver==3)?1:0;//savok[i]=(ver==2)?1:0;
  }
}

void F_savegame(int n,char *s) {

  FILE* h;
  char *p;
  p=getsavfpname(n,0);
  if((h=fopen(p,"wb"))==NULL) return;
  myfwrite(s,1,24,h);myfwrite("\3\0",1,2,h);//myfwrite("\2\0",1,2,h);
  G_savegame(h);
  W_savegame(h);
  DOT_savegame(h);
  SMK_savegame(h);
  FX_savegame(h);
  IT_savegame(h);
  MN_savegame(h);
  PL_savegame(h);
  SW_savegame(h);
  WP_savegame(h);
  fclose(h);
}

void F_loadgame(int n) {
  FILE* h;
  short ver;
  char *p;
  p=getsavfpname(n,1);
  
  if((h=fopen(p,"rb"))==NULL) return;//if((h=open(fn,O_BINARY|O_RDONLY))==-1) return;
  fseek(h,24,SEEK_SET);myfread(&ver,1,2,h);if(ver!=3) return;//if(ver!=2) return;
  G_loadgame(h);
  W_loadgame(h);
  DOT_loadgame(h);
  SMK_loadgame(h);
  FX_loadgame(h);
  IT_loadgame(h);
  MN_loadgame(h);
  PL_loadgame(h);
  SW_loadgame(h);
  WP_loadgame(h);
  fclose(h);
}

void F_addwad(char *fn) {
  int i;

  for(i=0;i<MAX_WADS;++i) if(wads[i][0]==0) {
    strcpy(wads[i],fn);return;
  }
  ERR_failinit("Не могу добавить WAD %s",fn);
}


int myfilelength(FILE *h)
{
    int pos = ftell(h);
    fseek (h, 0, SEEK_END);
    int len = ftell(h);
    fseek (h, pos, SEEK_SET);
    return len;
}

extern void mysplitpath(const char* path, char* drv, char* dir, char* name, char* ext);

// build wad directory
void F_initwads(void) {
  int i,j,k,p;
  FILE *h;
  char s[4];
  int n,o;
  wad_t w;

  logo("F_initwads: подключение WAD-файлов\n");
  for(i=0;i<MAX_WAD;++i) wad[i].n[0]=0;
  logo("   подключается  %s\n",wads[0]);
  if((wadh[0]=h=fopen(wads[0],"rb"))==NULL)//if((wadh[0]=h=open(wads[0],O_RDWR|O_BINARY))==-1)
	ERR_failinit("Не могу открыть файл: %s",wads[0]);//sys_errlist[errno]);
  *s=0;myfread(s,1,4,h);
  if(strncmp(s,"IWAD",4)!=0 && strncmp(s,"PWAD",4)!=0)
	ERR_failinit("Нет подписи IWAD или PWAD");
  myfread(&n,1,4,h);myfread(&o,1,4,h);fseek(h,o,SEEK_SET);
  for(j=0,p=0;j<n;++j) {
	myfread(&w,1,16,h);
	if(p>=MAX_WAD) ERR_failinit("Слишком много элементов WAD'а");
	memcpy(wad[p].n,w.n,8);
	wad[p].o=w.o;wad[p].l=w.l;wad[p].f=0;
	++p;
  }
  //fclose(h);
  for(i=1;i<MAX_WADS;++i) if(wads[i][0]!=0) {
      
	logo("  подключается %s\n",wads[i]);
	if((wadh[i]=h=fopen(wads[i], "rb"))==NULL) //if((wadh[i]=h=open(wads[i],O_RDONLY|O_BINARY))==-1)
	  ERR_failinit("Не могу открыть файл2:  %s",wads[i]);//sys_errlist[errno]);
	mysplitpath(wads[i],f_drive,f_dir,f_name,f_ext);
	if(strcasecmp(f_ext,".lmp")==0) {
	  for(k=0;k<MAX_WAD;++k) if(strncasecmp(wad[k].n,f_name,8)==0)
		{wad[k].o=0L;wad[k].l=myfilelength(h);wad[k].f=i;break;}
	  if(k>=MAX_WAD) {
		if(p>=MAX_WAD) ERR_failinit("Слишком много элементов WAD'а");
		memset(wad[p].n,0,8);
		strncpy(wad[p].n,f_name,8);
		wad[p].o=0L;wad[p].l=myfilelength(h);wad[p].f=i;
		++p;
	  }
	  continue;
	}
	*s=0;myfread(s,1,4,h);
	if(strncmp(s,"IWAD",4)!=0 && strncmp(s,"PWAD",4)!=0)
	  ERR_failinit("Нет подписи IWAD или PWAD");
    myfread(&n,1,4,h);myfread(&o,1,4,h);fseek(h,o,SEEK_SET);
    for(j=0;j<n;++j) {
	  myfread(&w,1,16,h);
	  for(k=0;k<MAX_WAD;++k) if(strncasecmp(wad[k].n,w.n,8)==0)
		{wad[k].o=w.o;wad[k].l=w.l;wad[k].f=i;break;}
	  if(k>=MAX_WAD) {
		if(p>=MAX_WAD) ERR_failinit("Слишком много элементов WAD'а");
		memcpy(wad[p].n,w.n,8);
		wad[p].o=w.o;wad[p].l=w.l;wad[p].f=i;
		++p;
      }
    }
  }
  wad_num=p;
   
}

// allocate resources
// (called from M_startup)
void F_allocres(void) {
  d_start=F_getresid("D_START");
  d_end=F_getresid("D_END");
  m_start=F_getresid("M_START");
  m_end=F_getresid("M_END");
  s_start=F_getresid("S_START");
  s_end=F_getresid("S_END");
}

// load resource
void F_loadres(int r,void *p,dword o,dword l) {

  int oo;
  FILE *fh;

  
  oo=ftell(fh=wadh[wad[r].f]);

  if(fseek(fh,wad[r].o+o,SEEK_SET)!=0)
    ERR_fatal("Ошибка при чтении файла");

  if((dword)myfreadc(p,1,l,fh)!=l)
    ERR_fatal("Ошибка при загрузке ресурса %.8s",wad[r].n);
  
  fseek(fh,oo,SEEK_SET);
  
}


void F_saveres(int r,void *p,dword o,dword l) {

  int oo;
 FILE* fh;
  oo=ftell(fh=wadh[wad[r].f]);
  if(fseek(fh,wad[r].o+o,SEEK_SET)!=0)
    ERR_fatal("Ошибка при чтении файла");
  myfwrite(p,l,1,fh);
  fseek(fh,oo,SEEK_SET);

}

// get resource id
int F_getresid(char *n) {
  int i;

  for(i=0;i<wad_num;++i) if(strncasecmp(wad[i].n,n,8)==0) return i;
    ERR_fatal("F_getresid: ресурс %.8s не найден",n);
  return -1;
}

// get resource id
int F_findres(char *n) {
  int i;

  for(i=0;i<wad_num;++i) if(strncasecmp(wad[i].n,n,8)==0) return i;
  return -1;
}

void F_getresname(char *n,int r) {
  memcpy(n,wad[r].n,8);
}

// get sprite id
int F_getsprid(char n[4],int s,int d) {
  int i;
  byte a,b;

  s+='A';d+='0';
  for(i=s_start+1;i<s_end;++i)
    if(strncasecmp(wad[i].n,n,4)==0 && (wad[i].n[4]==s || wad[i].n[6]==s)) {
      if(wad[i].n[4]==s) a=wad[i].n[5]; else a=0;
      if(wad[i].n[6]==s) b=wad[i].n[7]; else b=0;
      if(a=='0') return i;
      if(b=='0') return(i|0x8000);
      if(a==d) return i;
      if(b==d) return(i|0x8000);
    }
  ERR_fatal("F_getsprid: изображение %.4s%c%c не найдено",n,(byte)s,(byte)d);
  return -1;
}

int F_getreslen(int r) {
  return wad[r].l;
}

void F_nextmus(char *s) {
  int i;
  i=F_findres(s);
  if(i<=m_start || i>=m_end) i=m_start;
  for(++i;;++i) {
    if(i>=m_end) i=m_start+1;

    if (strcasecmp(wad[i].n,"MENU") == 0 ||
        strcasecmp(wad[i].n,"INTERMUS") == 0 ||
        strcasecmp(wad[i].n,"\x8a\x8e\x8d\x85\x96\x0") == 0)
        continue;

    if(strncasecmp(wad[i].n,"DMI",3)!=0) break;
  }
  memcpy(s,wad[i].n,8);
}

void F_randmus(char *s) {
   int n = myrand(10);
   int i;
   for (i=0; i<n; i++) {
       F_nextmus(s);
   }
}

// reads bytes from file until CR
void F_readstr(FILE* h,char *s,int m) {
  int i;
  static char c;

  for(i=0;;) {
    c=13;
    myfreadc(&c,1,1,h);
    if(c==13) break;
    if(i<m) s[i++]=c;
  }
  s[i]=0;
}

// reads bytes from file until NUL
void F_readstrz(FILE* h,char *s,int m) {
  int i;
  static char c;

  for(i=0;;) {
    c=0;
    myfreadc(&c,1,1,h);
    if(c==0) break;
    if(i<m) s[i++]=c;
  }
  s[i]=0;
}

map_block_t blk;

void F_loadmap(char n[8]) {

  int r;
  FILE *h;
  map_header_t hdr;
  int o;

  W_init();
  r=F_getresid(n);
  fseek(h=wadh[wad[r].f],wad[r].o,SEEK_SET);
  myfread(&hdr,1,sizeof(hdr),h);
  if(memcmp(hdr.id,"Doom2D\x1A",8)!=0)
	ERR_fatal("%.8s не является уровнем",n);
  for(;;) {
	myfread(&blk,1,sizeof(blk),h);
	if(blk.t==MB_END) break;
	if(blk.t==MB_COMMENT)
	  {fseek(h,blk.sz,SEEK_CUR);continue;}
	o=ftell(h)+blk.sz;
	if(!G_load(h))
	if(!W_load(h))
	if(!IT_load(h))
	if(!SW_load(h))
	  ERR_fatal("Неизвестный блок %d(%d) в уровне %.8s",blk.t,blk.st,n);
	fseek(h,o,SEEK_SET);
  }

}

/*void F_freemus(void) {

  int i;

  if(!pat) return;
  S_stopmusic();
  free(pat);free(patp);
  for(i=0;i<inum;++i) if(dmi[i]!=NULL) free(dmi[i]);
  free(dmi);
  pat=NULL;

}
*/

/*void F_loadmus(char n[8]) {
  int r,i,j;
  FILE *h;
  int o;
  struct{
	char id[4];
	byte ver,pat;
	word psz;
  }d;
  struct{byte t;char n[13];word r;}di;

  if((r=F_findres(n))==-1) return;
  fseek(h=wadh[wad[r].f],wad[r].o,SEEK_SET);
  myfread(&d,1,sizeof(d),h);
  if(memcmp(d.id,"DMM",4)!=0) return;
  if(!(pat=malloc(d.psz<<2))) return;
  myfread(pat,1,d.psz<<2,h);
  myfread(&seqn,1,1,h);if(seqn) myfread(seq,1,seqn,h);
  inum=0;myfread(&inum,1,1,h);
  if(!(dmi=malloc(inum*4))) {free(pat);pat=NULL;return;}
  if(!(patp=malloc((word)d.pat*32))) {free(pat);free(dmi);pat=NULL;return;}
  for(i=0;i<inum;++i) {
	dmi[i]=NULL;
	myfread(&di,1,16,h);o=ftell(h);
	for(r=0;r<12;++r) if(di.n[r]=='.') di.n[r]=0;
	if((r=F_findres(di.n))==-1) continue;
	if(!(dmi[i]=malloc(wad[r].l+8))) continue;
	memset(dmi[i],0,16);
	F_loadres(r,dmi[i],0,2);
	F_loadres(r,(int*)dmi[i]+1,2,2);
	F_loadres(r,(int*)dmi[i]+2,4,2);
	F_loadres(r,(int*)dmi[i]+3,6,2);
	F_loadres(r,(int*)dmi[i]+4,8,wad[r].l-8);
	fseek(h,o,SEEK_SET);
  }
  for(i=r=0,j=(word)d.pat<<3;i<j;++i) {
	patp[i]=r<<2;
	while(pat[r++].v!=0x80);
  }
 }
*/

