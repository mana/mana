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
 */

#ifndef __INIREAD_H
#define __INIREAD_H

#define INI_DELIMITER "="
#define INI_COMMENTER "#"

#include <list>
#include <string>
#include <iostream>
#include <fstream>

/**
 * INI configuration handler for reading (and writing)
 *
 * \ingroup CORE
 */
class Configuration {
    public:
        void Init(std::string);

        bool Write(std::string);

        void        setValue(std::string, std::string);
        void        setValue(std::string, float);

        std::string getValue(std::string, std::string);
        float       getValue(std::string, float);
    private:
        std::ifstream inFile;

        typedef struct INI_OPTION {
            std::string key;
            std::string stringValue;
            float       numericValue;
        };

        std::list<INI_OPTION> iniOptions;
        std::list<INI_OPTION>::iterator iter;
};

#endif
