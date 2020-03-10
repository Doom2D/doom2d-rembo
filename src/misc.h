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

// Miscellaneous functions

#define MAXDIST 2000000L

enum{
  Z_HITWALL=1,Z_HITCEIL=2,Z_HITLAND=4,Z_FALLOUT=8,
  Z_INWATER=16,Z_HITWATER=32,Z_HITAIR=64,Z_BLOCK=128
};

void *Z_getspr(char[4],int,int,char *);
void *Z_getsnd(char[6]);
void Z_drawspr(int,int,void *,char);
void Z_drawmanspr(int,int,void *,char,byte);
void Z_drawstair(int);
void Z_drawstprcnt(int,int);
void Z_drawstnum(int);
void Z_drawstwpn(int,int);
void Z_drawstkeys(byte);
void Z_drawstlives(char);
int Z_sound(void *,int);
void Z_initst(void);
void Z_drawfld(byte *, int);
int Z_sign(int);
int Z_dec(int,int);
int Z_canstand(int,int,int);
int Z_canfit(int x,int y,int r,int h);
void Z_teleobj(int o,int x,int y);
int Z_moveobj(obj_t *);
int Z_what_cpu(void);
int Z_gunhit(int,int,int,int,int);
int Z_overlap(obj_t *,obj_t *);
int Z_look(obj_t *,obj_t *,int);
int Z_hit(obj_t *,int,int,int);
int Z_hitobj(int id,int d,int o,int t);
void Z_kickobj(obj_t *,int,int,int);
void Z_explode(int,int,int,int);
void Z_bfg9000(int,int,int);
int Z_cansee(int,int,int,int);
int Z_chktrap(int,int d,int o,int t);
int Z_istrapped(int,int,int,int);
int Z_inwater(int x,int y,int r,int h);
int Z_canbreathe(int x,int y,int r,int h);
int Z_getobjpos(int,obj_t *o);

void Z_water_trap(obj_t *);
void Z_untrap(byte);

void Z_splash(obj_t *,int n);

void Z_set_speed(obj_t *,int);

void Z_calc_time(dword t,word *h,word *m,word *s);

void Z_putbfch(int);
void Z_printbf(char *,...);
void Z_putsfch(int);
void Z_printsf(char *,...);
void Z_gotoxy(int,int);

int Z_getacid(int x,int y,int r,int h);

void Z_clrst();

unsigned short int short2host (unsigned short int x);
unsigned int int2host (unsigned int x);
