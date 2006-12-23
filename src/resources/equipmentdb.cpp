/*
 *  The Mana World
 *  Copyright 2006 The Mana World Development Team
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
 *  $Id:
 */

#include "equipmentdb.h"

#include "resourcemanager.h"

#include "../log.h"

#include "../utils/dtor.h"
#include "../utils/xml.h"

namespace
{
    EquipmentDB::EquipmentInfos mEquipmentInfos;
    EquipmentInfo mUnknown;
    bool mLoaded = false;
}

void
EquipmentDB::load()
{
    if (mLoaded)
        return;

    logger->log("Initializing equipment database...");
    mUnknown.setSprite("error.xml", 0);
    mUnknown.setSprite("error.xml", 1);

    ResourceManager *resman = ResourceManager::getInstance();
    int size;
    char *data = (char*)resman->loadFile("equipment.xml", size);

    if (!data)
    {
        logger->error("Equipment Database: Could not find equipment.xml!");
    }

    xmlDocPtr doc = xmlParseMemory(data, size);
    free(data);

    if (!doc)
    {
        logger->error("Equipment Database: Error while parsing equipment database (equipment.xml)!");
    }

    xmlNodePtr rootNode = xmlDocGetRootElement(doc);
    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "equipments"))
    {
        logger->error("Equipment Database: equipment.xml is not a valid database file!");
    }

    //iterate <equipment>s
    for (   xmlNodePtr equipmentNode = rootNode->xmlChildrenNode;
            equipmentNode != NULL;
            equipmentNode = equipmentNode->next)
    {

        if (!xmlStrEqual(equipmentNode->name, BAD_CAST "equipment"))
        {
            continue;
        }

        EquipmentInfo *currentInfo = new EquipmentInfo();

        currentInfo->setSlot (XML::getProperty(equipmentNode, "slot", 0));

        //iterate <sprite>s
        for (   xmlNodePtr spriteNode = equipmentNode->xmlChildrenNode;
                spriteNode != NULL;
                spriteNode = spriteNode->next)
        {
            if (!xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
            {
                continue;
            }

            std::string gender = XML::getProperty(spriteNode, "gender", "unisex");
            std::string filename = (const char*) spriteNode->xmlChildrenNode->content;

            if (gender == "male" || gender == "unisex")
            {
                currentInfo->setSprite(filename, 0);
            }

            if (gender == "female" || gender == "unisex")
            {
                currentInfo->setSprite(filename, 1);
            }
        }

        setEquipment(   XML::getProperty(equipmentNode, "id", 0),
                        currentInfo);
    }

    mLoaded = true;
}

void
EquipmentDB::unload()
{
    // kill EquipmentInfos
    for_each (  mEquipmentInfos.begin(), mEquipmentInfos.end(),
                make_dtor(mEquipmentInfos));
    mEquipmentInfos.clear();

    mLoaded = false;
}

EquipmentInfo*
EquipmentDB::get(int id)
{
    if (!mLoaded) {
        logger->error("Error: Equipment database used before initialization!");
    }

    EquipmentInfoIterator i = mEquipmentInfos.find(id);

    if (i == mEquipmentInfos.end() )
    {
        logger->log("EquipmentDB: Error, unknown equipment ID# %d", id);
        return &mUnknown;
    }
    else
    {
        return i->second;
    }
}

void
EquipmentDB::setEquipment(int id, EquipmentInfo* equipmentInfo)
{
    if (mEquipmentInfos.find(id) != mEquipmentInfos.end()) {
        logger->log("Warning: Equipment Piece with ID %d defined multiple times",
                    id);
        delete equipmentInfo;
    }
    else {
        mEquipmentInfos[id] = equipmentInfo;
    };
}
