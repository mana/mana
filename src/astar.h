#ifndef _ASTAR_H
#define _ASTAR_H

#include "map.h"
#include "being.h"

PATH_NODE *find_path(int pathfinderID, int startingX, int startingY,
        int targetX, int targetY);

#endif
