#include "astar.h"
#include "being.h"
#include "map.h"

const int numberPeople = 1;
int onClosedList = 10;
const int notfinished = 0;// path-related constants

//Create needed arrays
//char tiledMap.getPathWalk [MAP_WIDTH][MAP_HEIGHT];
int openList[MAP_WIDTH*MAP_HEIGHT+2]; //1 dimensional array holding ID# of open list items
int whichList[MAP_WIDTH+1][MAP_HEIGHT+1];  //2 dimensional array used to record 
//     whether a cell is on the open list or on the closed list.
int openX[MAP_WIDTH*MAP_HEIGHT+2]; //1d array stores the x location of an item on the open list
int openY[MAP_WIDTH*MAP_HEIGHT+2]; //1d array stores the y location of an item on the open list
int parentX[MAP_WIDTH+1][MAP_HEIGHT+1]; //2d array to store parent of each cell (x)
int parentY[MAP_WIDTH+1][MAP_HEIGHT+1]; //2d array to store parent of each cell (y)
int F_cost[MAP_WIDTH*MAP_HEIGHT+2];  //1d array to store F cost of a cell on the open list
int G_cost[MAP_WIDTH+1][MAP_HEIGHT+1];   //2d array to store G_cost cost for each cell.
int H_cost[MAP_WIDTH*MAP_HEIGHT+2];  //1d array to store H cost of a cell on the open list
int pathLength;     //stores length of the FOUND path for critter
int pathLocation;   //stores current position along the chosen path for critter    
int* path_bank ;

//Path reading variables
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
PATH_NODE *find_path(int pathfinderID, int s_x, int s_y, int e_x, int e_y) {
  int onOpenList=0, parentXval=0, parentYval=0,
  a=0, b=0, m=0, u=0, v=0, temp=0, corner=0, numberOfOpenListItems=0,
  addedGCost=0, tempG = 0, path = 0, x=0, y=0,
  tempx, pathX, pathY, cellPosition,
  newOpenListItemID=0;

    // If starting location and target are in the same location...
  if (s_x==e_x && s_y==e_y && pathLocation>0)return NULL;
  else if (s_x==e_x && s_y==e_y && pathLocation==0)return NULL;

   // If dest tile is NOT_WALKABLE, return that it's a NOT_FOUND path.
  if (tiledMap.getPathWalk(e_x, e_y) == NOT_WALKABLE) {
    xPath = s_x;
    yPath = s_y;
    return NULL;
  }

    // Reset some variables that need to be cleared
  for(x=0;x<MAP_WIDTH;x++) {
        for(y=0;y<MAP_HEIGHT;y++)
      whichList [x][y] = 0;
  }
  onClosedList = 2; //changing the values of onOpenList and onClosed list is faster than redimming whichList() array
  onOpenList = 1;
  pathLength = NOT_STARTED;
  pathLocation = NOT_STARTED;
  G_cost[s_x][s_y] = 0; //reset starting square's G_cost value to 0

    // Add the starting location to the open list of tiles to be checked.
  numberOfOpenListItems = 1;
  openList[1] = 1;//assign it as the top (and currently only) item in the open list, which is maintained as a binary heap (explained below)
  openX[1] = s_x ; openY[1] = s_y;

    // Do the following until a path is FOUND or deemed NOT_FOUND.
  do   {

        // If the open list is not empty, take the first cell off of the list.
        // This is the lowest F cost cell on the open list.
        if (numberOfOpenListItems != 0) {

            // Pop the first item off the open list.
            parentXval = openX[openList[1]];
            parentYval = openY[openList[1]]; //record cell coordinates of the item
            whichList[parentXval][parentYval] = onClosedList;//add the item to the closed list

            // Open List = Binary Heap: Delete this item from the open list, which
            numberOfOpenListItems = numberOfOpenListItems - 1;//reduce number of open list items by 1  

            // Delete the top item in binary heap and reorder the heap, with the lowest F cost item rising to the top.
            openList[1] = openList[numberOfOpenListItems+1];//move the last item in the heap up to slot #1
            v = 1;

            // Repeat the following until the new item in slot #1 sinks to its proper spot in the heap.
            do {
                u = v;    
        if (2*u+1 <= numberOfOpenListItems) { //if both children exist
                    //Check if the F cost of the parent is greater than each child.
                    //Select the lowest of the two children.
                    if(F_cost[openList[u]] >= F_cost[openList[2*u]])v = 2*u;
                    if(F_cost[openList[v]] >= F_cost[openList[2*u+1]])v = 2*u+1;
        } else {
          if (2*u <= numberOfOpenListItems) { //if only child #1 exists
                        //Check if the F cost of the parent is greater than child #1
                        if (F_cost[openList[u]] >= F_cost[openList[2*u]])v = 2*u;
                    }
                }

                if (u!=v) { // if parent's F is > one of its children, swap them
                    temp = openList[u];
                    openList[u] = openList[v];
                    openList[v] = temp;
                } else break; //otherwise, exit loop
            } while (u!=v); //reorder the binary heap


//  Check the adjacent squares. (Its "children" -- these path children
//  are similar, conceptually, to the binary heap children mentioned
//  above, but don't confuse them. They are different. Path children
//  are portrayed in Demo 1 with grey pointers pointing toward
//  their parents.) Add these adjacent child squares to the open list
//  for later consideration if appropriate (see various if statements
//  below).

            for (b = parentYval - 1; b <= parentYval + 1; b++) {
                for (a = parentXval - 1; a <= parentXval + 1; a++) {
                    //  If not off the map (do this first to avoid array out-of-bounds errors)
                    if (a!=-1 && b!=-1 && a!=MAP_WIDTH && b!=MAP_HEIGHT) {
                        //  If not already on the closed list (items on the closed list have
                        //  already been considered and can now be ignored).
                        if (whichList[a][b]!=onClosedList) { 
                            //  If not a wall/obstacle square.
                            if (tiledMap.getPathWalk(a, b) != NOT_WALKABLE) { 
                                //  Don't cut across corners
                                corner = WALKABLE;  
                                if (a == parentXval-1) {
                                    if (b == parentYval-1) {
                                        if (tiledMap.getPathWalk(parentXval-1, parentYval)==NOT_WALKABLE || tiledMap.getPathWalk(parentXval, parentYval-1)==NOT_WALKABLE) // cera slash
                      corner = NOT_WALKABLE;
                                    } else if (b==parentYval+1) {
                                        if (tiledMap.getPathWalk(parentXval, parentYval+1)==NOT_WALKABLE || tiledMap.getPathWalk(parentXval-1, parentYval)==NOT_WALKABLE) 
                                            corner = NOT_WALKABLE; 
                                    }
                                } else if (a == parentXval+1) {
                                    if (b == parentYval-1) {
                                        if (tiledMap.getPathWalk(parentXval, parentYval-1)==NOT_WALKABLE || tiledMap.getPathWalk(parentXval+1, parentYval)==NOT_WALKABLE)
                                            corner = NOT_WALKABLE;
                                    } else if (b==parentYval+1) {
                                        if (tiledMap.getPathWalk(parentXval+1, parentYval)==NOT_WALKABLE || tiledMap.getPathWalk(parentXval, parentYval+1)==NOT_WALKABLE)
                                            corner = NOT_WALKABLE; 
                                    }
                                }

                                if(corner==WALKABLE) {
                                    // If not already on the open list, add it to the open list.      
                                    if (whichList[a][b]!=onOpenList) {  
                                        // Create a new open list item in the binary heap.
                                        newOpenListItemID = newOpenListItemID + 1; //each new item has a unique ID #
                                        m = numberOfOpenListItems+1;
                                        openList[m] = newOpenListItemID;//place the new open list item (actually, its ID#) at the bottom of the heap
                                        openX[newOpenListItemID] = a;
                                        openY[newOpenListItemID] = b;//record the x and y coordinates of the new item

                                        //Figure out its G_cost cost
                                        if (abs(a-parentXval) == 1 && abs(b-parentYval) == 1)addedGCost = 14;//cost of going to diagonal squares  
                                        else addedGCost = 10;//cost of going to non-diagonal squares        
                                        G_cost[a][b] = G_cost[parentXval][parentYval] + addedGCost;

                                        //Figure out its H and F costs and parent
                                        H_cost[openList[m]] = 10*(abs(a - e_x) + abs(b - e_y));
                                        F_cost[openList[m]] = G_cost[a][b] + H_cost[openList[m]];
                                        parentX[a][b] = parentXval ; parentY[a][b] = parentYval;

                                        //Move the new open list item to the proper place in the binary heap.
                                        //Starting at the bottom, successively compare to parent items,
                                        //swapping as needed until the item finds its place in the heap
                                        //or bubbles all the way to the top (if it has the lowest F cost).
                    while(m!=1) { // While item hasn't bubbled to the top (m=1)  
                                            //Check if child's F cost is < parent's F cost. If so, swap them.  
                                            if(F_cost[openList[m]]<=F_cost[openList[m/2]]) {
                                                temp = openList[m/2];
                                                openList[m/2] = openList[m];
                                                openList[m] = temp;
                                                m = m/2;
                      } else break;
                    }

                                        numberOfOpenListItems = numberOfOpenListItems+1;//add one to the number of items in the heap
                                        //Change whichList to show that the new item is on the open list.
                                        whichList[a][b] = onOpenList;
                  } else { // If whichList(a,b) = onOpenList
                                    // If adjacent cell is already on the open list, check to see if this 
                                    // path to that cell from the starting location is a better one. 
                                    // If so, change the parent of the cell and its G_cost and F costs.  
                                    //Figure out the G_cost cost of this possible new path
                                        if(abs(a-parentXval)==1 && abs(b-parentYval)==1)addedGCost = 14;//cost of going to diagonal tiles  
                                        else addedGCost = 10;//cost of going to non-diagonal tiles        

                                        tempG = G_cost[parentXval][parentYval] + addedGCost;

                                        // If this path is shorter (G_cost cost is lower) then change
                                        // the parent cell, G_cost cost and F cost.     
                    if(tempG<G_cost[a][b]) { //if G_cost cost is less,
                                            parentX[a][b] = parentXval; //change the square's parent
                                            parentY[a][b] = parentYval;
                                            G_cost[a][b] = tempG;//change the G_cost cost      

                                            // Because changing the G_cost cost also changes the F cost, if
                                            // the item is on the open list we need to change the item's
                                            // recorded F cost and its position on the open list to make
                                            // sure that we maintain a properly ordered open list.

                      for(int x=1;x<=numberOfOpenListItems;x++) { //look for the item in the heap
                        if(openX[openList[x]]==a && openY[openList[x]]==b) { //item FOUND
                                                    F_cost[openList[x]] = G_cost[a][b] + H_cost[openList[x]];//change the F cost
                                                    //See if changing the F score bubbles the item up from it's current location in the heap
                                                    m = x;
                          while(m!=1) { //While item hasn't bubbled to the top (m=1)  
                                                        //Check if child is < parent. If so, swap them.  
                            if(F_cost[openList[m]]<F_cost[openList[m/2]]) {
                                                            temp = openList[m/2];
                                                            openList[m/2] = openList[m];
                                                            openList[m] = temp;
                                                            m = m/2;
                            } else break;
                          }
                                                    break; //exit for x = loop
                        } // If openX(openList(x)) = a
                      } // For x = 1 To numberOfOpenListItems
                    } // If tempG < G_cost(a,b)
                  } // else If whichList(a,b) = onOpenList  
                } // If not cutting a corner
              } // If not a wall/obstacle square.
            } // If not already on the closed list 
          } // If not off the map
                } // for (a = parentXval-1; a <= parentXval+1; a++){
            } // for (b = parentYval-1; b <= parentYval+1; b++){
    } else {// if (numberOfOpenListItems != 0)
            // If open list is empty then there is no path.  
            path = NOT_FOUND;
      break;
    }
        //If target is added to open list then path has been FOUND.
        if (whichList[e_x][e_y]==onOpenList) {
            path = FOUND;
      break;
        }

    } while (path!=FOUND && path!=NOT_FOUND);//Do until path is FOUND or deemed NOT_FOUND

    // Save the path if it exists.
  if (path == FOUND) {

        // Working backwards from the target to the starting location by checking
        // each cell's parent, figure out the length of the path.
        pathX = e_x; pathY = e_y;
        do {
            //Look up the parent of the current cell.  
            tempx = parentX[pathX][pathY];    
            pathY = parentY[pathX][pathY];
            pathX = tempx;

            //Figure out the path length
            pathLength = pathLength + 1;
        } while (pathX != s_x || pathY != s_y);

        // Resize the data bank to the right size in bytes
        path_bank = (int*) realloc (path_bank, pathLength*8);

        // Now copy the path information over to the databank. Since we are
    // working backwards from the target to the start location, we copy
        // the information to the data bank in reverse order. The result is
        // a properly ordered set of path data, from the first step to the last.
        pathX = e_x ; pathY = e_y;
        cellPosition = pathLength*2;//start at the end  
        do {
            cellPosition = cellPosition - 2;//work backwards 2 integers
            path_bank [cellPosition] = pathX;
            path_bank [cellPosition+1] = pathY;
            // Look up the parent of the current cell.  
            tempx = parentX[pathX][pathY];    
            pathY = parentY[pathX][pathY];
            pathX = tempx;
            // If we have reached the starting square, exit the loop.  
        } while(pathX!=s_x || pathY!=s_y);

		char stringa[80];
		sprintf(stringa,"%i %i",s_x,s_y);

    PATH_NODE *ret = NULL, *temp = NULL;
    pathLocation = 1;
    ret = new PATH_NODE(s_x, s_y);
    temp = ret;
    //alert(stringa,"","","","",0,0);
    while(pathLocation<pathLength) {
			sprintf(stringa,"%i %i",path_bank[pathLocation*2-2], path_bank[pathLocation*2-1]);
      //alert(stringa,"","","","",0,0);
      temp->next = new PATH_NODE(
          path_bank[pathLocation * 2 - 2],
          path_bank[pathLocation * 2 - 1]);
      if(temp->next==NULL) throw "Unable to create path node";
      temp = temp->next;
      pathLocation++;
    }
    if(temp!=NULL)temp->next = new PATH_NODE(e_x, e_y);
    else throw "Null reference";
    return ret;

    }
  return NULL; // Path not found
}

/** Read the path data */
void ReadPath(int pathfinderID) {
    //If a path exists, read the path data
    //  from the pathbank.
    pathLocation = 1; //set pathLocation to 1st step
    while (pathLocation<pathLength) {
        int a = path_bank [pathLocation*2-2];
        int b = path_bank [pathLocation*2-1];
        pathLocation = pathLocation + 1;
        whichList[a][b] = 3;//draw dotted path
    } 
}
