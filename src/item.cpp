/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "item.h"

#include "configuration.h"
#include "event.h"

#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"

Item::Item(int id, int quantity, bool equipped):
    mQuantity(quantity),
    mEquipped(equipped)
{
    setId(id);
}

void Item::setId(int id)
{
    mId = id;

    // Load the associated image
    ResourceManager *resman = ResourceManager::getInstance();
    const SpriteDisplay &display = getInfo().display;
    mImage = resman->getImage(paths.getStringValue("itemIcons") + display.image);

    if (!mImage)
    {
        mImage = Theme::getImageFromTheme(paths.getValue("unknownItemFile",
                                                         "unknown-item.png"));
    }
}

void Item::doEvent(Event::Type eventName)
{
    Event event(eventName);
    event.setItem("item", this);
    event.trigger(Event::ItemChannel);
}

void Item::doEvent(Event::Type eventName, int amount)
{
    Event event(eventName);
    event.setItem("item", this);
    event.setInt("amount", amount);
    event.trigger(Event::ItemChannel);
}

bool Item::isEquippable() const
{
    return getInfo().equippable;
}
