/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "gui/widgets/tab.h"

#include "gui/widgets/tabbedarea.h"

#include "gui/palette.h"

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

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
    int gridX;
    int gridY;
};

static TabData const data[TAB_COUNT] = {
    { "graphics/gui/tab.png", 0, 0 },
    { "graphics/gui/tab.png", 9, 4 },
    { "graphics/gui/tabselected.png", 16, 19 },
    { "graphics/gui/tab.png", 25, 23 }
};

ImageRect Tab::tabImg[TAB_COUNT];

Tab::Tab() : gcn::Tab(),
    mTabColor(&guiPalette->getColor(Palette::TEXT))
{
    init();
}

Tab::~Tab()
{
    mInstances--;

    if (mInstances == 0)
    {
        for (int mode = 0; mode < TAB_COUNT; mode++)
        {
            for_each(tabImg[mode].grid, tabImg[mode].grid + 9, dtor<Image*>());
        }
    }
}

void Tab::init()
{
    setFrameSize(0);
    mHighlighted = false;

    if (mInstances == 0)
    {
        // Load the skin
        ResourceManager *resman = ResourceManager::getInstance();
        Image *tab[TAB_COUNT];

        int a, x, y, mode;

        for (mode = 0; mode < TAB_COUNT; mode++)
        {
            tab[mode] = resman->getImage(data[mode].file);
            a = 0;
            for (y = 0; y < 3; y++)
            {
                for (x = 0; x < 3; x++)
                {
                    tabImg[mode].grid[a] = tab[mode]->getSubImage(
                            data[x].gridX, data[y].gridY,
                            data[x + 1].gridX - data[x].gridX + 1,
                            data[y + 1].gridY - data[y].gridY + 1);
                    tabImg[mode].grid[a]->setAlpha(mAlpha);
                    a++;
                }
            }
            tab[mode]->decRef();
        }
    }
    mInstances++;
}

void Tab::draw(gcn::Graphics *graphics)
{
    int mode = TAB_STANDARD;

    // check which type of tab to draw
    if (mTabbedArea)
    {
        if (mTabbedArea->isTabSelected(this))
        {
            mode = TAB_SELECTED;
            // if tab is selected, it doesnt need to highlight activity
            mLabel->setForegroundColor(*mTabColor);
            mHighlighted = false;
        }
        else if (mHighlighted)
        {
            mode = TAB_HIGHLIGHTED;
            mLabel->setForegroundColor(guiPalette->getColor(Palette::TAB_HIGHLIGHT));
        }
        else
        {
            mLabel->setForegroundColor(*mTabColor);
        }
    }

    // TODO We don't need to do this for every tab on every draw
    // Maybe use a config listener to do it as the value changes.
    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        mAlpha = config.getValue("guialpha", 0.8);
        for (int a = 0; a < 9; a++)
        {
            for (int t = 0; t < TAB_COUNT; t++)
            {
                tabImg[t].grid[a]->setAlpha(mAlpha);
            }
        }
    }

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

void Tab::setHighlighted(bool high)
{
    mHighlighted = high;
}
