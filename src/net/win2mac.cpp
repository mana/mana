#include "win2mac.h"

UInt32 DR_SwapFourBytes(UInt32 dw)
{
    UInt32 tmp;
    tmp =  (dw & 0x000000FF);
    tmp = ((dw & 0x0000FF00) >> 0x08) | (tmp << 0x08);
    tmp = ((dw & 0x00FF0000) >> 0x10) | (tmp << 0x08);
    tmp = ((dw & 0xFF000000) >> 0x18) | (tmp << 0x08);
    return (tmp);
}

UInt16 DR_SwapTwoBytes(UInt16 w)
{
    UInt16 tmp;
    tmp =  (w & 0x00FF);
    tmp = ((w & 0xFF00) >> 0x08) | (tmp << 0x08);
    return(tmp);
}

char* SwapChar(char charlist[])
{
    for (int i = 0; i < 24 / 2; i++)
        SWAP(charlist[i],charlist[24 - i]);
    return charlist;
}

/*
char* SwapChar(char charlist[])
{
    for (int i = 0; i < sizeof(charlist) * 4 / 2; i++)
        SWAP(charlist[i],charlist[sizeof(charlist) * 4 - i]);
    return charlist;
}
*/
