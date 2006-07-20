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

#include "animation.h"

#include <libxml/tree.h>

#include "log.h"

#include "graphic/spriteset.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#define READ_PROP(node, prop, name, target, cast) \
        prop = xmlGetProp(node, BAD_CAST name); \
        if (prop) { \
            target = cast((const char*)prop); \
            xmlFree(prop); \
        }

Animation::Animation():
    mTime(0)
{
    iCurrentPhase = mAnimationPhases.begin();
}

void
Animation::update(unsigned int time)
{
    mTime += time;
    if (!mAnimationPhases.empty())
    {
        while ((mTime > (*iCurrentPhase).delay) &&
                (*iCurrentPhase).delay > 0)
        {
            mTime -= (*iCurrentPhase).delay;
            iCurrentPhase++;
            if (iCurrentPhase == mAnimationPhases.end())
            {
                iCurrentPhase = mAnimationPhases.begin();
            }
        }
    }
}

int
Animation::getCurrentPhase()
{
    if (mAnimationPhases.empty())
    {
        return -1;
    }
    else
    {
        return (*iCurrentPhase).image;
    }
}

void
Animation::addPhase (int image, unsigned int delay, int offsetX, int offsetY)
{
    //add new phase to animation list
    AnimationPhase newPhase;
    newPhase.image = image;
    newPhase.delay = delay;
    newPhase.offsetX = offsetX;
    newPhase.offsetY = offsetY;
    mAnimationPhases.push_back(newPhase);
    //reset animation circle
    iCurrentPhase = mAnimationPhases.begin();
}

int
Animation::getLength()
{
    std::list<AnimationPhase>::iterator i;
    int length = 0;
    if (!mAnimationPhases.empty())
    {
        for (i = mAnimationPhases.begin(); i != mAnimationPhases.end(); i++)
        {
            length += (*i).delay;
        }
    }
    printf("length: %i\n", length);
    return length;
}

Action::Action()
    :mImageset("")
{
    //NOOP
}

Action::~Action()
{
    for (AnimationIterator i = mAnimations.begin(); i != mAnimations.end(); i++)
    {
        delete i->second;
    }
    mAnimations.clear();
}

Animation *
Action::getAnimation(std::string direction)
{
    Animation *animation = NULL;
    AnimationIterator i = mAnimations.find(direction);
    if (i == mAnimations.end())
    {
        //when the direction isn't defined just use another one
        animation = mAnimations["default"];
    }
    else
    {
        animation = mAnimations[direction];
    }

    return animation;
}

void
Action::setAnimation(std::string direction, Animation *animation)
{
    //set first direction as default direction
    if (mAnimations.empty())
    {
        mAnimations["default"] = animation;
    }

    mAnimations[direction] = animation;
}


AnimatedSprite::AnimatedSprite(std::string animationFile, int variant):
    mAction("stand"), mDirection("down"), mLastTime(0), mSpeed(1.0f)
{
    int variant_num = 0;
    int variant_offset = 0;
    int size;
    ResourceManager *resman = ResourceManager::getInstance();
    char *data = (char*)resman->loadFile(
            animationFile.c_str(), size);

    if (!data) {
        logger->error("Animation: Could not find " + animationFile + " !");
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

    //get the variant
    xmlChar *prop = NULL;
    READ_PROP(node, prop, "variants", variant_num, atoi);
    READ_PROP(node, prop, "variant_offset", variant_offset, atoi);

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
            int width = 0, height = 0;
            std::string name = "", imageSrc = "";
            xmlChar *prop = NULL;
            READ_PROP(node, prop, "name", name, );
            READ_PROP(node, prop, "src", imageSrc, );
            READ_PROP(node, prop, "width", width, atoi);
            READ_PROP(node, prop, "height", height, atoi);

            Spriteset *spriteset = resman->createSpriteset(
                    imageSrc, width, height);
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
            std::string name = "", imageset = "";
            xmlChar *prop = NULL;
            READ_PROP(node, prop, "name", name, );
            READ_PROP(node, prop, "imageset", imageset, );

            Action *action = new Action();
            mActions[name] = action;
            action->setImageset(imageset);

            //get animations
            for (   xmlNodePtr animationNode = node->xmlChildrenNode;
                    animationNode != NULL;
                    animationNode = animationNode->next)
            {
                if (xmlStrEqual(animationNode->name, BAD_CAST "animation"))
                {
                    std::string direction = "";


                    Animation *animation = new Animation();
                    READ_PROP(animationNode, prop, "direction", direction, );

                    //get animation phases
                    for (   xmlNodePtr phaseNode = animationNode->xmlChildrenNode;
                            phaseNode != NULL;
                            phaseNode = phaseNode->next)
                    {
                        int index = -1;
                        int start = 0;
                        int end = 0;
                        int delay = 0;
                        int offsetX = 0;
                        int offsetY = 0;
                        if (xmlStrEqual(phaseNode->name, BAD_CAST "frame"))
                        {
                            READ_PROP(phaseNode, prop, "index", index, atoi);
                            READ_PROP(phaseNode, prop, "delay", delay, atoi);
                            READ_PROP(phaseNode, prop, "offsetX", offsetX, atoi);
                            READ_PROP(phaseNode, prop, "offsetY", offsetY, atoi);
                            offsetY = offsetY - mSpritesets[imageset]->getHeight() + 32;
                            offsetX = offsetX - mSpritesets[imageset]->getWidth() / 2 + 16;
                            animation->addPhase(index + variant_offset, delay, offsetX, offsetY);
                        }
                        if (xmlStrEqual(phaseNode->name, BAD_CAST "sequence"))
                        {
                            READ_PROP(phaseNode, prop, "start", start, atoi);
                            READ_PROP(phaseNode, prop, "end", end, atoi);
                            READ_PROP(phaseNode, prop, "delay", delay, atoi);
                            offsetY = 0 - mSpritesets[imageset]->getHeight() + 32;
                            offsetX = 0 - mSpritesets[imageset]->getWidth() / 2 + 16;
                            while (end >= start)
                            {
                                animation->addPhase(start + variant_offset, delay, offsetX, offsetY);
                                start++;
                            }
                        }
                    } // for phaseNode
                    action->setAnimation(direction, animation);
                } // if "<animation>"
            } // for animationNode
        } // if "<imageset>" else if "<action>"
    } // for node

    //complete missing actions
    substituteAction("stand", "");
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

    xmlFreeDoc(doc);
}

void
AnimatedSprite::substituteAction(std::string complete, std::string with)
{
    if (mActions.find(complete) == mActions.end())
    {
        mActions[complete] = mActions[with];
    }
}

AnimatedSprite::~AnimatedSprite()
{
    for (SpritesetIterator i = mSpritesets.begin(); i != mSpritesets.end(); i++)
    {
        delete i->second;
    }
    mSpritesets.clear();
}

void
AnimatedSprite::play(std::string action)
{
    if (mAction != action)
    {
        mAction = action;
    }
    mLastTime = 0;
    mSpeed = 1.0f;
}

void
AnimatedSprite::play(std::string action, int time)
{
    if (mAction != action)
    {
        mAction = action;
    }
    mLastTime = 0;
    int animationLength = 0;
    Action *nextAction = mActions[mAction];
    Animation *animation = nextAction->getAnimation(mDirection);
    animationLength = animation->getLength();
    if (animationLength)
    {
        mSpeed = time / animationLength;
    }
    else
    {
        mSpeed = 1.0f;
    }
}

void
AnimatedSprite::play(std::string action, std::string direction)
{
    play(action);
    mDirection = direction;
}

void
AnimatedSprite::update(int time)
{
    //avoid freaking out at first frame or when tick_time overflows
    if (time < mLastTime || mLastTime == 0) mLastTime = time;

    Action *action = mActions[mAction];
    Animation *animation = action->getAnimation(mDirection);
    animation->update((unsigned int)((time - mLastTime) / mSpeed));
    mLastTime = time;
}

bool
AnimatedSprite::draw(Graphics * graphics, Sint32 posX, Sint32 posY)
{
    Sint32 offsetX, offsetY;
    Action *action = mActions[mAction];
    Spriteset *spriteset = mSpritesets[action->getImageset()];
    Animation *animation = action->getAnimation(mDirection);

    if (animation->getCurrentPhase() >= 0)
    {
        Image *image = spriteset->get(animation->getCurrentPhase());
        offsetX = animation->getOffsetX();
        offsetY = animation->getOffsetY();
        return graphics->drawImage(image, posX + offsetX, posY + offsetY);
    }
    else
    {
        return false;
    }
}

Image *
AnimatedSprite::getCurrentFrame()
{
    Action *action = mActions[mAction];
    Spriteset *spriteset = mSpritesets[action->getImageset()];
    Animation *animation = action->getAnimation(mDirection);
    if (animation->getCurrentPhase() >= 0)
    {
        return spriteset->get(animation->getCurrentPhase());
    }
    else
    {
        return NULL;
    }
}

int
AnimatedSprite::getWidth()
{
    Action *action = mActions[mAction];
    Spriteset *spriteset = mSpritesets[action->getImageset()];
    return spriteset->getWidth();
}

int
AnimatedSprite::getHeight()
{
    Action *action = mActions[mAction];
    Spriteset *spriteset = mSpritesets[action->getImageset()];
    return spriteset->getHeight();
}
