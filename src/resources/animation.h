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

#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

class Image;

/**
 * A single frame in an animation, with a delay and an offset.
 */
struct Frame
{
    Image *image;
    int delay;
    int offsetX;
    int offsetY;
};

/**
 * An animation consists of several frames, each with their own delay and
 * offset.
 */
class Animation final
{
    public:
        Animation() = default;

        /**
         * Appends a new animation at the end of the sequence.
         */
        void addFrame(Image *image, int delay, int offsetX, int offsetY);

        /**
         * Appends an animation terminator that states that the animation
         * should not loop.
         */
        void addTerminator();

        /**
         * Returns the frame at the specified index.
         */
        Frame *getFrame(int index) { return &(mFrames[index]); }

        /**
         * Returns the length of this animation in frames.
         */
        int getLength() const { return mFrames.size(); }

        /**
         * Returns the duration of this animation in milliseconds.
         */
        int getDuration() const { return mDuration; }

        /**
         * Determines whether the given animation frame is a terminator.
         */
        static bool isTerminator(const Frame &phase);

    protected:
        std::vector<Frame> mFrames;
        int mDuration = 0;
};

#endif
