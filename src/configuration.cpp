/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */


#include "configuration.h"
#include "log.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/tree.h>

void Configuration::init(std::string filename)
{
    //xmlDocPtr doc = xmlReadFile(filename.c_str(), NULL, 0);
    std::ifstream inFile(filename.c_str(), std::ifstream::in);
    std::string inBuffer;
    unsigned int position;

    options.clear();

    while (inFile.good())
    {
        std::getline(inFile, inBuffer, '\n');

        if (inBuffer.substr(0, 1) != INI_COMMENTER)
        {
            // Replace spaces with void
            while (inBuffer.find(" ", 0) != std::string::npos) {
                inBuffer.replace(inBuffer.find(" ", 0), 1, "");
            }

            position = inBuffer.find(INI_DELIMITER, 0);

            if (position != std::string::npos)
            {
                std::string key = inBuffer.substr(0, position);

                if (inBuffer.length() > position + 1)
                {
                    options[key] =
                        inBuffer.substr(position + 1, inBuffer.length());
                }

                log("Configuration::init(%s, \"%s\")",
                        key.c_str(), options[key].c_str());
            }
        }
    }

    inFile.close();
}

bool Configuration::write(std::string filename)
{
    std::map<std::string, std::string>::iterator iter;
    std::ofstream out(filename.c_str(),
            std::ofstream::out | std::ofstream::trunc);

    for (iter = options.begin(); iter != options.end(); iter++)
    {
        log("Configuration::write(%s, \"%s\")",
                iter->first.c_str(), iter->second.c_str());

        out.write(iter->first.c_str(), iter->first.length());
        out.write("=", 1);
        out.write(iter->second.c_str(), iter->second.length());
        out.write("\n", 1);
    }

    out.close();
    return true;
}

void Configuration::setValue(std::string key, std::string value)
{
#ifdef __DEBUG
    std::cout << "Configuration::setValue(" << key << ", " << value << ")\n";
#endif
    options[key] = value;
}

void Configuration::setValue(std::string key, float value)
{
#ifdef __DEBUG
    std::cout << "Configuration::setValue(" << key << ", " << value << ")\n";
#endif
    std::stringstream ss;
    if (value == floor(value)) {
        ss << (int)value;
    } else {
        ss << value;
    }
    options[key] = ss.str();
}

std::string Configuration::getValue(std::string key, std::string deflt)
{
    std::map<std::string, std::string>::iterator iter = options.find(key);
    if (iter != options.end()) {
        return options[key];
    }
    return deflt;
}

float Configuration::getValue(std::string key, float deflt)
{
    std::map<std::string, std::string>::iterator iter = options.find(key);
    if (iter != options.end()) {
        return atof(options[key].c_str());
    }
    return deflt;
}
