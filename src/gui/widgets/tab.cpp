/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "gui/widgets/tab.h"

#include "configuration.h"
#include "graphics.h"

#include "gui/palette.h"

#include "gui/widgets/tabbedarea.h"

#include "resources/image.h"
#include "resources/theme.h"

#include "utils/dtor.h"

#include <guichan/widgets/label.hpp>

int Tab::mInstances = 0;
float Tab::mAlpha = 1.0;

enum {
    TAB_STANDARD,    // 0
    TAB_HIGHLIGHTED, // 1
    TAB_SELECTED,    // 2
    TAB_UNUSED,      // 3
    TAB_COUNT        // 4 - Must be last.
};

struct TabData
{
    char const *file;
    int gridX[4];
    int gridY[4];
};

static TabData const data[TAB_COUNT] = {
    { "tab.png", {0, 9, 16, 25}, {0, 13, 19, 20} },
    { "tab_hilight.png", {0, 9, 16, 25}, {0, 13, 19, 20} },
    { "tabselected.png", {0, 9, 16, 25}, {0, 4, 12, 20} },
    { "tab.png", {0, 9, 16, 25}, {0, 13, 19, 20} }
};

ImageRect Tab::tabImg[TAB_COUNT];

Tab::Tab() : gcn::Tab(),
    mTabColor(&Theme::getThemeColor(Theme::TAB))
{
    init();
}

Tab::~Tab()
{
    mInstances--;

    if (mInstances == 0)
    {
        for (auto &imgRect : tabImg)
        {
            std::for_each(imgRect.grid, imgRect.grid + 9, dtor<Image*>());
        }
    }
}

void Tab::init()
{
    setFocusable(false);
    setFrameSize(0);
    mFlash = false;

    if (mInstances == 0)
    {
        // Load the skin
        Image *tab[TAB_COUNT];

        int a, x, y, mode;

        for (mode = 0; mode < TAB_COUNT; mode++)
        {
            tab[mode] = Theme::getImageFromTheme(data[mode].file);
            a = 0;
            for (y = 0; y < 3; y++)
            {
                for (x = 0; x < 3; x++)
                {
                    tabImg[mode].grid[a] = tab[mode]->getSubImage(
                            data[mode].gridX[x], data[mode].gridY[y],
                            data[mode].gridX[x + 1] - data[mode].gridX[x] + 1,
                            data[mode].gridY[y + 1] - data[mode].gridY[y] + 1);
                    a++;
                }
            }
            tabImg[mode].setAlpha(mAlpha);
            tab[mode]->decRef();
        }
    }
    mInstances++;
}

void Tab::updateAlpha()
{
    float alpha = std::max(config.getFloatValue("guialpha"),
                           Theme::instance()->getMinimumOpacity());

    // TODO We don't need to do this for every tab on every draw
    // Maybe use a config listener to do it as the value changes.
    if (alpha != mAlpha)
    {
        mAlpha = alpha;

        for (auto &t : tabImg)
        {
            t.setAlpha(mAlpha);
        }
    }
}

void Tab::draw(gcn::Graphics *graphics)
{
    int mode = TAB_STANDARD;

    // check which type of tab to draw
    if (mTabbedArea)
    {
        mLabel->setForegroundColor(*mTabColor);
        if (mTabbedArea->isTabSelected(this))
        {
            mode = TAB_SELECTED;
            // if tab is selected, it doesnt need to highlight activity
            mFlash = false;
        }
        else if (mHasMouse)
        {
            mode = TAB_HIGHLIGHTED;
        }
        if (mFlash)
        {
            mLabel->setForegroundColor(Theme::getThemeColor(Theme::TAB_FLASH));
        }
    }

    updateAlpha();

    // draw tab
    static_cast<Graphics*>(graphics)->
        drawImageRect(0, 0, getWidth(), getHeight(), tabImg[mode]);

    // draw label
    drawChildren(graphics);
}

void Tab::setTabColor(const gcn::Color *color)
{
    mTabColor = color;
}

void Tab::setFlash(bool flash)
{
    mFlash = flash;
}
