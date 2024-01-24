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

#include "configuration.h"
#include "resources/attributes.h"
#include "resources/hairdb.h"
#include "resources/itemdb.h"
#include "resources/monsterdb.h"
#include "resources/specialdb.h"
#include "resources/npcdb.h"
#include "resources/emotedb.h"
#include "statuseffect.h"
#include "units.h"

#include "net/manaserv/itemhandler.h"
#include "net/net.h"

#include "utils/xml.h"
#include "utils/path.h"
#include "log.h"

namespace SettingsManager
{
    static std::string mSettingsFile;
    static std::set<std::string> mIncludedFiles;

    static void loadFile(const std::string &filename);

    void load()
    {
        // initialize managers
        paths.clear();
        Attributes::init();
        hairDB.init();
        itemDb->init();
        MonsterDB::init();
        SpecialDB::init();
        NPCDB::init();
        EmoteDB::init();
        StatusEffect::init();
        Units::init();

        // load stuff from settings
        loadFile("settings.xml");

        Attributes::checkStatus();
        hairDB.checkStatus();
        itemDb->checkStatus();
        MonsterDB::checkStatus();
        SpecialDB::checkStatus();
        NPCDB::checkStatus();
        EmoteDB::checkStatus();
        StatusEffect::checkStatus();
        Units::checkStatus();

        if (Net::getNetworkType() == ServerInfo::MANASERV)
        {
            Attributes::informItemDB();
        }
    }

    void unload()
    {
        StatusEffect::unload();
        EmoteDB::unload();
        NPCDB::unload();
        SpecialDB::unload();
        MonsterDB::unload();
        if (itemDb)
            itemDb->unload();
        hairDB.unload();
        Attributes::unload();
    }

    /**
     * Loads a settings file.
     */
    static void loadFile(const std::string &filename)
    {
        logger->log("Loading game settings from %s", filename.c_str());

        XML::Document doc(filename);
        xmlNodePtr node = doc.rootNode();

        // add file to include set
        mIncludedFiles.insert(filename);

        // FIXME: check root node's name when bjorn decides it's time
        if (!node /*|| !xmlStrEqual(node->name, BAD_CAST "settings") */)
        {
            logger->log("Settings Manager: %s is not a valid settings file!", filename.c_str());
            return;
        }


        // go through every node
        for_each_xml_child_node(childNode, node)
        {
            if (childNode->type != XML_ELEMENT_NODE)
                continue;

            if (xmlStrEqual(childNode->name, BAD_CAST "include"))
            {
                // include an other file
                const std::string includeFile = XML::getProperty(childNode, "file", std::string());

                // check if file property was given
                if (!includeFile.empty())
                {
                    // build absolute path path
                    const utils::splittedPath splittedPath = utils::splitFileNameAndPath(filename);
                    const std::string realIncludeFile = utils::cleanPath(
                            utils::joinPaths(splittedPath.path, includeFile));

                    // check if we're not entering a loop
                    if (mIncludedFiles.find(realIncludeFile) != mIncludedFiles.end())
                    {
                        logger->log("Circular include loop detecting while including %s from %s", includeFile.c_str(), filename.c_str());
                    }
                    else
                    {
                        // include that file
                        loadFile(realIncludeFile);
                    }
                }
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "option"))
            {
                // options from paths.xml
                std::string name = XML::getProperty(childNode, "name", std::string());
                std::string value = XML::getProperty(childNode, "value", std::string());

                if (!name.empty())
                    paths.setValue(name, value);
                else
                    logger->log("Warning: option without a name found in %s", filename.c_str());
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "attribute"))
            {
                // map config
                Attributes::readAttributeNode(childNode, filename);
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "points"))
            {
                Attributes::readPointsNode(childNode, filename);
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "color"))
            {
                hairDB.readHairColorNode(childNode, filename);
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "item"))
            {
                itemDb->readItemNode(childNode, filename);
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "monster"))
            {
                MonsterDB::readMonsterNode(childNode, filename);
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "special-set"))
            {
                SpecialDB::readSpecialSetNode(childNode, filename);
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "npc"))
            {
                NPCDB::readNPCNode(childNode, filename);
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "emote"))
            {
                EmoteDB::readEmoteNode(childNode, filename);
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "status-effect") || xmlStrEqual(childNode->name, BAD_CAST "stun-effect"))
            {
                StatusEffect::readStatusEffectNode(childNode, filename);
            }
            else if (xmlStrEqual(childNode->name, BAD_CAST "unit"))
            {
                Units::readUnitNode(childNode, filename);
            }
            else
            {
                // compatibility stuff with older configs/games
                if (xmlStrEqual(node->name, BAD_CAST "specials") && xmlStrEqual(childNode->name, BAD_CAST "set"))
                {
                    // specials.xml:/specials/set
                    SpecialDB::readSpecialSetNode(childNode, filename);
                }
            }
        }

        mIncludedFiles.erase(filename);
    }

}
