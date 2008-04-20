/*
 *  TMXCopy
 *  Copyright 2007 Philipp Sehmisch
 *
 *
 *  TMXCopy is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  TMXCopy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TMXCopy; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <iostream>
#include <string>

#include "map.hpp"

int main(int argc, char * argv[] )
{
    // parsing command line options
    if (argc < 9 || argc > 10)
    {
        std::cerr<<"Usage: srcFile x y width height tgtFile x y [outfile]";
        return -1;
    }

    std::string srcFile = argv[1];
    int srcX= atoi(argv[2]);
    int srcY= atoi(argv[3]);
    int width= atoi(argv[4]);
    int height=atoi(argv[5]);
    std::string tgtFile = argv[6];
    int destX=atoi(argv[7]);
    int destY=atoi(argv[8]);
    std::string outFile = tgtFile;
    if (argc == 10) outFile = argv[9];

    // plausibility check of command line options
    if (height < 1 || width < 1 || srcX < 0 || srcY < 0 || destX < 0 || destY < 0)
    {
        std::cerr<<"Illegal coordinates!"<<std::endl;
        std::cerr<<"Usage: sourceFile x y height width targetFile x y [outputFile]"<<std::endl;
        return -1;
    }

    try
    {
        Map* srcMap = new Map(srcFile);
        Map* tgtMap = new Map(tgtFile);
        if (tgtMap->overwrite(srcMap, srcX, srcY, width, height, destX, destY))
        {
            tgtMap->save(outFile);
        } else {
            return -1;
        }
    }
    catch (int)
    {
        return -1;
    }

}
