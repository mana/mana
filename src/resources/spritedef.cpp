/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "resources/spritedef.h"

#include "resources/action.h"
#include "resources/animation.h"
#include "resources/dye.h"
#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"

#include "log.h"

#include "utils/xml.h"

#include <set>

Action *SpriteDef::getAction(SpriteAction action) const
{
    Actions::const_iterator i = mActions.find(action);

    if (i == mActions.end())
    {
        logger->log("Warning: no action \"%u\" defined!", action);
        return NULL;
    }

    return i->second;
}

SpriteDef *SpriteDef::load(const std::string &animationFile, int variant)
{
    std::string::size_type pos = animationFile.find('|');
    std::string palettes;
    if (pos != std::string::npos)
        palettes = animationFile.substr(pos + 1);

    XML::Document doc(animationFile.substr(0, pos));
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "sprite"))
    {
        logger->log("Error, failed to parse %s", animationFile.c_str());

        if (animationFile != "graphics/sprites/error.xml") {
            return load("graphics/sprites/error.xml", 0);
        } else {
            return NULL;
        }
    }

    SpriteDef *def = new SpriteDef;
    def->loadSprite(rootNode, variant, palettes);
    def->substituteActions();
    return def;
}

void SpriteDef::substituteActions()
{
    substituteAction(ACTION_STAND, ACTION_DEFAULT);
    substituteAction(ACTION_WALK, ACTION_STAND);
    substituteAction(ACTION_WALK, ACTION_RUN);
    substituteAction(ACTION_ATTACK, ACTION_STAND);
    substituteAction(ACTION_ATTACK_SWING, ACTION_ATTACK);
    substituteAction(ACTION_ATTACK_STAB, ACTION_ATTACK_SWING);
    substituteAction(ACTION_ATTACK_BOW, ACTION_ATTACK_STAB);
    substituteAction(ACTION_ATTACK_THROW, ACTION_ATTACK_SWING);
    substituteAction(ACTION_CAST_MAGIC, ACTION_ATTACK_SWING);
    substituteAction(ACTION_USE_ITEM, ACTION_CAST_MAGIC);
    substituteAction(ACTION_SIT, ACTION_STAND);
    substituteAction(ACTION_SLEEP, ACTION_SIT);
    substituteAction(ACTION_HURT, ACTION_STAND);
    substituteAction(ACTION_DEAD, ACTION_HURT);
}

void SpriteDef::loadSprite(xmlNodePtr spriteNode, int variant,
                           const std::string &palettes)
{
    // Get the variant
    const int variantCount = XML::getProperty(spriteNode, "variants", 0);
    int variant_offset = 0;

    if (variantCount > 0 && variant < variantCount)
    {
        variant_offset =
            variant * XML::getProperty(spriteNode, "variant_offset", 0);
    }

    for_each_xml_child_node(node, spriteNode)
    {
        if (xmlStrEqual(node->name, BAD_CAST "imageset"))
        {
            loadImageSet(node, palettes);
        }
        else if (xmlStrEqual(node->name, BAD_CAST "action"))
        {
            loadAction(node, variant_offset);
        }
        else if (xmlStrEqual(node->name, BAD_CAST "include"))
        {
            includeSprite(node);
        }
    }
}

void SpriteDef::loadImageSet(xmlNodePtr node, const std::string &palettes)
{
    const std::string name = XML::getProperty(node, "name", "");

    // We don't allow redefining image sets. This way, an included sprite
    // definition will use the already loaded image set with the same name.
    if (mImageSets.find(name) != mImageSets.end())
        return;

    const int width = XML::getProperty(node, "width", 0);
    const int height = XML::getProperty(node, "height", 0);
    std::string imageSrc = XML::getProperty(node, "src", "");
    Dye::instantiate(imageSrc, palettes);

    ResourceManager *resman = ResourceManager::getInstance();
    ImageSet *imageSet = resman->getImageSet(imageSrc, width, height);

    if (!imageSet)
    {
        logger->error("Couldn't load imageset!");
    }

    mImageSets[name] = imageSet;
}

void SpriteDef::loadAction(xmlNodePtr node, int variant_offset)
{
    const std::string actionName = XML::getProperty(node, "name", "");
    const std::string imageSetName = XML::getProperty(node, "imageset", "");

    ImageSetIterator si = mImageSets.find(imageSetName);
    if (si == mImageSets.end())
    {
        logger->log("Warning: imageset \"%s\" not defined in %s",
                imageSetName.c_str(), getIdPath().c_str());
        return;
    }
    ImageSet *imageSet = si->second;

    SpriteAction actionType = makeSpriteAction(actionName);
    if (actionType == ACTION_INVALID)
    {
        logger->log("Warning: Unknown action \"%s\" defined in %s",
                actionName.c_str(), getIdPath().c_str());
        return;
    }
    Action *action = new Action;
    mActions[actionType] = action;

    // When first action set it as default direction
    if (mActions.empty())
    {
        mActions[ACTION_DEFAULT] = action;
    }

    // Load animations
    for_each_xml_child_node(animationNode, node)
    {
        if (xmlStrEqual(animationNode->name, BAD_CAST "animation"))
        {
            loadAnimation(animationNode, action, imageSet, variant_offset);
        }
    }
}

void SpriteDef::loadAnimation(xmlNodePtr animationNode,
                              Action *action, ImageSet *imageSet,
                              int variant_offset)
{
    const std::string directionName =
        XML::getProperty(animationNode, "direction", "");
    const SpriteDirection directionType = makeSpriteDirection(directionName);

    if (directionType == DIRECTION_INVALID)
    {
        logger->log("Warning: Unknown direction \"%s\" used in %s",
                directionName.c_str(), getIdPath().c_str());
        return;
    }

    Animation *animation = new Animation;
    action->setAnimation(directionType, animation);

    // Get animation frames
    for_each_xml_child_node(frameNode, animationNode)
    {
        const int delay = XML::getProperty(frameNode, "delay", 0);
        int offsetX = XML::getProperty(frameNode, "offsetX", 0);
        int offsetY = XML::getProperty(frameNode, "offsetY", 0);
        offsetY -= imageSet->getHeight() - 32;
        offsetX -= imageSet->getWidth() / 2 - 16;

        if (xmlStrEqual(frameNode->name, BAD_CAST "frame"))
        {
            const int index = XML::getProperty(frameNode, "index", -1);

            if (index < 0)
            {
                logger->log("No valid value for 'index'");
                continue;
            }

            Image *img = imageSet->get(index + variant_offset);

            if (!img)
            {
                logger->log("No image at index %d", index + variant_offset);
                continue;
            }

            animation->addFrame(img, delay, offsetX, offsetY);
        }
        else if (xmlStrEqual(frameNode->name, BAD_CAST "sequence"))
        {
            int start = XML::getProperty(frameNode, "start", -1);
            const int end = XML::getProperty(frameNode, "end", -1);

            if (start < 0 || end < 0)
            {
                logger->log("No valid value for 'start' or 'end'");
                continue;
            }

            while (end >= start)
            {
                Image *img = imageSet->get(start + variant_offset);

                if (!img)
                {
                    logger->log("No image at index %d", start + variant_offset);
                    continue;
                }

                animation->addFrame(img, delay, offsetX, offsetY);
                start++;
            }
        }
        else if (xmlStrEqual(frameNode->name, BAD_CAST "end"))
        {
            animation->addTerminator();
        }
    } // for frameNode
}

void SpriteDef::includeSprite(xmlNodePtr includeNode)
{
    // TODO: Perform circular dependency check, since it's easy to crash the
    // client this way.
    const std::string filename = XML::getProperty(includeNode, "file", "");

    if (filename.empty())
        return;

    XML::Document doc("graphics/sprites/" + filename);
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "sprite"))
    {
        logger->log("Error, no sprite root node in %s", filename.c_str());
        return;
    }

    loadSprite(rootNode, 0);
}

void SpriteDef::substituteAction(SpriteAction complete, SpriteAction with)
{
    if (mActions.find(complete) == mActions.end())
    {
        Actions::iterator i = mActions.find(with);
        if (i != mActions.end()) {
            mActions[complete] = i->second;
        }
    }
}

SpriteDef::~SpriteDef()
{
    // Actions are shared, so ensure they are deleted only once.
    std::set< Action * > actions;
    for (Actions::const_iterator i = mActions.begin(),
         i_end = mActions.end(); i != i_end; ++i)
    {
        actions.insert(i->second);
    }

    for (std::set< Action * >::const_iterator i = actions.begin(),
         i_end = actions.end(); i != i_end; ++i)
    {
        delete *i;
    }

    for (ImageSetIterator i = mImageSets.begin();
            i != mImageSets.end(); ++i)
    {
        i->second->decRef();
    }
}

SpriteAction SpriteDef::makeSpriteAction(const std::string &action)
{
    if (action.empty() || action == "default") {
        return ACTION_DEFAULT;
    }
    if (action == "stand") {
        return ACTION_STAND;
    }
    else if (action == "walk") {
        return ACTION_WALK;
    }
    else if (action == "run") {
        return ACTION_RUN;
    }
    else if (action == "attack") {
        return ACTION_ATTACK;
    }
    else if (action == "attack_swing") {
        return ACTION_ATTACK_SWING;
    }
    else if (action == "attack_stab") {
        return ACTION_ATTACK_STAB;
    }
    else if (action == "attack_bow") {
        return ACTION_ATTACK_BOW;
    }
    else if (action == "attack_throw") {
        return ACTION_ATTACK_THROW;
    }
    else if (action == "special0") {
        return ACTION_SPECIAL_0;
    }
    else if (action == "special1") {
        return ACTION_SPECIAL_1;
    }
    else if (action == "special2") {
        return ACTION_SPECIAL_2;
    }
    else if (action == "special3") {
        return ACTION_SPECIAL_3;
    }
    else if (action == "special4") {
        return ACTION_SPECIAL_4;
    }
    else if (action == "special5") {
        return ACTION_SPECIAL_5;
    }
    else if (action == "special6") {
        return ACTION_SPECIAL_6;
    }
    else if (action == "special7") {
        return ACTION_SPECIAL_7;
    }
    else if (action == "special8") {
        return ACTION_SPECIAL_8;
    }
    else if (action == "special9") {
        return ACTION_SPECIAL_9;
    }
    else if (action == "cast_magic") {
        return ACTION_CAST_MAGIC;
    }
    else if (action == "use_item") {
        return ACTION_USE_ITEM;
    }
    else if (action == "sit") {
        return ACTION_SIT;
    }
    else if (action == "sleep") {
        return ACTION_SLEEP;
    }
    else if (action == "hurt") {
        return ACTION_HURT;
    }
    else if (action == "dead") {
        return ACTION_DEAD;
    }
    else {
        return ACTION_INVALID;
    }
}

SpriteDirection SpriteDef::makeSpriteDirection(const std::string &direction)
{
    if (direction.empty() || direction == "default") {
        return DIRECTION_DEFAULT;
    }
    else if (direction == "up") {
        return DIRECTION_UP;
    }
    else if (direction == "left") {
        return DIRECTION_LEFT;
    }
    else if (direction == "right") {
        return DIRECTION_RIGHT;
    }
    else if (direction == "down") {
        return DIRECTION_DOWN;
    }
    else {
        return DIRECTION_INVALID;
    }
}
