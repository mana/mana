/*
 *  TMXCopy
 *  Copyright (C) 2007  Philipp Sehmisch
 *  Copyright (C) 2009  Steve Cotton
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <iostream>
#include <string>
#include <unistd.h>

#include "map.hpp"

void printUsage()
{
    std::cerr<<"Usage: tmxcopy [-c] [-n] srcFile x y width height tgtFile x y [outfile]"<<std::endl
             <<"    -c create layers, if they don't already exist in the target"<<std::endl
             <<"    -n copy layers by number, not name"<<std::endl;
}

int main(int argc, char * argv[] )
{
    ConfigurationOptions config = {0};
    int opt;
    while ((opt = getopt(argc, argv, "cn")) != -1)
    {
        switch (opt)
        {
            case 'c':
                config.createMissingLayers = true;
                break;
            case 'n':
                config.copyLayersByOrdinal = true;
                break;
            case '?':
                std::cerr<<"Unrecognized option"<<std::endl;
                printUsage();
                return -1;
        }
    }

    if ((argc-optind) < 8)
    {
        std::cerr<<"Too few args"<<std::endl;
        printUsage();
        return -1;
    }
    if ((argc-optind) > 9)
    {
        std::cerr<<"Too many args"<<std::endl;
        printUsage();
        return -1;
    }

    std::string srcFile = argv[optind+0];
    int srcX= atoi(argv[optind+1]);
    int srcY= atoi(argv[optind+2]);
    int width= atoi(argv[optind+3]);
    int height=atoi(argv[optind+4]);
    std::string tgtFile = argv[optind+5];
    int destX=atoi(argv[optind+6]);
    int destY=atoi(argv[optind+7]);
    std::string outFile = tgtFile;
    if (argc == optind+9)
    {
        outFile = argv[optind+8];
    }

    // plausibility check of command line options
    if (height < 1 || width < 1 || srcX < 0 || srcY < 0 || destX < 0 || destY < 0)
    {
        std::cerr<<"Illegal coordinates!"<<std::endl;
        printUsage();
        return -1;
    }

    try
    {
        Map* srcMap = new Map(srcFile);
        Map* tgtMap = new Map(tgtFile);
        if (tgtMap->overwrite(srcMap, srcX, srcY, width, height, destX, destY, config))
        {
            tgtMap->save(outFile);
        } else {
            return -1;
        }
        delete srcMap;
        delete tgtMap;
    }
    catch (int)
    {
        return -1;
    }
}
