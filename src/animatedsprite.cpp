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

#include "animatedsprite.h"

#include "animation.h"
#include "graphics.h"
#include "log.h"

#include "resources/resourcemanager.h"
#include "resources/spriteset.h"

AnimatedSprite::AnimatedSprite(const std::string& animationFile, int variant):
    mAction(NULL),
    mDirection(DIRECTION_DOWN),
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
    int variant_num = getProperty(node, "variants", 0);
    int variant_offset = getProperty(node, "variant_offset", 0);

    if (variant_num > 0 && variant < variant_num ) {
        variant_offset *= variant;
    } else {
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

            if (!spriteset) {
                logger->error("Couldn't load spriteset!");
            }

            mSpritesets[name] = spriteset;
        }
        // get action
        else if (xmlStrEqual(node->name, BAD_CAST "action"))
        {
            std::string actionName = getProperty(node, "name", "");
            std::string imageset = getProperty(node, "imageset", "");

            if (mSpritesets.find(imageset) == mSpritesets.end()) {
                logger->log("Warning: imageset \"%s\" not defined in %s",
                            imageset.c_str(),
                            animationFile.c_str());

                // skip loading animations
                continue;
            }


            SpriteAction actionType = makeSpriteAction(actionName);
            if (actionType == ACTION_INVALID)
            {
                logger->log("Warning: Unknown action \"%s\" defined in %s",
                    actionName.c_str(),
                    animationFile.c_str());
                continue;
            }
            Action *action = new Action();
            action->setSpriteset(mSpritesets[imageset]);
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

                std::string directionName = getProperty(animationNode, "direction", "");

                SpriteDirection directionType = makeSpriteDirection(directionName);
                if (directionType == DIRECTION_INVALID)
                {
                    logger->log("Warning: Unknown direction \"%s\" defined for action %s in %s",
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
                    int delay = getProperty(phaseNode, "delay", 0);

                    if (xmlStrEqual(phaseNode->name, BAD_CAST "frame"))
                    {
                        int index = getProperty(phaseNode, "index", -1);
                        int offsetX = getProperty(phaseNode, "offsetX", 0);
                        int offsetY = getProperty(phaseNode, "offsetY", 0);

                        offsetY -= mSpritesets[imageset]->getHeight() - 32;
                        offsetX -= mSpritesets[imageset]->getWidth() / 2 - 16;
                        animation->addPhase(index + variant_offset, delay,
                                offsetX, offsetY);
                    }
                    else if (xmlStrEqual(phaseNode->name, BAD_CAST "sequence"))
                    {
                        int start = getProperty(phaseNode, "start", 0);
                        int end = getProperty(phaseNode, "end", 0);
                        int offsetY = -mSpritesets[imageset]->getHeight() + 32;
                        int offsetX = -mSpritesets[imageset]->getWidth() / 2 + 16;
                        while (end >= start)
                        {
                            animation->addPhase(start + variant_offset,
                                    delay, offsetX, offsetY);
                            start++;
                        }
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

    // Play the stand animation by default
    play(ACTION_STAND);

    xmlFreeDoc(doc);
}

int
AnimatedSprite::getProperty(xmlNodePtr node, const char* name, int def)
{
    int &ret = def;

    xmlChar *prop = xmlGetProp(node, BAD_CAST name);
    if (prop) {
        ret = atoi((char*)prop);
        xmlFree(prop);
    }

    return ret;
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

    return def;
}

void
AnimatedSprite::substituteAction(SpriteAction complete,
                                 SpriteAction with)
{
    if (mActions.find(complete) == mActions.end())
    {
        ActionIterator i = mActions.find(with);
        if (i != mActions.end()) {
            mActions[complete] = i->second;
        }
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
AnimatedSprite::reset()
{
    // Reset all defined actions (because of aliases some will be resetted
    // multiple times, but this doesn't matter)
    for (ActionIterator i = mActions.begin(); i != mActions.end(); ++i)
    {
        if (i->second)
        {
            i->second->reset();
        }
    }
}

void
AnimatedSprite::play(SpriteAction action, int time)
{
    ActionIterator i = mActions.find(action);

    if (i == mActions.end())
    {
        logger->log("Warning: no action \"%u\" defined!", action);
        mAction = NULL;
        return;
    }

    if (mAction != i->second)
    {
        mAction = i->second;
        mLastTime = 0;
    }

    if (!mAction || !time)
        mSpeed = 1.0f;
    else {
        Animation* animation= mAction->getAnimation(mDirection);
        if (animation) {
            int animationLength = animation->getLength();
            mSpeed = (float) animationLength / time;
        }
    }
}

void
AnimatedSprite::update(int time)
{
    // Avoid freaking out at first frame or when tick_time overflows
    if (time < mLastTime || mLastTime == 0)
        mLastTime = time;

    // If not enough time has passed yet, do nothing
    if (time > mLastTime && mAction)
    {
        Animation *animation = mAction->getAnimation(mDirection);
        if (animation != NULL) {
            animation->update((unsigned int)((time - mLastTime) * mSpeed));}
        mLastTime = time;
    }
}

bool
AnimatedSprite::draw(Graphics* graphics, Sint32 posX, Sint32 posY) const
{
    if (!mAction)
        return false;

    Animation *animation = mAction->getAnimation(mDirection);
    if (animation == NULL) return false;

    int phase = animation->getCurrentPhase();
    if (phase < 0)
        return false;

    Spriteset *spriteset = mAction->getSpriteset();
    Image *image = spriteset->get(phase);
    Sint32 offsetX = animation->getOffsetX();
    Sint32 offsetY = animation->getOffsetY();
    return graphics->drawImage(image, posX + offsetX, posY + offsetY);
}

int
AnimatedSprite::getWidth() const
{
    return mAction ? mAction->getSpriteset()->getWidth() : 0;
}

int
AnimatedSprite::getHeight() const
{
    return mAction ? mAction->getSpriteset()->getHeight() : 0;
}

SpriteAction
AnimatedSprite::makeSpriteAction(const std::string& action)
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
AnimatedSprite::makeSpriteDirection(const std::string& direction)
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
