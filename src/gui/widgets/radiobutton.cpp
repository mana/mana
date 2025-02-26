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

#include "gui/widgets/radiobutton.h"

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/theme.h"

int RadioButton::instances = 0;
float RadioButton::mAlpha = 1.0;
ResourceRef<Image> RadioButton::radioNormal;
ResourceRef<Image> RadioButton::radioChecked;
ResourceRef<Image> RadioButton::radioDisabled;
ResourceRef<Image> RadioButton::radioDisabledChecked;
ResourceRef<Image> RadioButton::radioNormalHi;
ResourceRef<Image> RadioButton::radioCheckedHi;

RadioButton::RadioButton(const std::string &caption, const std::string &group,
        bool marked):
    gcn::RadioButton(caption, group, marked)
{
    if (instances == 0)
    {
        radioNormal = Theme::getImageFromTheme("radioout.png");
        radioChecked = Theme::getImageFromTheme("radioin.png");
        radioDisabled = Theme::getImageFromTheme("radioout.png");
        radioDisabledChecked = Theme::getImageFromTheme("radioin.png");
        radioNormalHi = Theme::getImageFromTheme("radioout_highlight.png");
        radioCheckedHi = Theme::getImageFromTheme("radioin_highlight.png");
        radioNormal->setAlpha(mAlpha);
        radioChecked->setAlpha(mAlpha);
        radioDisabled->setAlpha(mAlpha);
        radioDisabledChecked->setAlpha(mAlpha);
        radioNormalHi->setAlpha(mAlpha);
        radioCheckedHi->setAlpha(mAlpha);
    }

    instances++;
}

RadioButton::~RadioButton()
{
    instances--;

    if (instances == 0)
    {
        radioNormal = nullptr;
        radioChecked = nullptr;
        radioDisabled = nullptr;
        radioDisabledChecked = nullptr;
        radioNormalHi = nullptr;
        radioCheckedHi = nullptr;
    }
}

void RadioButton::drawBox(gcn::Graphics* graphics)
{
    if (config.guiAlpha != mAlpha)
    {
        mAlpha = config.guiAlpha;
        radioNormal->setAlpha(mAlpha);
        radioChecked->setAlpha(mAlpha);
        radioDisabled->setAlpha(mAlpha);
        radioDisabledChecked->setAlpha(mAlpha);
        radioNormalHi->setAlpha(mAlpha);
        radioCheckedHi->setAlpha(mAlpha);
    }

    Image *box = nullptr;

    if (isEnabled())
        if (isSelected())
            if (mHasMouse)
                box = radioCheckedHi;
            else
                box = radioChecked;
        else
            if (mHasMouse)
                box = radioNormalHi;
            else
                box = radioNormal;
    else
        if (isSelected())
            box = radioDisabledChecked;
        else
            box = radioDisabled;

    if (box)
        static_cast<Graphics*>(graphics)->drawImage(box, 2, 2);
}

void RadioButton::draw(gcn::Graphics* graphics)
{
    graphics->pushClipArea(gcn::Rectangle(1, 1, getWidth() - 1,
                                          getHeight() - 1));

    drawBox(graphics);

    graphics->popClipArea();

    graphics->setFont(getFont());
    graphics->setColor(getForegroundColor());

    int h = getHeight() + getHeight() / 2;
    graphics->drawText(getCaption(), h - 2, 0);
}

void RadioButton::mouseEntered(gcn::MouseEvent& event)
{
    mHasMouse = true;
}

void RadioButton::mouseExited(gcn::MouseEvent& event)
{
    mHasMouse = false;
}
