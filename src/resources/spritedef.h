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

#ifndef SPRITEDEF_H
#define SPRITEDEF_H

#include <map>
#include <string>

#include <libxml/tree.h>

#include "resource.h"

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
         * Loads a sprite definition file.
         */
        static SpriteDef *load(std::string const &file, int variant);

        /**
         * Returns the specified action.
         */
        Action *getAction(SpriteAction action) const;

    private:
        /**
         * Constructor.
         */
        SpriteDef() {}

        /**
         * Destructor.
         */
        ~SpriteDef();

        /**
         * Loads a sprite element.
         */
        void loadSprite(xmlNodePtr spriteNode, int variant,
                        const std::string &palettes = "");

        /**
         * Loads an imageset element.
         */
        void loadImageSet(xmlNodePtr node, std::string const &palettes);

        /**
         * Loads an action element.
         */
        void loadAction(xmlNodePtr node, int variant_offset);

        /**
         * Loads an animation element.
         */
        void loadAnimation(xmlNodePtr animationNode,
                           Action *action, ImageSet *imageSet,
                           int variant_offset);

        /**
         * Include another sprite into this one.
         */
        void includeSprite(xmlNodePtr includeNode);

        /**
         * Complete missing actions by copying existing ones.
         */
        void substituteActions();

        /**
         * When there are no animations defined for the action "complete", its
         * animations become a copy of those of the action "with".
         */
        void substituteAction(SpriteAction complete, SpriteAction with);

        /**
         * Converts a string into a SpriteAction enum.
         */
        static SpriteAction makeSpriteAction(const std::string &action);

        /**
         * Converts a string into a SpriteDirection enum.
         */
        static SpriteDirection makeSpriteDirection(const std::string &direction);

        typedef std::map<std::string, ImageSet*> ImageSets;
        typedef ImageSets::iterator ImageSetIterator;

        typedef std::map<SpriteAction, Action*> Actions;

        ImageSets mImageSets;
        Actions mActions;
};

#endif
