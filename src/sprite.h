/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#include "resources/spritedef.h"

class Graphics;
class Image;
struct Frame;

// Default frame display delay in milliseconds
const int DEFAULT_FRAME_DELAY = 75;

/**
 * Animates a sprite by adding playback state.
 */
class Sprite
{
    public:
        /**
         * Constructor.
         * @param sprite the sprite to animate
         */
        Sprite(SpriteDef *sprite);

        /**
         * An helper function, which will request the sprite to animate
         * from the resource manager.
         *
         * @param filename the file of the sprite to animate
         * @param variant  the sprite variant
         */
        static Sprite *load(const std::string &filename, int variant = 0);

        ~Sprite();

        /**
         * Resets the sprite.
         *
         * @returns true if the sprite changed, false otherwise
         */
        bool reset();

        /**
         * Plays an action using the current direction.
         *
         * @returns true if the sprite changed, false otherwise
         */
        bool play(const std::string &action);

        /**
         * Inform the animation of the passed time so that it can output the
         * correct animation frame.
         *
         * @returns true if the sprite changed, false otherwise
         */
        bool update(int time);

        /**
         * Draw the current animation frame at the coordinates given in screen
         * pixels.
         */
        bool draw(Graphics *graphics, int posX, int posY) const;

        /**
         * Gets the width in pixels of the image
         */
        int getWidth() const;

        /**
         * Gets the height in pixels of the image
         */
        int getHeight() const;

        /**
         * Gets the horizontal offset that the sprite will be drawn at
         */
        int getOffsetX() const;

        /**
         * Gets the vertical offset that the sprite will be drawn at
         */
        int getOffsetY() const;

        /**
         * Returns a reference to the current image being drawn.
         */
        const Image *getImage() const;

        /**
         * Sets the direction.
         *
         * @returns true if the sprite changed, false otherwise
         */
        bool setDirection(SpriteDirection direction);

        /**
         * Sets the alpha value of the sprite.
         */
        void setAlpha(float alpha) { mAlpha = alpha; }

        /**
         * Returns the current alpha opacity of the sprite.
         */
        float getAlpha() const { return mAlpha; }

        /**
         * Returns the duration of the current sprite animation in milliseconds.
         */
        int getDuration() const;

    private:
        bool updateCurrentAnimation(int dt);

        float mAlpha = 1.0f;                /**< The alpha opacity used to draw */

        SpriteDirection mDirection = DIRECTION_DOWN;    /**< The sprite direction. */

        int mFrameIndex = 0;                /**< The index of the current frame. */
        int mFrameTime = 0;                 /**< The time since start of frame. */

        ResourceRef<SpriteDef> mSprite;     /**< The sprite definition. */
        Action *mAction = nullptr;          /**< The currently active action. */
        Animation *mAnimation = nullptr;    /**< The currently active animation. */
        Frame *mFrame = nullptr;            /**< The currently active frame. */
};
