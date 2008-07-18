/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id: inttextbox.cpp 3035 2007-01-14 14:54:39Z b_lindeijer $
 */

#include "inttextbox.h"

#include <guichan/key.hpp>

#include "../utils/tostring.h"

IntTextBox::IntTextBox(int i):
    mValue(i)
{
}

void
IntTextBox::keyPressed(gcn::KeyEvent &event)
{
    const gcn::Key &key = event.getKey();

    if (key.isNumber() || key.getValue() == gcn::Key::BACKSPACE
        || key.getValue() == gcn::Key::DELETE)
    {
        gcn::TextBox::keyPressed(event);
    }

    std::stringstream s(gcn::TextBox::getText());
    int i;
    s >> i;
    if (gcn::TextBox::getText() != "")
        setInt(i);
}

void IntTextBox::setRange(int min, int max)
{
    mMin = min;
    mMax = max;
}

int IntTextBox::getInt()
{
    if (gcn::TextBox::getText() == "")
        return 0;
    return mValue;
}

void IntTextBox::setInt(int i)
{
    if (i >= mMin && i <= mMax)
        mValue = i;

    const std::string valStr = toString(mValue);
    setText(valStr);
    setCaretPosition(valStr.length() + 1);
}
