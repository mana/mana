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

#include "iteminfo.h"

ItemInfo::ItemInfo() :
    image(0), name(""),
    description(""), type(0),
    weight(0), slot(0)
{
}

ItemInfo::~ItemInfo()
{
}

void ItemInfo::setImage(short image)
{
    this->image = image;
}

short ItemInfo::getImage()
{
    return image;
}

void ItemInfo::setArt(short art)
{
    this->art = art;
}

short ItemInfo::getArt()
{
    return art;
}

void ItemInfo::setName(const std::string &name)
{
    this->name = name;
}

std::string ItemInfo::getName()
{
    return name;
}

void ItemInfo::setDescription(const std::string &description)
{
    this->description = description;
}

std::string ItemInfo::getDescription()
{
    return description;
}

void ItemInfo::setEffect(const std::string &effect)
{
    this->effect = effect;
}

std::string ItemInfo::getEffect()
{
    return effect;
}

void ItemInfo::setType(short type)
{
    this->type = type;
}

short ItemInfo::getType()
{
    return type;
}

void ItemInfo::setWeight(short weight)
{
    this->weight = weight;
}

short ItemInfo::getWeight()
{
    return weight;
}

void ItemInfo::setSlot(char slot)
{
    this->slot = slot;
}

char ItemInfo::getSlot()
{
    return slot;
}
