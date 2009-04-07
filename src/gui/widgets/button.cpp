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

#include "gui/widgets/button.h"

#include "gui/palette.h"

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"

#include <guichan/exception.hpp>
#include <guichan/font.hpp>

int Button::mInstances = 0;
float Button::mAlpha = 1.0;

enum{
    BUTTON_STANDARD,    // 0
    BUTTON_HIGHLIGHTED, // 1
    BUTTON_PRESSED,     // 2
    BUTTON_DISABLED,    // 3
    BUTTON_COUNT        // 4 - Must be last.
};

struct ButtonData
{
    char const *file;
    int gridX;
    int gridY;
};

static ButtonData const data[BUTTON_COUNT] = {
    { "graphics/gui/button.png", 0, 0 },
    { "graphics/gui/buttonhi.png", 9, 4 },
    { "graphics/gui/buttonpress.png", 16, 19 },
    { "graphics/gui/button_disabled.png", 25, 23 }
};

ImageRect Button::button[BUTTON_COUNT];

Button::Button()
{
    init();
}

Button::Button(const std::string &caption, const std::string &actionEventId,
    gcn::ActionListener *listener):
    gcn::Button(caption)
{
    init();
    setActionEventId(actionEventId);

    if (listener)
        addActionListener(listener);
}

void Button::init()
{
    setFrameSize(0);

    if (mInstances == 0)
    {
        // Load the skin
        ResourceManager *resman = ResourceManager::getInstance();
        Image *btn[BUTTON_COUNT];

        int a, x, y, mode;

        for (mode = 0; mode < BUTTON_COUNT; mode++)
        {
            btn[mode] = resman->getImage(data[mode].file);
            a = 0;
            for (y = 0; y < 3; y++)
            {
                for (x = 0; x < 3; x++)
                {
                    button[mode].grid[a] = btn[mode]->getSubImage(
                            data[x].gridX, data[y].gridY,
                            data[x + 1].gridX - data[x].gridX + 1,
                            data[y + 1].gridY - data[y].gridY + 1);
                    button[mode].grid[a]->setAlpha(mAlpha);
                    a++;
                }
            }
            btn[mode]->decRef();
        }
    }
    mInstances++;
}

Button::~Button()
{
    mInstances--;

    if (mInstances == 0)
    {
        for (int mode = 0; mode < BUTTON_COUNT; mode++)
        {
            for_each(button[mode].grid, button[mode].grid + 9, dtor<Image*>());
        }
    }
}

void Button::draw(gcn::Graphics *graphics)
{
    int mode;

    if (!isEnabled())
        mode = BUTTON_DISABLED;
    else if (isPressed())
        mode = BUTTON_PRESSED;
    else if (mHasMouse || isFocused())
        mode = BUTTON_HIGHLIGHTED;
    else
        mode = BUTTON_STANDARD;

    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        mAlpha = config.getValue("guialpha", 0.8);
        for (int a = 0; a < 9; a++)
        {
            button[BUTTON_DISABLED].grid[a]->setAlpha(mAlpha);
            button[BUTTON_PRESSED].grid[a]->setAlpha(mAlpha);
            button[BUTTON_HIGHLIGHTED].grid[a]->setAlpha(mAlpha);
            button[BUTTON_STANDARD].grid[a]->setAlpha(mAlpha);
        }
    }

    static_cast<Graphics*>(graphics)->
        drawImageRect(0, 0, getWidth(), getHeight(), button[mode]);

    graphics->setColor(guiPalette->getColor(Palette::TEXT));

    int textX;
    int textY = getHeight() / 2 - getFont()->getHeight() / 2;

    switch (getAlignment())
    {
        case gcn::Graphics::LEFT:
            textX = 4;
            break;
        case gcn::Graphics::CENTER:
            textX = getWidth() / 2;
            break;
        case gcn::Graphics::RIGHT:
            textX = getWidth() - 4;
            break;
        default:
            throw GCN_EXCEPTION("Button::draw. Unknown alignment.");
    }

    graphics->setFont(getFont());

    if (isPressed())
        graphics->drawText(getCaption(), textX + 1, textY + 1, getAlignment());
    else
        graphics->drawText(getCaption(), textX, textY, getAlignment());
}
