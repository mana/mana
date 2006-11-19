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

#include "../animation.h"
#include "../action.h"
#include "../graphics.h"
#include "../log.h"

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
    int variant_offset = XML::getProperty(node, "variant_offset", 0);

    if (variant_num > 0 && variant < variant_num ) {
        variant_offset *= variant;
    } else {
        variant_offset = 0;
    }

    for (node = node->xmlChildrenNode; node != NULL; node = node->next)
    {
        if (xmlStrEqual(node->name, BAD_CAST "imageset"))
        {
            int width = XML::getProperty(node, "width", 0);
            int height = XML::getProperty(node, "height", 0);
            std::string name = XML::getProperty(node, "name", "");
            std::string imageSrc = XML::getProperty(node, "src", "");

            Spriteset *spriteset =
                resman->getSpriteset(imageSrc, width, height);

            if (!spriteset) {
                logger->error("Couldn't load spriteset!");
            }

            mSpritesets[name] = spriteset;
        }
        // get action
        else if (xmlStrEqual(node->name, BAD_CAST "action"))
        {
            std::string actionName = XML::getProperty(node, "name", "");
            std::string imagesetName = XML::getProperty(node, "imageset", "");

            SpritesetIterator si = mSpritesets.find(imagesetName);
            if (si == mSpritesets.end()) {
                logger->log("Warning: imageset \"%s\" not defined in %s",
                            imagesetName.c_str(),
                            animationFile.c_str());

                // skip loading animations
                continue;
            }
            Spriteset *imageset = si->second;

            SpriteAction actionType = makeSpriteAction(actionName);
            if (actionType == ACTION_INVALID)
            {
                logger->log("Warning: Unknown action \"%s\" defined in %s",
                    actionName.c_str(),
                    animationFile.c_str());
                continue;
            }
            Action *action = new Action();
            mActions[actionType] = action;

            // When first action set it as default direction
            if (mActions.empty())
            {
                mActions[ACTION_DEFAULT] = action;
            }


            // get animations
            for (xmlNodePtr animationNode = node->xmlChildrenNode;
                 animationNode != NULL;
                 animationNode = animationNode->next)
            {
                // We're only interested in animations
                if (!xmlStrEqual(animationNode->name, BAD_CAST "animation"))
                    continue;

                std::string directionName =
                    XML::getProperty(animationNode, "direction", "");
                SpriteDirection directionType =
                    makeSpriteDirection(directionName);

                if (directionType == DIRECTION_INVALID)
                {
                    logger->log("Warning: Unknown direction \"%s\" defined "
                                "for action %s in %s",
                        directionName.c_str(),
                        actionName.c_str(),
                        animationFile.c_str());
                    continue;
                }

                Animation *animation = new Animation();
                action->setAnimation(directionType, animation);

                // Get animation phases
                for (xmlNodePtr phaseNode = animationNode->xmlChildrenNode;
                        phaseNode != NULL;
                        phaseNode = phaseNode->next)
                {
                    int delay = XML::getProperty(phaseNode, "delay", 0);

                    if (xmlStrEqual(phaseNode->name, BAD_CAST "frame"))
                    {
                        int index = XML::getProperty(phaseNode, "index", -1);
                        int offsetX = XML::getProperty(phaseNode, "offsetX", 0);
                        int offsetY = XML::getProperty(phaseNode, "offsetY", 0);

                        offsetY -= imageset->getHeight() - 32;
                        offsetX -= imageset->getWidth() / 2 - 16;
                        Image *img = imageset->get(index + variant_offset);
                        animation->addPhase(img, delay, offsetX, offsetY);
                    }
                    else if (xmlStrEqual(phaseNode->name, BAD_CAST "sequence"))
                    {
                        int start = XML::getProperty(phaseNode, "start", 0);
                        int end = XML::getProperty(phaseNode, "end", 0);
                        int offsetY = -imageset->getHeight() + 32;
                        int offsetX = -imageset->getWidth() / 2 + 16;
                        while (end >= start)
                        {
                            Image *img = imageset->get(start + variant_offset);
                            animation->addPhase(img, delay, offsetX, offsetY);
                            start++;
                        }
                    }
                    else if (xmlStrEqual(phaseNode->name, BAD_CAST "end"))
                    {
                        animation->addTerminator();
                    }
                } // for phaseNode
            } // for animationNode
        } // if "<imageset>" else if "<action>"
    } // for node

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

    xmlFreeDoc(doc);
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
    for (SpritesetIterator i = mSpritesets.begin(); i != mSpritesets.end(); ++i)
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
