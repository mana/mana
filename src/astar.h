#ifdef WIN32
  #pragma warning (disable:4312)
#endif

#include <allegro.h>

#include "map.h"
#include "being.h"

#define NOT_STARTED 0
#define FOUND 1
#define NOT_FOUND 2

extern char walkability [MAP_WIDTH][MAP_HEIGHT];
extern int whichList[MAP_WIDTH+1][MAP_HEIGHT+1];
extern int G_cost[MAP_WIDTH+1][MAP_HEIGHT+1];

void ReadPath(int pathfinderID,int currentX,int currentY, int pixelsPerFrame);
int ReadPathX(int pathfinderID,int pathLocation);
int ReadPathY(int pathfinderID,int pathLocation);
void RenderScreen (bool stepByStep=false);
PATH_NODE *find_path(int pathfinderID,int startingX, int startingY, int targetX, int targetY);
void ReadPath(int pathfinderID);


