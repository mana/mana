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

/**
    \brief read INI file and parse all options into memory
    \param filename full path to INI file (~/.manaworld/tmw.ini)
*/
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

    #ifdef __DEBUG
        for (iter = iniOptions.begin(); iter != iniOptions.end(); iter++) {
            optionTmp = *iter;
            std::cout << "Configuration::Init(" << optionTmp.key << ", \"" << optionTmp.stringValue << "\" / " << optionTmp.numericValue << ")\n";
        }
    #endif
}

bool Configuration::Write(std::string filename) {
    std::ofstream out(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
    char tmp[20];

    INI_OPTION optionTmp;
    for (iter = iniOptions.begin(); iter != iniOptions.end(); iter++) {
        optionTmp = *iter;
        out.write(optionTmp.key.c_str(), optionTmp.key.length());
        out.write("=", 1);

        if(optionTmp.numericValue == 0) {
            out.write(optionTmp.stringValue.c_str(), optionTmp.stringValue.length());
        }else{
            sprintf(tmp, "%f", optionTmp.numericValue);
            out.write(tmp, strlen(tmp));
            strcpy(tmp, "");
        }

        out.write("\n", 1);
    }

    out.close();
    return true;
}

void Configuration::setValue(std::string key, std::string value) {
    if(getValue(key, value) == value) {
        #ifdef __DEBUG
            std::cout << "Configuration::setValue(" << key << ", \"" << value << "\")\n";
        #endif
        INI_OPTION optionTmp;

        optionTmp.key = key;
        optionTmp.stringValue = value;
        optionTmp.numericValue = 0;

        iniOptions.push_back(optionTmp);
    }
}

void Configuration::setValue(std::string key, float value) {
    if(getValue(key, value) == value) {
        #ifdef __DEBUG
            std::cout << "Configuration::setValue(" << key << ", " << value << ")\n";
        #endif

        INI_OPTION optionTmp;

        optionTmp.key = key;
        optionTmp.numericValue = value;

        iniOptions.push_back(optionTmp);
    }
}

/**
    \brief get a value as string
    \param key option identifier
    \param deflt default option if not there or error
*/
std::string Configuration::getValue(std::string key, std::string deflt) {
    INI_OPTION optionTmp;
    for (iter = iniOptions.begin(); iter != iniOptions.end(); iter++) {
        optionTmp = *iter;
        if(optionTmp.key == key)
            return optionTmp.stringValue;
    }

    return deflt;
}

/**
    \brief get a value as numeric (float)
    \param key option identifier
    \param deflt default option if not there or error
*/
float Configuration::getValue(std::string key, float deflt) {
    INI_OPTION optionTmp;
    for (iter = iniOptions.begin(); iter != iniOptions.end(); iter++) {
        optionTmp = *iter;
        if(optionTmp.key == key)
            return optionTmp.numericValue;
    }

    return deflt;
}
