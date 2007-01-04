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

#include "spritedef.h"

#include "../log.h"

#include "animation.h"
#include "action.h"
#include "resourcemanager.h"
#include "spriteset.h"
#include "image.h"

#include "../utils/xml.h"

SpriteDef::SpriteDef(const std::string &idPath,
                     const std::string &file, int variant):
    Resource(idPath),
    mAction(NULL),
    mDirection(DIRECTION_DOWN),
    mLastTime(0)
{
    load(file, variant);
}

Action*
SpriteDef::getAction(SpriteAction action) const
{
    Actions::const_iterator i = mActions.find(action);

    if (i == mActions.end())
    {
        logger->log("Warning: no action \"%u\" defined!", action);
        return NULL;
    }

    return i->second;
}

void
SpriteDef::load(const std::string &animationFile, int variant)
{
    int size;
    ResourceManager *resman = ResourceManager::getInstance();
    char *data = (char*) resman->loadFile(animationFile.c_str(), size);

    if (!data) {
        logger->error("Animation: Could not find " + animationFile + "!");
    }

    xmlDocPtr doc = xmlParseMemory(data, size);
    free(data);

    if (!doc) {
        logger->error(
                "Animation: Error while parsing animation definition file!");
    }

    xmlNodePtr node = xmlDocGetRootElement(doc);
    if (!node || !xmlStrEqual(node->name, BAD_CAST "sprite")) {
        logger->error(
                "Animation: this is not a valid animation definition file!");
    }

    // Get the variant
    int variant_num = XML::getProperty(node, "variants", 0);
    int variant_offset = 0;

    if (variant_num > 0 && variant < variant_num)
    {
        variant_offset = variant * XML::getProperty(node, "variant_offset", 0);
    }

    for (node = node->xmlChildrenNode; node != NULL; node = node->next)
    {
        if (xmlStrEqual(node->name, BAD_CAST "imageset"))
        {
            loadImageSet(node);
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

    xmlFreeDoc(doc);

    // Complete missing actions
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

void
SpriteDef::loadImageSet(xmlNodePtr node)
{
    int width = XML::getProperty(node, "width", 0);
    int height = XML::getProperty(node, "height", 0);
    std::string name = XML::getProperty(node, "name", "");
    std::string imageSrc = XML::getProperty(node, "src", "");

    ResourceManager *resman = ResourceManager::getInstance();
    Spriteset *spriteset = resman->getSpriteset(imageSrc, width, height);

    if (!spriteset)
    {
        logger->error("Couldn't load imageset!");
    }

    mSpritesets[name] = spriteset;
}

void
SpriteDef::loadAction(xmlNodePtr node, int variant_offset)
{
    const std::string actionName = XML::getProperty(node, "name", "");
    const std::string imagesetName = XML::getProperty(node, "imageset", "");

    SpritesetIterator si = mSpritesets.find(imagesetName);
    if (si == mSpritesets.end())
    {
        logger->log("Warning: imageset \"%s\" not defined in %s",
                imagesetName.c_str(), getIdPath().c_str());
        return;
    }
    Spriteset *imageset = si->second;

    SpriteAction actionType = makeSpriteAction(actionName);
    if (actionType == ACTION_INVALID)
    {
        logger->log("Warning: Unknown action \"%s\" defined in %s",
                actionName.c_str(), getIdPath().c_str());
        return;
    }
    Action *action = new Action();
    mActions[actionType] = action;

    // When first action set it as default direction
    if (mActions.empty())
    {
        mActions[ACTION_DEFAULT] = action;
    }

    // Load animations
    for (xmlNodePtr animationNode = node->xmlChildrenNode;
            animationNode != NULL;
            animationNode = animationNode->next)
    {
        if (xmlStrEqual(animationNode->name, BAD_CAST "animation"))
        {
            loadAnimation(animationNode, action, imageset, variant_offset);
        }
    }
}

void
SpriteDef::loadAnimation(xmlNodePtr animationNode,
                         Action *action, Spriteset *imageset,
                         int variant_offset)
{
    std::string directionName =
        XML::getProperty(animationNode, "direction", "");
    SpriteDirection directionType = makeSpriteDirection(directionName);

    if (directionType == DIRECTION_INVALID)
    {
        logger->log("Warning: Unknown direction \"%s\" used in %s",
                directionName.c_str(), getIdPath().c_str());
        return;
    }

    Animation *animation = new Animation();
    action->setAnimation(directionType, animation);

    // Get animation frames
    for (xmlNodePtr frameNode = animationNode->xmlChildrenNode;
            frameNode != NULL;
            frameNode = frameNode->next)
    {
        int delay = XML::getProperty(frameNode, "delay", 0);
        int offsetX = XML::getProperty(frameNode, "offsetX", 0);
        int offsetY = XML::getProperty(frameNode, "offsetY", 0);
        offsetY -= imageset->getHeight() - 32;
        offsetX -= imageset->getWidth() / 2 - 16;

        if (xmlStrEqual(frameNode->name, BAD_CAST "frame"))
        {
            int index = XML::getProperty(frameNode, "index", -1);

            if (index < 0)
            {
                logger->log("No valid value for 'index'");
                continue;
            }

            Image *img = imageset->get(index + variant_offset);

            if (!img)
            {
                logger->log("No image at index " + (index + variant_offset));
                continue;
            }

            animation->addFrame(img, delay, offsetX, offsetY);
        }
        else if (xmlStrEqual(frameNode->name, BAD_CAST "sequence"))
        {
            int start = XML::getProperty(frameNode, "start", -1);
            int end = XML::getProperty(frameNode, "end", -1);

            if (start < 0 || end < 0)
            {
                logger->log("No valid value for 'start' or 'end'");
                continue;
            }

            while (end >= start)
            {
                Image *img = imageset->get(start + variant_offset);

                if (!img)
                {
                    logger->log("No image at index " +
                            (start + variant_offset));
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

void
SpriteDef::includeSprite(xmlNodePtr includeNode)
{
    std::string filename = XML::getProperty(includeNode, "file", "");
    ResourceManager *resman = ResourceManager::getInstance();
    SpriteDef *sprite = resman->getSprite("graphics/sprites/" + filename);

    // TODO: Somehow implement actually including it
    sprite->decRef();
}

void
SpriteDef::substituteAction(SpriteAction complete, SpriteAction with)
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
    for (SpritesetIterator i = mSpritesets.begin();
            i != mSpritesets.end(); ++i)
    {
        i->second->decRef();
    }
}

SpriteAction
SpriteDef::makeSpriteAction(const std::string& action)
{
    if (action == "" || action == "default") {
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

SpriteDirection
SpriteDef::makeSpriteDirection(const std::string& direction)
{
    if (direction == "" || direction == "default") {
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
    };
}
