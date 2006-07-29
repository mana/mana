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
 *  $Id: animation.cpp 2430 2006-07-24 00:13:24Z b_lindeijer $
 */

#include "animatedsprite.h"

#include "animation.h"
#include "graphics.h"
#include "log.h"

#include "resources/resourcemanager.h"
#include "resources/spriteset.h"

AnimatedSprite::AnimatedSprite(const std::string& animationFile, int variant):
    mAction(NULL),
    mDirection("down"),
    mLastTime(0),
    mSpeed(1.0f)
{
    int size;
    ResourceManager *resman = ResourceManager::getInstance();
    char *data = (char*)resman->loadFile(animationFile.c_str(), size);

    if (!data) {
        logger->error("Animation: Could not find " + animationFile + "!");
    }

    xmlDocPtr doc = xmlParseMemory(data, size);
    free(data);

    if (!doc)
    {
        logger->error("Animation: Error while parsing animation definition file!");
        return;
    }

    xmlNodePtr node = xmlDocGetRootElement(doc);
    if (!node || !xmlStrEqual(node->name, BAD_CAST "sprite"))
    {
        logger->error("Animation: this is not a valid animation definition file!");
        return;
    }

    // Get the variant
    int variant_num = getProperty(node, "variants", 0);
    int variant_offset = getProperty(node, "variant_offset", 0);

    if (variant_num > 0 && variant < variant_num )
    {
        variant_offset *= variant;
    }
    else
    {
        variant_offset = 0;
    }

    for (node = node->xmlChildrenNode; node != NULL; node = node->next)
    {
        if (xmlStrEqual(node->name, BAD_CAST "imageset"))
        {
            int width = getProperty(node, "width", 0);
            int height = getProperty(node, "height", 0);
            std::string name = getProperty(node, "name", "");
            std::string imageSrc = getProperty(node, "src", "");

            Spriteset *spriteset =
                resman->getSpriteset(imageSrc, width, height);

            if (!spriteset)
            {
                logger->error("Couldn't load spriteset!");
            }
            else
            {
                mSpritesets[name] = spriteset;
            }
        }
        // get action
        else if (xmlStrEqual(node->name, BAD_CAST "action"))
        {
            std::string name = getProperty(node, "name", "");
            std::string imageset = getProperty(node, "imageset", "");

            if (name.length() == 0)
            {
                logger->log("Warning: unnamed action in %s",
                            animationFile.c_str());
            }

            Action *action = new Action();

            if (mSpritesets.find(imageset) != mSpritesets.end())
            {
                action->setSpriteset(mSpritesets[imageset]);
                mActions[name] = action;
            }
            else
            {
                logger->log("Warning: imageset \"%s\" not defined in %s",
                            imageset.c_str(),
                            animationFile.c_str());

                // Discard action and skip loading animations
                delete action;
                continue;
            }

            // get animations
            for (xmlNodePtr animationNode = node->xmlChildrenNode;
                 animationNode != NULL;
                 animationNode = animationNode->next)
            {
                if (xmlStrEqual(animationNode->name, BAD_CAST "animation"))
                {
                    std::string direction =
                        getProperty(animationNode, "direction", "");

                    Animation *animation = new Animation();

                    // Get animation phases
                    for (xmlNodePtr phaseNode = animationNode->xmlChildrenNode;
                         phaseNode != NULL;
                         phaseNode = phaseNode->next)
                    {
                        int delay = getProperty(phaseNode, "delay", 0);

                        if (xmlStrEqual(phaseNode->name, BAD_CAST "frame"))
                        {
                            int index = getProperty(phaseNode, "index", -1);
                            int offsetX = getProperty(phaseNode, "offsetX", 0);
                            int offsetY = getProperty(phaseNode, "offsetY", 0);

                            offsetY = offsetY - mSpritesets[imageset]->getHeight() + 32;
                            offsetX = offsetX - mSpritesets[imageset]->getWidth() / 2 + 16;
                            animation->addPhase(index + variant_offset, delay,
                                                offsetX, offsetY);
                        }
                        else if (xmlStrEqual(phaseNode->name, BAD_CAST "sequence"))
                        {
                            int start = getProperty(phaseNode, "start", 0);
                            int end = getProperty(phaseNode, "end", 0);
                            int offsetY = 0 - mSpritesets[imageset]->getHeight() + 32;
                            int offsetX = 0 - mSpritesets[imageset]->getWidth() / 2 + 16;
                            while (end >= start)
                            {
                                animation->addPhase(start + variant_offset,
                                                    delay, offsetX, offsetY);
                                start++;
                            }
                        }
                    } // for phaseNode
                    action->setAnimation(direction, animation);
                } // if "<animation>"
            } // for animationNode
        } // if "<imageset>" else if "<action>"
    } // for node

    // Complete missing actions
    substituteAction("walk", "stand");
    substituteAction("walk", "run");
    substituteAction("attack", "stand");
    substituteAction("attack_swing", "attack");
    substituteAction("attack_stab", "attack_swing");
    substituteAction("attack_bow", "attack_stab");
    substituteAction("attack_throw", "attack_swing");
    substituteAction("cast_magic", "attack_swing");
    substituteAction("use_item", "cast_magic");
    substituteAction("sit", "stand");
    substituteAction("sleeping", "sit");
    substituteAction("hurt", "stand");
    substituteAction("dead", "hurt");

    // Play the stand animation by default
    play("stand");

    xmlFreeDoc(doc);
}

int
AnimatedSprite::getProperty(xmlNodePtr node, const char* name, int def)
{
    xmlChar *prop = xmlGetProp(node, BAD_CAST name);
    if (prop) {
        int val = atoi((char*)prop);
        xmlFree(prop);
        return val;
    }
    else {
        return def;
    }
}

std::string
AnimatedSprite::getProperty(xmlNodePtr node, const char* name,
                            const std::string& def)
{
    xmlChar *prop = xmlGetProp(node, BAD_CAST name);
    if (prop) {
        std::string val = (char*)prop;
        xmlFree(prop);
        return val;
    }
    else {
        return def;
    }
}

void
AnimatedSprite::substituteAction(const std::string& complete,
                                 const std::string& with)
{
    if (mActions.find(complete) == mActions.end())
    {
        mActions[complete] = mActions[with];
    }
}

AnimatedSprite::~AnimatedSprite()
{
    for (SpritesetIterator i = mSpritesets.begin(); i != mSpritesets.end(); ++i)
    {
        i->second->decRef();
    }
    mSpritesets.clear();
}

void
AnimatedSprite::play(const std::string& action)
{
    Actions::iterator iAction;
    iAction = mActions.find(action);

    if (iAction == mActions.end())
    {
        logger->log("Warning: no action \"%s\" defined!", action.c_str());
        mAction = NULL;
        return;
    }

    if (mAction != iAction->second)
    {
        mAction = iAction->second;
        mLastTime = 0;
    }

    mSpeed = 1.0f;
}

void
AnimatedSprite::play(const std::string& action, int time)
{
    play(action);

    if (mAction != NULL)
    {
        Animation *animation = mAction->getAnimation(mDirection);
        int animationLength = animation->getLength();
        mSpeed = (float) animationLength / time;
    }
}

void
AnimatedSprite::update(int time)
{
    // Avoid freaking out at first frame or when tick_time overflows
    if (time < mLastTime || mLastTime == 0) mLastTime = time;

    // If not enough time have passed yet, do nothing
    if (time > mLastTime)
    {
        if (mAction != NULL)
        {
            Animation *animation = mAction->getAnimation(mDirection);
            animation->update((unsigned int)((time - mLastTime) * mSpeed));
            mLastTime = time;
        }
    }
}

bool
AnimatedSprite::draw(Graphics* graphics, Sint32 posX, Sint32 posY) const
{
    if (mAction != NULL)
    {
        Animation *animation = mAction->getAnimation(mDirection);

        if (animation->getCurrentPhase() >= 0)
        {
            Spriteset *spriteset = mAction->getSpriteset();
            Image *image = spriteset->get(animation->getCurrentPhase());
            Sint32 offsetX = animation->getOffsetX();
            Sint32 offsetY = animation->getOffsetY();
            return graphics->drawImage(image, posX + offsetX, posY + offsetY);
        }
    }

    return false;
}

int
AnimatedSprite::getWidth() const
{
    if (mAction != NULL)
    {
        Spriteset *spriteset = mAction->getSpriteset();
        return spriteset->getWidth();
    }

    return 0;
}

int
AnimatedSprite::getHeight() const
{
    if (mAction != NULL)
    {
        Spriteset *spriteset = mAction->getSpriteset();
        return spriteset->getHeight();
    }

    return 0;
}
