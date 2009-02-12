/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
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

#include "color.h"

#include "../configuration.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

Color::Color()
{
    addColor('C', 0x000000, _("Chat"));
    addColor('G', 0xff0000, _("GM"));
    addColor('H', 0xebc873, _("Highlight"));
    addColor('Y', 0x1fa052, _("Player"));
    addColor('W', 0x0000ff, _("Whisper"));
    addColor('I', 0xa08527, _("Is"));
    addColor('P', 0xff00d8, _("Party"));
    addColor('S', 0x8415e2, _("Server"));
    addColor('L', 0x919191, _("Logger"));
    addColor('<', 0xe50d0d, _("Hyperlink"));
    commit();
}

Color::~Color()
{
    for (ColVector::iterator col = mColVector.begin(),
             colEnd = mColVector.end();
         col != colEnd;
         ++col)
    {
        config.setValue("color" + col->text, toString(col->rgb));
    }
}

void Color::setColor(const char c, const int rgb)
{
    for (ColVector::iterator col = mColVector.begin(),
             colEnd = mColVector.end();
         col != colEnd;
         ++col)
    {
        if (col->ch == c)
        {
            col->rgb = rgb;
            return;
        }
    }
}

int Color::getColor(const char c, bool &valid) const
{
    for (ColVector::const_iterator col = mColVector.begin(),
             colEnd = mColVector.end();
         col != colEnd;
         ++col)
    {
        if (col->ch == c)
        {
            valid = true;
            return col->rgb;
        }
    }
    valid = false;
    return 0x000000;
}

std::string Color::getElementAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return "";
    }
    return mColVector[i].text;
}

char Color::getColorCharAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return 'C';
    }
    return mColVector[i].ch;
}

void Color::addColor(const char c, const int rgb, const std::string &text)
{
    int trueRgb = (int)config.getValue("color" + text, rgb);
    mColVector.push_back(colorElem(c, trueRgb, text));
}

int Color::getColorAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return 0;
    }
    return mColVector[i].rgb;
}

void Color::setColorAt(int i, int rgb)
{
    if (i >= 0 && i < getNumberOfElements())
    {
        mColVector[i].rgb = rgb;
    }
}

void Color::commit()
{
    for (ColVector::iterator i = mColVector.begin(), iEnd = mColVector.end();
         i != iEnd;
         ++i)
    {
        i->committedRgb = i->rgb;
    }
}

void Color::rollback()
{
    for (ColVector::iterator i = mColVector.begin(), iEnd = mColVector.end();
         i != iEnd;
         ++i)
    {
        i->rgb = i->committedRgb;
    }
}
