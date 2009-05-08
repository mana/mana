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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <string>

#include <guichan/widget.hpp>

class ImageRect;

/**
 * A progress bar.
 *
 * \ingroup GUI
 */
class ProgressBar : public gcn::Widget
{
    public:
        /**
         * Constructor, initializes the progress with the given value.
         */
        ProgressBar(float progress = 0.0f,
                    int width = 40, int height = 7,
                    const gcn::Color &color = gcn::Color(150, 150, 150));

        ~ProgressBar();

        /**
         * Performs progress bar logic (fading colors)
         */
        void logic();

        /**
         * Draws the progress bar.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Sets the current progress.
         */
        void setProgress(float progress);

        /**
         * Returns the current progress.
         */
        float getProgress() const { return mProgress; }

        /**
         * Change the color of the progress bar.
         */
        void setColor(const gcn::Color &color);

        /**
         * Change the color of the progress bar.
         *
         * This is an overload provided for convenience.
         */
        inline void setColor(int r, int g, int b)
        { setColor(gcn::Color(r, g, b)); }

        /**
         * Returns the color of the progress bar.
         */
        const gcn::Color &getColor() const { return mColor; }

        /**
         * Sets the text shown on the progress bar.
         */
        void setText(const std::string &text)
        { mText = text; }

        /**
         * Returns the text shown on the progress bar.
         */
        const std::string &text() const
        { return mText; }

        /**
         * Set whether the progress is moved smoothly.
         */
        void setSmoothProgress(bool smoothProgress)
        { mSmoothProgress = smoothProgress; }

        /**
         * Set whether the color changing is made smoothly.
         */
        void setSmoothColorChange(bool smoothColorChange)
        { mSmoothColorChange = smoothColorChange; }

    private:
        float mProgress, mProgressToGo;
        bool mSmoothProgress;

        gcn::Color mColor;
        gcn::Color mColorToGo;
        bool mSmoothColorChange;

        std::string mText;

        static ImageRect mBorder;
        static int mInstances;
        static float mAlpha;

        static const gcn::Color TEXT_COLOR;
};

#endif
