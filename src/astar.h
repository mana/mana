#ifndef _ASTAR_H
#define _ASTAR_H

#include "map.h"
#include "being.h"

#define NOT_STARTED 0
#define FOUND 1
#define NOT_FOUND 2

PATH_NODE *find_path(int pathfinderID, int startingX, int startingY,
        int targetX, int targetY);

/** Read the path data */
void ReadPath(int pathfinderID);

#endif
