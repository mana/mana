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
 *  $Id$
 */

#include "inttextbox.h"

IntTextBox::IntTextBox()
    : value(0)
{
}

IntTextBox::IntTextBox(int i)
    : value(i)
{
}

void IntTextBox::keyPress(const gcn::Key &key)
{
    if (key.isNumber() || key.getValue() == gcn::Key::BACKSPACE
        || key.getValue() == gcn::Key::DELETE)
    {
        gcn::TextBox::keyPress(key);
    }

    std::stringstream s;
    int i;
    s << gcn::TextBox::getText();
    s >> i;
    if (gcn::TextBox::getText() != "")
        setInt(i);
}

void IntTextBox::setRange(int minValue, int maxValue)
{
    min = minValue;
    max = maxValue;
}

int IntTextBox::getInt()
{
    if (gcn::TextBox::getText() == "")
        return 0;
    return value;
}

void IntTextBox::setInt(int i)
{
    std::stringstream s;

    if (i >= min && i <= max)
        value = i;
    s << value;
    setText(s.str());
    setCaretPosition(s.str().length() + 1);
}


