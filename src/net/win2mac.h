#ifndef _TMW_MAC_H
#define _TMW_MAC_H

#include <stdio.h>

#define UInt16 unsigned short int
#define INT16 short int
#define UInt32 unsigned long int
#define INT32 long int
#define SWAP( a,  b ) { char c; c=a; a=b; b=c; }

UInt32 DR_SwapFourBytes(UInt32 dw);
UInt16 DR_SwapTwoBytes(UInt16 w);
char* SwapChar(char charlist[]);

#endif
