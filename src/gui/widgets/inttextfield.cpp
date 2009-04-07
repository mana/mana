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

#include "gui/widgets/inttextfield.h"

#include "gui/sdlinput.h"

#include "utils/stringutils.h"

IntTextField::IntTextField(int def):
    TextField(toString(def)),
    mDefault(def),
    mValue(def)
{
}

void IntTextField::keyPressed(gcn::KeyEvent &event)
{
    const gcn::Key &key = event.getKey();

    if (key.getValue() == Key::BACKSPACE ||
        key.getValue() == Key::DELETE)
    {
        setText(std::string());
        event.consume();
    }

    if (!key.isNumber())
        return;

    TextField::keyPressed(event);

    std::istringstream s(getText());
    int i;
    s >> i;
    setValue(i);
}

void IntTextField::setRange(int min, int max)
{
    mMin = min;
    mMax = max;

    if (mValue < mMin)
        mValue = mMin;
    else if (mValue > mMax)
        mValue = mMax;

    if (mDefault < mMin)
        mDefault = mMin;
    else if (mDefault > mMax)
        mDefault = mMax;
}

int IntTextField::getValue()
{
    return getText().empty() ? mMin : mValue;
}

void IntTextField::setValue(int i)
{
    if (i < mMin)
        mValue = mMin;
    else if (i > mMax)
        mValue = mMax;
    else
        mValue = i;

    const std::string valStr = toString(mValue);
    setText(valStr);
    setCaretPosition(valStr.length() + 1);
}

void IntTextField::setDefaultValue(int value)
{
    if (value < mMin)
        mDefault = mMin;
    else if (value > mMax)
        mDefault = mMax;
    else
        mDefault = value;
}

void IntTextField::reset()
{
    setValue(mDefault);
}
