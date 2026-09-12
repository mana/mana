/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "gui/widgets/inttextfield.h"

#include "gui/sdlinput.h"

#include "utils/stringutils.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

IntTextField::IntTextField(int value):
    TextField(toString(value)),
    mValue(value)
{
}

void IntTextField::keyPressed(gcn::KeyEvent &event)
{
    const std::string before = getText();

    TextField::keyPressed(event);

    if (getText() != before)
        textChanged();
}

void IntTextField::textInput(const TextInput &textInput)
{
    const std::string &text = textInput.getText();
    if (text.empty())
        return;

    for (const char c : text)
        if (!std::isdigit(static_cast<unsigned char>(c)))
            return;

    TextField::textInput(textInput);
    textChanged();
}

void IntTextField::textChanged()
{
    // Leave an emptied field empty so that a new value can be typed. In that
    // case getValue() reports the minimum value.
    if (!getText().empty())
    {
        const long value = std::strtol(getText().c_str(), nullptr, 10);
        setValue(static_cast<int>(std::clamp<long>(value, mMin, mMax)));
    }

    distributeActionEvent();
}

void IntTextField::setRange(int min, int max)
{
    mMin = min;
    mMax = max;

    if (mValue < mMin)
        mValue = mMin;
    else if (mValue > mMax)
        mValue = mMax;
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
