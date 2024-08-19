/*
 *  The Mana Server
 *  Copyright (C) 2013  The Mana World Development Team
 *
 *  This file is part of The Mana Server.
 *
 *  The Mana Server is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana Server is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana Server.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "resources/settingsmanager.h"

#include "resources/attributes.h"
#include "resources/emotedb.h"
#include "resources/hairdb.h"
#include "resources/itemdb.h"
#include "resources/monsterdb.h"
#include "resources/npcdb.h"
#include "resources/abilitydb.h"
#include "resources/statuseffectdb.h"

#include "net/net.h"

#include "utils/xml.h"
#include "utils/path.h"

#include "configuration.h"
#include "log.h"
#include "units.h"

namespace SettingsManager
{
    static std::string mSettingsFile;
    static std::set<std::string> mIncludedFiles;

    static bool loadFile(const std::string &filename);

    void load()
    {
        // initialize managers
        paths.clear();
        Attributes::init();
        hairDB.init();
        itemDb->init();
        MonsterDB::init();
        AbilityDB::init();
        NPCDB::init();
        EmoteDB::init();
        StatusEffectDB::init();
        Units::init();

        // load stuff from settings
        if (!loadFile("settings.xml"))
        {
            // fall back to loading certain known files for TMW compatibility
            loadFile("paths.xml");
            loadFile("items.xml");
            loadFile("monsters.xml");
            loadFile("npcs.xml");
            loadFile("emotes.xml");
            loadFile("status-effects.xml");
            loadFile("itemcolors.xml");
            loadFile("units.xml");
        }

        Attributes::checkStatus();
        hairDB.checkStatus();
        itemDb->checkStatus();
        MonsterDB::checkStatus();
        AbilityDB::checkStatus();
        NPCDB::checkStatus();
        EmoteDB::checkStatus();
        StatusEffectDB::checkStatus();
        Units::checkStatus();

        if (Net::getNetworkType() == ServerType::MANASERV)
        {
            Attributes::informItemDB();
        }
    }

    void unload()
    {
        StatusEffectDB::unload();
        EmoteDB::unload();
        NPCDB::unload();
        AbilityDB::unload();
        MonsterDB::unload();
        if (itemDb)
            itemDb->unload();
        hairDB.unload();
        Attributes::unload();
    }

    /**
     * Loads a settings file.
     */
    static bool loadFile(const std::string &filename)
    {
        logger->log("Loading game settings from %s", filename.c_str());

        XML::Document doc(filename);
        XML::Node node = doc.rootNode();

        // add file to include set
        mIncludedFiles.insert(filename);

        // FIXME: check root node's name when bjorn decides it's time
        if (!node /*|| node.name() != "settings" */)
        {
            logger->log("Settings Manager: %s is not a valid settings file!", filename.c_str());
            return false;
        }

        if (node.name() == "monsters")
        {
            if (node.hasAttribute("offset"))
            {
                MonsterDB::setMonsterIdOffset(node.getProperty("offset", 0));
            }
        }

        // go through every node
        for (auto childNode : node.children())
        {
            if (childNode.name() == "include")
            {
                // include an other file
                std::string includeFile = childNode.getProperty("file", std::string());

                if (!includeFile.empty())
                {
                    // build absolute path
                    const auto path = utils::path(filename);
                    includeFile = utils::cleanPath(utils::joinPaths(path, includeFile));
                }
                else
                {
                    // try to get name property, which has an absolute value
                    includeFile = childNode.getProperty("name", std::string());
                }

                // check if file property was given
                if (!includeFile.empty())
                {
                    // check if we're not entering a loop
                    if (mIncludedFiles.find(includeFile) != mIncludedFiles.end())
                    {
                        logger->log("Warning: Circular include loop detecting while including %s from %s", includeFile.c_str(), filename.c_str());
                    }
                    else
                    {
                        loadFile(includeFile);
                    }
                }
                else
                {
                    logger->log("Warning: <include> element without 'file' or 'name' attribute in %s", filename.c_str());
                }
            }
            else if (childNode.name() == "option")
            {
                // options from paths.xml
                std::string name = childNode.getProperty("name", std::string());
                std::string value = childNode.getProperty("value", std::string());

                if (!name.empty())
                    paths.setValue(name, value);
                else
                    logger->log("Warning: option without a name found in %s", filename.c_str());
            }
            else if (childNode.name() == "attribute")
            {
                // map config
                Attributes::readAttributeNode(childNode, filename);
            }
            else if (childNode.name() == "points")
            {
                Attributes::readPointsNode(childNode, filename);
            }
            else if (childNode.name() == "color")
            {
                hairDB.readHairColorNode(childNode, filename);
            }
            else if (childNode.name() == "list")
            {
                // todo: consider if we need a "color DB", but in tmwa clientdata
                // I only see hair colors in the itemcolors.xml file.
                const std::string name = childNode.getProperty("name", std::string());
                if (name == "hair")
                {
                    for (auto hairColorNode : childNode.children())
                    {
                        if (hairColorNode.name() == "color")
                            hairDB.readHairColorNode(hairColorNode, filename);
                    }
                }
            }
            else if (childNode.name() == "item")
            {
                itemDb->readItemNode(childNode, filename);
            }
            else if (childNode.name() == "monster")
            {
                MonsterDB::readMonsterNode(childNode, filename);
            }
            else if (childNode.name() == "ability-category")
            {
                AbilityDB::readAbilityCategoryNode(childNode, filename);
            }
            else if (childNode.name() == "npc")
            {
                NPCDB::readNPCNode(childNode, filename);
            }
            else if (childNode.name() == "emote")
            {
                EmoteDB::readEmoteNode(childNode, filename);
            }
            else if (childNode.name() == "status-effect")
            {
                StatusEffectDB::readStatusEffectNode(childNode, filename);
            }
            else if (childNode.name() == "unit")
            {
                Units::readUnitNode(childNode, filename);
            }
        }

        mIncludedFiles.erase(filename);
        return true;
    }
}
