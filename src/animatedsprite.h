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

#ifndef ANIMATEDSPRITE_H
#define ANIMATEDSPRITE_H

#include "sprite.h"

#include <string>

class Animation;
struct Frame;

/**
 * Animates a sprite by adding playback state.
 */
class AnimatedSprite final : public Sprite
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

        ~AnimatedSprite() override;

        bool reset() override;

        bool play(const std::string &action) override;

        bool update(int time) override;

        bool draw(Graphics *graphics, int posX, int posY) const override;

        int getWidth() const override;

        int getHeight() const override;

        int getOffsetX() const override;

        int getOffsetY() const override;

        const Image *getImage() const override;

        bool setDirection(SpriteDirection direction) override;

        virtual bool drawnWhenBehind() const
        { return true; }

        int getDuration() const override;

    private:
        bool updateCurrentAnimation(unsigned int dt);

        SpriteDirection mDirection = DIRECTION_DOWN;    /**< The sprite direction. */
        int mLastTime = 0;                  /**< The last time update was called. */

        int mFrameIndex = 0;                /**< The index of the current frame. */
        int mFrameTime = 0;                 /**< The time since start of frame. */

        ResourceRef<SpriteDef> mSprite;     /**< The sprite definition. */
        Action *mAction = nullptr;          /**< The currently active action. */
        Animation *mAnimation = nullptr;    /**< The currently active animation. */
        Frame *mFrame = nullptr;            /**< The currently active frame. */
};

#endif
