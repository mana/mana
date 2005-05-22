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

#include "menu.h"

Menu::Menu(const std::string& menulabel, std::vector<MenuItem *> items):
    Window(menulabel)
{
    setContentSize(52,60);
    fill(items);
}

Menu::~Menu()
{
}

void Menu::fill(std::vector<MenuItem *> items)
{   
    int i=0;
        
    for(std::vector<MenuItem *>::iterator item = items.begin();
            item != items.end(); ++item) 
    {
        MenuItem *optr;
        optr = *item;
        optr->setSize(50,10);
        optr->setPosition(0,i);
        i=i+10;
        add(*item);
    }
    
    resizeToContent();
}

