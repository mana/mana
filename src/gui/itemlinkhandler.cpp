/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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

#include <sstream>
#include <string>

#include "itemlinkhandler.h"
#include "itempopup.h"
#include "viewport.h"

#include "../resources/iteminfo.h"
#include "../resources/itemdb.h"

ItemLinkHandler::ItemLinkHandler()
{
    mItemPopup = new ItemPopup;
}

ItemLinkHandler::~ItemLinkHandler()
{
    delete mItemPopup;
}

void ItemLinkHandler::handleLink(const std::string &link)
{
    int id = 0;
    std::stringstream stream;
    stream << link;
    stream >> id;

    if (id > 0)
    {
        const ItemInfo &iteminfo = ItemDB::get(id);

        mItemPopup->setItem(iteminfo);

        if (mItemPopup->isVisible())
            mItemPopup->setVisible(false);
        else
            mItemPopup->view(viewport->getMouseX(), viewport->getMouseY());
    }
}
