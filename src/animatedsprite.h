/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef ANIMATEDSPRITE_H
#define ANIMATEDSPRITE_H

#include "sprite.h"

#include <map>
#include <string>

class Animation;
struct Frame;

/**
 * Animates a sprite by adding playback state.
 */
class AnimatedSprite : public Sprite
{
    public:
        /**
         * Constructor.
         * @param sprite the sprite to animate
         */
        AnimatedSprite(SpriteDef *sprite);

        /**
         * An helper function, which will request the sprite to animate
         * from the resource manager.
         *
         * @param filename the file of the sprite to animate
         * @param variant  the sprite variant
         */
        static AnimatedSprite *load(const std::string &filename,
                                    int variant = 0);

        virtual ~AnimatedSprite();

        bool reset();

        bool play(SpriteAction action);

        bool update(int time);

        bool draw(Graphics* graphics, int posX, int posY) const;

        int getWidth() const;

        int getHeight() const;

        const Image* getImage() const;

        bool setDirection(SpriteDirection direction);

        int getNumberOfLayers()
        { return 1; }

        size_t getCurrentFrame() const;

        size_t getFrameCount() const;

    private:
        bool updateCurrentAnimation(unsigned int dt);

        SpriteDirection mDirection;    /**< The sprite direction. */
        int mLastTime;                 /**< The last time update was called. */

        int mFrameIndex;               /**< The index of the current frame. */
        int mFrameTime;                /**< The time since start of frame. */

        SpriteDef *mSprite;            /**< The sprite definition. */
        Action *mAction;               /**< The currently active action. */
        Animation *mAnimation;         /**< The currently active animation. */
        Frame *mFrame;                 /**< The currently active frame. */
};

#endif
