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
#include <math.h>

/**
    \brief read INI file and parse all options into memory
    \param filename full path to INI file (~/.manaworld/tmw.ini)

    NOTE:
        first a line is checked wether it is a comment or not by
        looking for INI_COMMENTER. after this another check is
        done for INI_DELIMITER if the previous check failed.
        if this line is a valid option all spaces in it get
        stripped away (including the value) and it is added to
        the list iniOptions.
*/
void Configuration::init(std::string filename) {
    std::ifstream inFile(filename.c_str(), std::ifstream::in);
    std::string inBuffer;
    unsigned int position;
    INI_OPTION optionTmp;

    iniOptions.clear();

    while (inFile.good()) {
        getline(inFile, inBuffer, '\n');

        if (inBuffer.substr(0, 1) != INI_COMMENTER) {
            position = inBuffer.find(INI_DELIMITER, 0);

            if (position != std::string::npos) {
                // replace spaces with void :)
                while (inBuffer.find(" ", 0) != std::string::npos) {
                    inBuffer.replace(inBuffer.find(" ", 0), 1, "");
                }
                std::string key = inBuffer.substr(0, position);
                optionTmp.stringValue  = inBuffer.substr(position + 1,
                        inBuffer.length());
                inBuffer = inBuffer.substr(position+1, inBuffer.length());

                optionTmp.numericValue = atof(inBuffer.c_str());

                iniOptions[key] = optionTmp;
                
                #ifdef __DEBUG                
                    std::cout << "Configuration::init(" << key << ", \"" <<
                        optionTmp.stringValue << "\" / " <<
                        optionTmp.numericValue << ")\n";
                #endif
            }
        }
    }
    inFile.close();
}

/**
    \brief write the current settings back to an ini-file
    \param filename full path to INI file (~/.manaworld/tmw.ini)
*/
bool Configuration::write(std::string filename) {
    std::ofstream out(filename.c_str(),
            std::ofstream::out | std::ofstream::trunc);
    char tmp[20];

    INI_OPTION optionTmp;
    for (iter = iniOptions.begin(); iter != iniOptions.end(); iter++) {
        out.write(iter->first.c_str(), iter->first.length());
        out.write("=", 1);

        double numericValue = iter->second.numericValue;

        if (numericValue == 0) {
            out.write(iter->second.stringValue.c_str(),
                    iter->second.stringValue.length());
        }
        else if (numericValue == floor(numericValue)) {
            out << (int)numericValue;
        }
        else {
            sprintf(tmp, "%f", numericValue);
            out.write(tmp, strlen(tmp));
            strcpy(tmp, "");
        }
        #ifdef __DEBUG
            std::cout << "Configuration::write(" << iter->first.c_str() <<
                ", \"" << iter->second.stringValue << "\" / " <<
                iter->second.numericValue << ")\n";
        #endif
        out.write("\n", 1);
    }

    out.close();
    return true;
}

/**
    \brief set an option using a string value
    \param key option identifier
    \param value value
*/
void Configuration::setValue(std::string key, std::string value) {
    INI_OPTION optionTmp;
    #ifdef __DEBUG
        std::cout << "Configuration::setValue(" << key << ", " << value <<
            ")\n";
    #endif
    optionTmp.stringValue = value;
    optionTmp.numericValue = 0;    
    iniOptions[key] = optionTmp;
}

/**
    \brief set an option using a numeric value
    \param key option identifier
    \param value value
*/
void Configuration::setValue(std::string key, float value) {
    INI_OPTION optionTmp;
    #ifdef __DEBUG
        std::cout << "Configuration::setValue(" << key << ", " << value <<
            ")\n";
    #endif
    optionTmp.stringValue = "";
    optionTmp.numericValue = value;    
    iniOptions[key] = optionTmp;
}

/**
    \brief get a value as string
    \param key option identifier
    \param deflt default option if not there or error
*/
std::string Configuration::getValue(std::string key, std::string deflt) {
    iter = iniOptions.find(key);
    if (iter != iniOptions.end()) {
        return iniOptions[key].stringValue;
    }
    return deflt;
}

/**
    \brief get a value as numeric (float)
    \param key option identifier
    \param deflt default option if not there or error
*/
float Configuration::getValue(std::string key, float deflt) {
    iter = iniOptions.find(key);
    if (iter != iniOptions.end()) {
        return iniOptions[key].numericValue;
    }
    return deflt;
}
