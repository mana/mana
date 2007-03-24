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

#ifndef _TMW_SPRITEDEF_H
#define _TMW_SPRITEDEF_H

#include "resource.h"

#include <map>
#include <string>

#include <libxml/tree.h>

class Action;
class ImageSet;

enum SpriteAction
{
    ACTION_DEFAULT = 0,
    ACTION_STAND,
    ACTION_WALK,
    ACTION_RUN,
    ACTION_ATTACK,
    ACTION_ATTACK_SWING,
    ACTION_ATTACK_STAB,
    ACTION_ATTACK_BOW,
    ACTION_ATTACK_THROW,
    ACTION_CAST_MAGIC,
    ACTION_USE_ITEM,
    ACTION_SIT,
    ACTION_SLEEP,
    ACTION_HURT,
    ACTION_DEAD,
    ACTION_INVALID
};

enum SpriteDirection
{
    DIRECTION_DEFAULT = 0,
    DIRECTION_DOWN,
    DIRECTION_UP,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_INVALID
};

/**
 * Defines a class to load an animation.
 */
class SpriteDef : public Resource
{
    public:
        /**
         * Constructor.
         */
        SpriteDef(const std::string &idPath,
                  const std::string &file, int variant);

        /**
         * Destructor.
         */
        ~SpriteDef();

        /**
         * Returns the specified action.
         */
        Action*
        getAction(SpriteAction action) const;

    private:
        /**
         * Loads a sprite definition file.
         */
        void
        load(const std::string &file, int variant);

        /**
         * Loads an imageset element.
         */
        void
        loadImageSet(xmlNodePtr node);

        /**
         * Loads an action element.
         */
        void
        loadAction(xmlNodePtr node, int variant_offset);

        /**
         * Loads an animation element.
         */
        void
        loadAnimation(xmlNodePtr animationNode,
                      Action *action, ImageSet *imageSet,
                      int variant_offset);

        /**
         * Include another sprite into this one.
         */
        void
        includeSprite(xmlNodePtr includeNode);

        /**
         * When there are no animations defined for the action "complete", its
         * animations become a copy of those of the action "with".
         */
        void
        substituteAction(SpriteAction complete, SpriteAction with);

        /**
         * Converts a string into a SpriteAction enum.
         */
        static SpriteAction
        makeSpriteAction(const std::string &action);

        /**
         * Converts a string into a SpriteDirection enum.
         */
        static SpriteDirection
        makeSpriteDirection(const std::string &direction);


        typedef std::map<std::string, ImageSet*> ImageSets;
        typedef ImageSets::iterator ImageSetIterator;

        typedef std::map<SpriteAction, Action*> Actions;

        ImageSets mImageSets;
        Actions mActions;
        Action *mAction;
        SpriteDirection mDirection;
        int mLastTime;
};

#endif
