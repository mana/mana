/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "resources/imageset.h"

#include "utils/xml.h"

#include <map>
#include <string>
#include <vector>

class Animation;
class ImageSet;

struct SpriteReference
{
    std::string sprite;
    int variant = 0;
};

struct SpriteDisplay
{
    std::string image;
    std::vector<SpriteReference> sprites;
    std::vector<std::string> particles;
};

/*
 * Remember those are the main action.
 * Action subtypes, e.g.: "attack_bow" are to be passed by items.xml after
 * an ACTION_ATTACK call.
 * Which ability to be use to to be passed with the USE_ABILITY call.
 * Running, walking, ... is a sub-type of moving.
 * ...
 * Please don't add hard-coded subtypes here!
 */
namespace SpriteAction
{
    static const std::string DEFAULT = "stand";
    static const std::string STAND = "stand";
    static const std::string SIT = "sit";
    static const std::string SLEEP = "sleep";
    static const std::string DEAD = "dead";
    static const std::string MOVE = "walk";
    static const std::string ATTACK = "attack";
    static const std::string HURT = "hurt";
    static const std::string USE_ABILITY = "ability";
    static const std::string CAST_MAGIC = "magic";
    static const std::string USE_ITEM = "item";
    static const std::string INVALID;
}

enum SpriteDirection
{
    DIRECTION_DEFAULT = 0,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_INVALID
};

/**
 * An action consists of several animations, one for each direction.
 */
class Action
{
    public:
        Action();
        ~Action();

        void setAnimation(int direction, Animation *animation);
        Animation *getAnimation(int direction) const;

    protected:
        std::map<int, Animation *> mAnimations;
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
        static SpriteDef *load(const std::string &file, int variant);

        /**
         * Returns the specified action.
         */
        Action *getAction(const std::string &action) const;

        /**
         * Converts a string into a SpriteDirection enum.
         */
        static SpriteDirection
        makeSpriteDirection(const std::string &direction);

    private:
        SpriteDef() {}

        ~SpriteDef() override;

        /**
         * Loads a sprite element.
         */
        void loadSprite(XML::Node spriteNode, int variant,
                        const std::string &palettes = std::string());

        /**
         * Loads an imageset element.
         */
        void loadImageSet(XML::Node node, const std::string &palettes);

        /**
         * Loads an action element.
         */
        void loadAction(XML::Node node, int variant_offset);

        /**
         * Loads an animation element.
         */
        void loadAnimation(XML::Node animationNode,
                           Action *action, ImageSet *imageSet,
                           int variant_offset);

        /**
         * Include another sprite into this one.
         */
        void includeSprite(XML::Node includeNode);

        /**
         * Complete missing actions by copying existing ones.
         */
        void substituteActions();

        /**
         * When there are no animations defined for the action "complete", its
         * animations become a copy of those of the action "with".
         */
        void substituteAction(std::string complete, std::string with);

        std::map<std::string, ResourceRef<ImageSet>> mImageSets;
        std::map<std::string, Action *> mActions;
};
