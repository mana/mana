#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
#define MAKEWORD(low,high) \
        ((WORD)(((BYTE)(low)) | ((WORD)((BYTE)(high))) << 8))
#define  closesocket(a)  close(a)
#define SOCKET int
#define SOCKET_ERROR -1
#define SOCKADDR_IN struct sockaddr_in
typedef struct	sockaddr                SOCKADDR;
typedef	SOCKADDR *      LPSOCKADDR;
#define WSACleanup() ;



typedef unsigned short                  WORD;
typedef unsigned long int LWORD;
typedef unsigned char BYTE;
#define LOBYTE(w)                       ((BYTE)          (w)            )
#define HIBYTE(w)                       ((BYTE)  (((WORD)(w)) >> 8)     )
#define LOWORD(l)                       ((WORD)          (l)            )
#define HIWORD(l)                       ((WORD) (((LWORD)(l)) >> 16)    )
#define HANDLE int
#define HANDLE int
#define PHANDLE int
#define SMALL_RECT int
//#define WORD int
#define DWORD int
#define PDWORD int
#define BOOL int
#define LPBOOL int
#define LPSTR int
#define LPTSTR int
#define LPCTSTR int
#define LPDWORD int
#define LPVOID int
#define WINAPI

#define LOBYTE(w)                       ((BYTE)          (w)            )
#define HIBYTE(w)                       ((BYTE)  (((WORD)(w)) >> 8)     )
#define LPTHREAD_START_ROUTINE void *(*)(void *)
#define CloseHandle close
