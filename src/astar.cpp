#include "astar.h"

#define MAP_WIDTH  200
#define MAP_HEIGHT 200

#define WALKABLE     0
#define NOT_WALKABLE 1

// path-related constants
const int numberPeople = 1;
int onClosedList = 10;
const int notfinished = 0;

// Declare needed arrays
//char tiledMap.getPathWalk [MAP_WIDTH][MAP_HEIGHT];

/** 1 dimensional array holding ID# of open list items */
int openList[MAP_WIDTH * MAP_HEIGHT + 2];

/**
 * 2 dimensional array used to record whether a cell is on the open list or
 * on the closed list
 */
int whichList[MAP_WIDTH + 1][MAP_HEIGHT + 1];

/** 1d array stores the x location of an item on the open list */
int openX[MAP_WIDTH * MAP_HEIGHT + 2];

/** 1d array stores the y location of an item on the open list */
int openY[MAP_WIDTH * MAP_HEIGHT + 2];

/** 2d array to store parent of each cell (x) */
int parentX[MAP_WIDTH + 1][MAP_HEIGHT + 1];

/** 2d array to store parent of each cell (y) */
int parentY[MAP_WIDTH + 1][MAP_HEIGHT + 1];

/** 1d array to store F cost of a cell on the open list */
int F_cost[MAP_WIDTH * MAP_HEIGHT + 2];

/** 2d array to store G_cost cost for each cell */
int G_cost[MAP_WIDTH + 1][MAP_HEIGHT + 1];

/** 1d array to store H cost of a cell on the open list */
int H_cost[MAP_WIDTH * MAP_HEIGHT + 2];

int pathLength;     /**< length of the FOUND path for critter */
int pathLocation;   /**< current position along the chosen path for critter */
int* path_bank = NULL;

// Path reading variables
int pathStatus;
int xPath;
int yPath;

/** Initialize pathfinder */
void pathfinder_init() {
    path_bank = (int*)malloc(4);
}

/** Exit pathfinder */
void pathfinder_exit() {
    free(path_bank);
}

/** Find path */
PATH_NODE *find_path(int pathfinderID, int s_x, int s_y, int e_x, int e_y)
{
    int onOpenList = 0, parentXval = 0, parentYval = 0;
    int a = 0, b = 0, m = 0, u = 0, v = 0, temp = 0, corner = 0;
    int numberOfOpenListItems = 0, addedGCost = 0, tempG = 0, path = 0, x = 0;
    int y = 0, tempx, pathX, pathY, cellPosition, newOpenListItemID = 0;

    // If starting location and target are in the same location...
    if (s_x == e_x && s_y == e_y && pathLocation > 0) return NULL;
    else if (s_x == e_x && s_y == e_y && pathLocation == 0) return NULL;

    // If dest tile is not walkable, return that it's a NOT_FOUND path.
    if (!tiledMap.getWalk(e_x, e_y)) {
        xPath = s_x;
        yPath = s_y;
        return NULL;
    }

    // Reset some variables that need to be cleared
    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            whichList[x][y] = 0;
        }
    }

    // Changing the values of onOpenList and onClosed list is faster than
    // redimming whichList() array
    onClosedList = 2;
    onOpenList = 1;
    pathLength = NOT_STARTED;
    pathLocation = NOT_STARTED;

    // Reset starting square's G_cost value to 0
    G_cost[s_x][s_y] = 0;

    // Add the starting location to the open list of tiles to be checked.
    numberOfOpenListItems = 1;

    // Assign it as the top (and currently only) item in the open list, which
    // is maintained as a binary heap (explained below)
    openList[1] = 1;
    openX[1] = s_x ; openY[1] = s_y;

    // Do the following until a path is FOUND or deemed NOT_FOUND.
    do {
        // If the open list is not empty, take the first cell off of the list.
        // This is the lowest F cost cell on the open list.
        if (numberOfOpenListItems != 0)
        {
            // Pop the first item off the open list.

            // Record cell coordinates of the item
            parentXval = openX[openList[1]];
            parentYval = openY[openList[1]];

            // Add the item to the closed list
            whichList[parentXval][parentYval] = onClosedList;

            // Open List = Binary Heap: Delete this item from the open list,
            // which
            // Reduce number of open list items by 1
            numberOfOpenListItems = numberOfOpenListItems - 1;

            // Delete the top item in binary heap and reorder the heap, with
            // the lowest F cost item rising to the top.
            // Move the last item in the heap up to slot #1
            openList[1] = openList[numberOfOpenListItems + 1];
            v = 1;

            // Repeat the following until the new item in slot #1 sinks to its
            // proper spot in the heap.
            do {
                u = v;
                // If both children exist
                if (2 * u + 1 <= numberOfOpenListItems) {
                    // Check if the F cost of the parent is greater than each
                    // child.
                    // Select the lowest of the two children.
                    if (F_cost[openList[u]] >= F_cost[openList[2 * u]])
                        v = 2 * u;
                    if (F_cost[openList[v]] >= F_cost[openList[2 * u + 1]])
                        v = 2 * u + 1;
                } else {
                    // If only child #1 exists
                    if (2 * u <= numberOfOpenListItems) {
                        // Check if the F cost of the parent is greater than
                        // child #1
                        if (F_cost[openList[u]] >= F_cost[openList[2 * u]])
                            v = 2 * u;
                    }
                }

                if (u != v) {
                    // If parent's F is > one of its children, swap them
                    temp = openList[u];
                    openList[u] = openList[v];
                    openList[v] = temp;
                }
                else {
                    // Otherwise, exit loop
                    break;
                }
            } while (u != v); // Reorder the binary heap


            //  Check the adjacent squares. (Its "children" -- these path
            //  children are similar, conceptually, to the binary heap children
            //  mentioned above, but don't confuse them. They are different.
            //  Path children are portrayed in Demo 1 with grey pointers
            //  pointing toward their parents.) Add these adjacent child
            //  squares to the open list for later consideration if appropriate
            //  (see various if statements below).

            for (b = parentYval - 1; b <= parentYval + 1; b++) {
                for (a = parentXval - 1; a <= parentXval + 1; a++) {
                    //  If not off the map (do this first to avoid array
                    //  out-of-bounds errors)
                    if (a != -1 && b != -1 &&
                            a != MAP_WIDTH && b != MAP_HEIGHT)
                    {
                        // If not already on the closed list (items on the
                        // closed list have already been considered and can now
                        // be ignored).
                        if (whichList[a][b] != onClosedList) {
                            // If not a wall/obstacle square.
                            if (tiledMap.getWalk(a, b)) {
                                //  Don't cut across corners
                                corner = WALKABLE;

                                if (a == parentXval - 1) {
                                    if (b == parentYval - 1) {
                                        if (!tiledMap.getWalk(parentXval - 1, parentYval) || !tiledMap.getWalk(parentXval, parentYval - 1)) // cera slash
                                            corner = NOT_WALKABLE;
                                    } else if (b == parentYval + 1) {
                                        if (!tiledMap.getWalk(parentXval, parentYval + 1) || !tiledMap.getWalk(parentXval - 1, parentYval))
                                            corner = NOT_WALKABLE;
                                    }
                                }
                                else if (a == parentXval + 1) {
                                    if (b == parentYval - 1) {
                                        if (!tiledMap.getWalk(parentXval, parentYval - 1) || !tiledMap.getWalk(parentXval + 1, parentYval))
                                            corner = NOT_WALKABLE;
                                    } else if (b == parentYval + 1) {
                                        if (!tiledMap.getWalk(parentXval + 1, parentYval) || !tiledMap.getWalk(parentXval, parentYval + 1))
                                            corner = NOT_WALKABLE;
                                    }
                                }

                                if (corner == WALKABLE) {
                                    // If not already on the open list, add it
                                    // to the open list.
                                    if (whichList[a][b] != onOpenList) {
                                        // Create a new open list item in the
                                        // binary heap.
                                        // Each new item has a unique ID #
                                        newOpenListItemID += 1;
                                        m = numberOfOpenListItems + 1;
                                        // Place the new open list item
                                        // (actually, its ID#) at the bottom of
                                        // the heap
                                        openList[m] = newOpenListItemID;
                                        // Record the x and y coordinates of
                                        // the new item
                                        openX[newOpenListItemID] = a;
                                        openY[newOpenListItemID] = b;

                                        // Figure out its G_cost cost
                                        if (abs(a - parentXval) == 1 &&
                                                abs(b - parentYval) == 1)
                                        {
                                            // Cost of going to diagonal
                                            // squares.
                                            addedGCost = 14;
                                        }
                                        else {
                                            // Cost of going to non-diagonal
                                            // squares.
                                            addedGCost = 10;
                                        }

                                        G_cost[a][b] =
                                            G_cost[parentXval][parentYval] +
                                            addedGCost;

                                        // Figure out its H and F costs and
                                        // parent.
                                        H_cost[openList[m]] =
                                            10 * (abs(a - e_x) + abs(b - e_y));
                                        F_cost[openList[m]] =
                                            G_cost[a][b] + H_cost[openList[m]];
                                        parentX[a][b] = parentXval;
                                        parentY[a][b] = parentYval;

                                        // Move the new open list item to the
                                        // proper place in the binary heap.
                                        // Starting at the bottom, successively
                                        // compare to parent items, swapping as
                                        // needed until the item finds its
                                        // place in the heap or bubbles all the
                                        // way to the top (if it has the lowest
                                        // F cost).

                                        // While item hasn't bubbled to the
                                        // top (m = 1)
                                        while (m != 1) {
                                            // Check if child's F cost is <
                                            // parent's F cost. If so, swap
                                            // them.
                                            if (F_cost[openList[m]] <=
                                                    F_cost[openList[m / 2]])
                                            {
                                                temp = openList[m / 2];
                                                openList[m / 2] = openList[m];
                                                openList[m] = temp;
                                                m = m / 2;
                                            }
                                            else {
                                                break;
                                            }
                                        }

                                        // Add one to the number of items in
                                        // the heap.
                                        numberOfOpenListItems += 1;
                                        // Change whichList to show that the
                                        // new item is on the open list.
                                        whichList[a][b] = onOpenList;
                                    } else {
                                        // If whichList(a,b) = onOpenList

                                        // If adjacent cell is already on the
                                        // open list, check to see if this path
                                        // to that cell from the starting
                                        // location is a better one.  If so,
                                        // change the parent of the cell and
                                        // its G_cost and F costs.  Figure out
                                        // the G_cost cost of this possible new
                                        // path

                                        if (abs(a - parentXval) == 1 &&
                                                abs(b - parentYval) == 1)
                                        {
                                            // Cost of going to diagonal tiles
                                            addedGCost = 14;
                                        }
                                        else {
                                            // Cost of going to non-diagonal
                                            // tiles
                                            addedGCost = 10;
                                        }

                                        tempG = G_cost[parentXval][parentYval]
                                            + addedGCost;

                                        // If this path is shorter (G_cost cost
                                        // is lower) then change the parent
                                        // cell, G_cost cost and F cost.
                                        if (tempG < G_cost[a][b]) {
                                            // If G_cost cost is less,
                                            // change the square's parent
                                            parentX[a][b] = parentXval;
                                            parentY[a][b] = parentYval;
                                            // Change the G_cost cost
                                            G_cost[a][b] = tempG;

                                            // Because changing the G_cost cost
                                            // also changes the F cost, if the
                                            // item is on the open list we need
                                            // to change the item's recorded F
                                            // cost and its position on the
                                            // open list to make sure that we
                                            // maintain a properly ordered open
                                            // list.

                                            // Look for the item in the heap
                                            for (int x = 1;
                                                    x <= numberOfOpenListItems;
                                                    x++)
                                            {
                                                if (openX[openList[x]] == a &&
                                                        openY[openList[x]] == b)
                                                {
                                                    // Item FOUND
                                                    // Change the F cost
                                                    F_cost[openList[x]] =
                                                        G_cost[a][b] +
                                                        H_cost[openList[x]];

                                                    // See if changing the F
                                                    // score bubbles the item
                                                    // up from it's current
                                                    // location in the heap
                                                    m = x;

                                                    // While item hasn't
                                                    // bubbled to the top
                                                    // (m = 1)
                                                    while (m != 1) {
                                                        // Check if child is <
                                                        // parent. If so, swap
                                                        // them.
                                                        if (F_cost[openList[m]] < F_cost[openList[m / 2]]) {
                                                            temp = openList[m / 2];
                                                            openList[m / 2] =
                                                                openList[m];
                                                            openList[m] = temp;
                                                            m = m / 2;
                                                        }
                                                        else {
                                                            break;
                                                        }
                                                    }
                                                    //Exit for x = loop
                                                    break;
                                                } // If openX(openList(x)) = a
                                            } // F x = 1 To nrOfOpenListItems
                                        } // If tempG < G_cost(a, b)
                                    } // else If whichList(a, b) = onOpenList
                                } // If not cutting a corner
                            } // If not a wall/obstacle square.
                        } // If not already on the closed list
                    } // If not off the map
                } // for (a = parentXval - 1; a <= parentXval + 1; a++)
            } // for (b = parentYval - 1; b <= parentYval + 1; b++)
        } else { // if (numberOfOpenListItems != 0)
            // If open list is empty then there is no path.
            path = NOT_FOUND;
            break;
        }

        // If target is added to open list then path has been FOUND.
        if (whichList[e_x][e_y] == onOpenList) {
            path = FOUND;
            break;
        }

    }
    // Do until path is FOUND or deemed NOT_FOUND
    while (path != FOUND && path != NOT_FOUND);

    // Save the path if it exists.
    if (path == FOUND)
    {
        // Working backwards from the target to the starting location by
        // checking each cell's parent, figure out the length of the path.
        pathX = e_x; pathY = e_y;
        do {
            // Look up the parent of the current cell.
            tempx = parentX[pathX][pathY];
            pathY = parentY[pathX][pathY];
            pathX = tempx;

            // Figure out the path length
            pathLength = pathLength + 1;
        }
        while (pathX != s_x || pathY != s_y);

        // Resize the data bank to the right size in bytes
        path_bank = (int*)realloc(path_bank, pathLength * 8);

        // Now copy the path information over to the databank. Since we are
        // working backwards from the target to the start location, we copy the
        // information to the data bank in reverse order. The result is a
        // properly ordered set of path data, from the first step to the last.

        pathX = e_x ; pathY = e_y;
        // Start at the end
        cellPosition = pathLength * 2;
        do {
            // Work backwards 2 integers
            cellPosition = cellPosition - 2;
            path_bank[cellPosition] = pathX;
            path_bank[cellPosition + 1] = pathY;
            // Look up the parent of the current cell.
            tempx = parentX[pathX][pathY];
            pathY = parentY[pathX][pathY];
            pathX = tempx;
            // If we have reached the starting square, exit the loop.
        }
        while (pathX != s_x || pathY != s_y);

        PATH_NODE *ret = NULL, *temp = NULL;
        pathLocation = 1;
        ret = new PATH_NODE(s_x, s_y);
        temp = ret;
        while (pathLocation < pathLength) {
            temp->next = new PATH_NODE(
                    path_bank[pathLocation * 2 - 2],
                    path_bank[pathLocation * 2 - 1]);
            if (temp->next == NULL) throw "Unable to create path node";
            temp = temp->next;
            pathLocation++;
        }
        if (temp != NULL) {
            temp->next = new PATH_NODE(e_x, e_y);
        }
        else {
            throw "Null reference";
        }

        return ret;
    }

    // Path not found
    return NULL;
}

void ReadPath(int pathfinderID)
{
    // If a path exists, read the path data from the pathbank.
    // Set pathLocation to 1st step
    pathLocation = 1;
    while (pathLocation<pathLength) {
        int a = path_bank[pathLocation * 2 - 2];
        int b = path_bank[pathLocation * 2 - 1];
        pathLocation = pathLocation + 1;
        // Draw dotted path
        whichList[a][b] = 3;
    }
}
