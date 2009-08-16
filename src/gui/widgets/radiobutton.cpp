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

#include "gui/widgets/radiobutton.h"

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

int RadioButton::instances = 0;
float RadioButton::mAlpha = 1.0;
Image *RadioButton::radioNormal;
Image *RadioButton::radioChecked;
Image *RadioButton::radioDisabled;
Image *RadioButton::radioDisabledChecked;
Image *RadioButton::radioNormalHi;
Image *RadioButton::radioCheckedHi;

RadioButton::RadioButton(const std::string &caption, const std::string &group,
        bool marked):
    gcn::RadioButton(caption, group, marked),
    mHasMouse(false)
{
    if (instances == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        radioNormal = resman->getImage("graphics/gui/radioout.png");
        radioChecked = resman->getImage("graphics/gui/radioin.png");
        radioDisabled = resman->getImage("graphics/gui/radioout.png");
        radioDisabledChecked = resman->getImage("graphics/gui/radioin.png");
        radioNormalHi = resman->getImage("graphics/gui/radioout_highlight.png");
        radioCheckedHi = resman->getImage("graphics/gui/radioin_highlight.png");
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
        radioNormal->decRef();
        radioChecked->decRef();
        radioDisabled->decRef();
        radioDisabledChecked->decRef();
        radioNormalHi->decRef();
        radioCheckedHi->decRef();
    }
}

void RadioButton::drawBox(gcn::Graphics* graphics)
{
    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        mAlpha = config.getValue("guialpha", 0.8);
        radioNormal->setAlpha(mAlpha);
        radioChecked->setAlpha(mAlpha);
        radioDisabled->setAlpha(mAlpha);
        radioDisabledChecked->setAlpha(mAlpha);
        radioNormalHi->setAlpha(mAlpha);
        radioCheckedHi->setAlpha(mAlpha);
    }

    Image *box = NULL;

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

