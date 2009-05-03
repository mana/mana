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

#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

#include <libxml/tree.h>

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
class Animation
{
    public:
        Animation();

        /**
         * Appends a new animation at the end of the sequence.
         */
        void addFrame(Image *image, unsigned int delay,
                      int offsetX, int offsetY);

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
         * Returns the duration of this animation.
         */
        int getDuration() const { return mDuration; }

        /**
         * Determines whether the given animation frame is a terminator.
         */
        static bool isTerminator(const Frame &phase);

    protected:
        std::vector<Frame> mFrames;
        int mDuration;
};

#endif
