#ifdef WIN32
#pragma warning(disable:4312)
#endif

#ifndef _2XSAI_H
#define _2XSAI_H

#include <allegro.h>
#include <allegro/internal/aintern.h>
//#include <string.h>



#define uint32 unsigned long
#define uint16 unsigned short
#define uint8 unsigned char

int Init_2xSaI(int depth);
void Super2xSaI(BITMAP * src, BITMAP * dest, int s_x, int s_y, int d_x, int d_y, int w, int h);
void Super2xSaI_ex(uint8 *src, uint32 src_pitch, uint8 *unused, BITMAP *dest, uint32 width, uint32 height);

void SuperEagle(BITMAP * src, BITMAP * dest, int s_x, int s_y, int d_x, int d_y, int w, int h);
void SuperEagle_ex(uint8 *src, uint32 src_pitch, uint8 *unused, BITMAP *dest, uint32 width, uint32 height);

#endif
