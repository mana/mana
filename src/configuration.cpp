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


#include "configuration.h"

void Configuration::Init(std::string filename) {
    inFile.open(filename.c_str(), std::ifstream::in);
    std::string inBuffer;
    int position;
    INI_OPTION optionTmp;

    while (inFile.good()) {
        getline(inFile, inBuffer, '\n');

        if(inBuffer.substr(0,1) != INI_COMMENTER) {
            position = inBuffer.find(INI_DELIMITER, 0);

            if(position > 0 && position >= -1) {
                optionTmp.key = inBuffer.substr(0, position);
                optionTmp.stringValue  = inBuffer.substr(position+1, inBuffer.length());
                inBuffer = inBuffer.substr(position+1, inBuffer.length());

                optionTmp.numericValue = atof(inBuffer.c_str());

                iniOptions.push_back(optionTmp);
            }
        }
    }
    inFile.close();

    for (iter = iniOptions.begin(); iter != iniOptions.end(); iter++) {
        optionTmp = *iter;
        #ifdef __DEBUG
            std::cout << "key=(" << optionTmp.key << ") stringValue=(" << optionTmp.stringValue << ") numericValue=(" << optionTmp.numericValue << ")\n";
        #endif
    }
}

bool Configuration::Write() {
    return true;
}

bool Configuration::setValue(std::string, std::string) {
    return true;
}

std::string Configuration::getValue(std::string key, std::string deflt) {
    INI_OPTION optionTmp;
    for (iter = iniOptions.begin(); iter != iniOptions.end(); iter++) {
        optionTmp = *iter;
        if(optionTmp.key == key)
            return optionTmp.stringValue;
    }

    return deflt;
}

float Configuration::getValue(std::string key, float deflt) {
    INI_OPTION optionTmp;
    for (iter = iniOptions.begin(); iter != iniOptions.end(); iter++) {
        optionTmp = *iter;
        if(optionTmp.key == key)
            return optionTmp.numericValue;
    }

    return deflt;
}
