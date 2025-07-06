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

#include "gui/widgets/progressbar.h"

#include "graphics.h"

#include "gui/gui.h"

#include "resources/theme.h"

#include <guichan/font.hpp>

ProgressBar::ProgressBar(float progress,
                         int width, int height,
                         int color):
    mProgressPalette(color)
{
    // The progress value is directly set at load time:
    if (progress > 1.0f || progress < 0.0f)
        progress = 1.0f;

    mProgress = mProgressToGo = progress;

    mColor = mColorToGo = Theme::getProgressColor(color >= 0 ? color : 0,
                                                  mProgress);

    setSize(width, height);
}

void ProgressBar::logic()
{
    if (mSmoothColorChange && mColorToGo != mColor)
    {
        // Smoothly changing the color for a nicer effect.
        if (mColorToGo.r > mColor.r)
            mColor.r++;
        if (mColorToGo.g > mColor.g)
            mColor.g++;
        if (mColorToGo.b > mColor.b)
            mColor.b++;
        if (mColorToGo.r < mColor.r)
            mColor.r--;
        if (mColorToGo.g < mColor.g)
            mColor.g--;
        if (mColorToGo.b < mColor.b)
            mColor.b--;
    }

    if (mSmoothProgress && mProgressToGo != mProgress)
    {
        // Smoothly showing the progressbar changes.
        if (mProgressToGo > mProgress)
            mProgress = std::min(1.0f, mProgress + 0.005f);
        if (mProgressToGo < mProgress)
            mProgress = std::max(0.0f, mProgress - 0.005f);
    }
}

void ProgressBar::draw(gcn::Graphics *graphics)
{
    mColor.a = gui->getTheme()->getGuiAlpha();

    gcn::Rectangle rect = getDimension();
    rect.x = 0;
    rect.y = 0;

    Theme::ProgressPalette palette = Theme::THEME_PROG_END;
    if (mProgressPalette >= 0)
        palette = static_cast<Theme::ProgressPalette>(mProgressPalette);

    gui->getTheme()->drawProgressBar(static_cast<Graphics *>(graphics),
                                     rect,
                                     mColor,
                                     mProgress,
                                     mText,
                                     palette);
}

void ProgressBar::setProgress(float progress)
{
    const float p = std::min(1.0f, std::max(0.0f, progress));
    mProgressToGo = p;

    if (!mSmoothProgress)
        mProgress = p;

    if (mProgressPalette >= 0)
        mColorToGo = Theme::getProgressColor(mProgressPalette, progress);
}

void ProgressBar::setProgressPalette(int progressPalette)
{
    int oldPalette = mProgressPalette;
    mProgressPalette = progressPalette;

    if (mProgressPalette != oldPalette && mProgressPalette >= 0)
        mColorToGo = Theme::getProgressColor(mProgressPalette, mProgressToGo);
}

void ProgressBar::setColor(const gcn::Color &color)
{
    mColorToGo = color;

    if (!mSmoothColorChange)
        mColor = color;
}
