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

#ifndef _TMW_ITEMINFO_H
#define _TMW_ITEMINFO_H

#include <string>

/**
 * Defines a class for storing item infos.
 */
class ItemInfo
{
    public:
        /**
         * Constructor.
         */
        ItemInfo();


        void setImage(short image);
        
        short getImage();
        
        void setArt(short art);
        
        short getArt();
        
        void setName(const std::string &name);
        
        std::string getName();
        
        void setDescription(const std::string &description);

        std::string getDescription();

        void setEffect(const std::string &effect);

        std::string getEffect();

        void setType(short type);

        short getType();
        
        void setWeight(short weight);

        short getWeight();
        
        void setSlot(char slot);
        
        char getSlot();


    protected:
        /**
         * Destructor.
         */
        ~ItemInfo();
        
        short image, art;
        std::string name;
        std::string description;
        std::string effect;
        short type, weight;
        char slot;
        

};

#endif
