/*
 *  Aethyra
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *
 *  This file is part of Aethyra.
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

#include "inttextfield.h"
#include "sdlinput.h"

#include "../utils/stringutils.h"

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
}

int IntTextField::getValue()
{
    return getText().empty() ? mMin : mValue;
}

void IntTextField::setValue(int i)
{
    if (i >= mMin && i <= mMax)
        mValue = i;
    else if (i < mMin)
        mValue = mMin;
    else if (i > mMax)
        mValue = mMax;

    const std::string valStr = toString(mValue);
    setText(valStr);
    setCaretPosition(valStr.length() + 1);
}

void IntTextField::reset()
{
    setValue(mDefault);
}
