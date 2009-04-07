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

#include "gui/widgets/checkbox.h"

#include "gui/palette.h"

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

int CheckBox::instances = 0;
float CheckBox::mAlpha = 1.0;
Image *CheckBox::checkBoxNormal;
Image *CheckBox::checkBoxChecked;
Image *CheckBox::checkBoxDisabled;
Image *CheckBox::checkBoxDisabledChecked;

CheckBox::CheckBox(const std::string &caption, bool selected):
    gcn::CheckBox(caption, selected)
{
    if (instances == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        Image *checkBox = resman->getImage("graphics/gui/checkbox.png");
        checkBoxNormal = checkBox->getSubImage(0, 0, 9, 10);
        checkBoxChecked = checkBox->getSubImage(9, 0, 9, 10);
        checkBoxDisabled = checkBox->getSubImage(18, 0, 9, 10);
        checkBoxDisabledChecked = checkBox->getSubImage(27, 0, 9, 10);
        checkBoxNormal->setAlpha(mAlpha);
        checkBoxChecked->setAlpha(mAlpha);
        checkBoxDisabled->setAlpha(mAlpha);
        checkBoxDisabledChecked->setAlpha(mAlpha);
        checkBox->decRef();
    }

    instances++;
}

CheckBox::~CheckBox()
{
    instances--;

    if (instances == 0)
    {
        delete checkBoxNormal;
        delete checkBoxChecked;
        delete checkBoxDisabled;
        delete checkBoxDisabledChecked;
    }
}

void CheckBox::draw(gcn::Graphics* graphics)
{
    drawBox(graphics);

    graphics->setFont(getFont());
    graphics->setColor(guiPalette->getColor(Palette::TEXT));

    const int h = getHeight() + getHeight() / 2;

    graphics->drawText(getCaption(), h - 2, 0);
}

void CheckBox::drawBox(gcn::Graphics* graphics)
{
    Image *box;

    if (isSelected())
    {
        if (isEnabled())
            box = checkBoxChecked;
        else
            box = checkBoxDisabledChecked;
    }
    else if (isEnabled())
        box = checkBoxNormal;
    else
        box = checkBoxDisabled;

    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        mAlpha = config.getValue("guialpha", 0.8);
        checkBoxNormal->setAlpha(mAlpha);
        checkBoxChecked->setAlpha(mAlpha);
        checkBoxDisabled->setAlpha(mAlpha);
        checkBoxDisabledChecked->setAlpha(mAlpha);
    }

    static_cast<Graphics*>(graphics)->drawImage(box, 2, 2);
}
