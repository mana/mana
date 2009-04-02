/*
 *  TMXRandomFill
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
    std::cerr<<"Usage: tmx_random_fill [-c] mapFile destLayer x y width height templateFile [-o outfile]"<<std::endl
             <<"    -c create layers, if they don't already exist in the target"<<std::endl
             <<"    -o save results to outfile, instead of overwriting the original"<<std::endl
             <<std::endl
             <<"Fill a rectangular area of mapFile's layer 'destLayer' with a random selection from the templateFile"<<std::endl
             <<"TODO - more help here"<<std::endl;
}

int main(int argc, char * argv[] )
{
    ConfigurationOptions config = {0};
    std::string outFile;

    int opt;
    while ((opt = getopt(argc, argv, "co:")) != -1)
    {
        switch (opt)
        {
            case 'c':
                config.createMissingLayers = true;
                break;
            case 'o':
                if (optarg)
                {
                    outFile = optarg;
                } else {
                    printUsage();
                    return -1;
                }
                break;
            case '?':
                std::cerr<<"Unrecognized option"<<std::endl;
                printUsage();
                return -1;
        }
    }

    if ((argc-optind) < 7)
    {
        std::cerr<<"Too few args"<<std::endl;
        printUsage();
        return -1;
    }
    if ((argc-optind) > 7)
    {
        std::cerr<<"Too many args"<<std::endl;
        printUsage();
        return -1;
    }

    std::string mapFile = argv[optind+0];
    std::string destLayer = argv[optind+1];
    int destX= atoi(argv[optind+2]);
    int destY= atoi(argv[optind+3]);
    int width= atoi(argv[optind+4]);
    int height=atoi(argv[optind+5]);
    std::string templateFile = argv[optind+6];
    if (outFile.empty())
    {
        outFile = mapFile;
    }

    // plausibility check of command line options
    if (height < 1 || width < 1 || destX < 0 || destY < 0)
    {
        std::cerr<<"Illegal coordinates!"<<std::endl;
        printUsage();
        return -1;
    }

    try
    {
        Map* mainMap = new Map(mapFile);
        Map* templateMap = new Map(templateFile);
        if (mainMap->randomFill(templateMap, destLayer, destX, destY, width, height, config))
        {
            mainMap->save(outFile);
        } else {
            return -1;
        }
        delete mainMap;
        delete templateMap;
    }
    catch (int)
    {
        return -1;
    }
}
