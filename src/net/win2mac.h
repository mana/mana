#ifndef _TMW_WIN2MAC_
#define _TMW_WIN2MAC_

#include <stdio.h>

#define UInt16 unsigned short int
#define UInt32 unsigned long int

UInt32 DR_SwapFourBytes(UInt32 dw);
UInt16 DR_SwapTwoBytes(UInt16 w);
char* SwapChar(char charlist[]);

#endif
