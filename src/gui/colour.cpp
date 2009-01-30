/***************************************************************************
 *   Copyright (C) 2008 by Douglas Boffey                                  *
 *                                                                         *
 *       DougABoffey@netscape.net                                          *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed with The Mana Experiment                  *
 *   in the hope that it will be useful,                                   *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <cstdio>

#include "colour.h"

#include "../configuration.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

Colour::Colour()
{
    addColour('C', 0x000000, _("Chat"));
    addColour('G', 0xff0000, _("GM"));
    addColour('Y', 0x1fa052, _("Player"));
    addColour('W', 0x0000ff, _("Whisper"));
    addColour('I', 0xf1dc27, _("Is"));
    addColour('P', 0xff00d8, _("Party"));
    addColour('S', 0x8415e2, _("Server"));
    addColour('L', 0x919191, _("Logger"));
    addColour('<', 0xe50d0d, _("Hyperlink"));
    commit();
}

Colour::~Colour()
{
    for (ColVector::iterator col = mColVector.begin(),
             colEnd = mColVector.end();
         col != colEnd;
         ++col)
    {
        config.setValue("Colour" + col->text, toString(col->rgb));
    }
}

void Colour::setColour(const char c, const int rgb)
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

int Colour::getColour(const char c, bool &valid) const
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

std::string Colour::getElementAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return "";
    }
    return mColVector[i].text;
}

void Colour::addColour(const char c, const int rgb, const std::string &text)
{
    int trueRgb = config.getValue("Colour" + text, rgb);
    mColVector.push_back(ColourElem(c, trueRgb, text));
}

int Colour::getColourAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return 0;
    }
    return mColVector[i].rgb;
}

void Colour::setColourAt(int i, int rgb)
{
    if (i >= 0 && i < getNumberOfElements())
    {
        mColVector[i].rgb = rgb;
    }
}

void Colour::commit()
{
    for (ColVector::iterator i = mColVector.begin(), iEnd = mColVector.end();
         i != iEnd;
         ++i)
    {
        i->committedRgb = i->rgb;
    }
}

void Colour::rollback()
{
    for (ColVector::iterator i = mColVector.begin(), iEnd = mColVector.end();
         i != iEnd;
         ++i)
    {
        i->rgb = i->committedRgb;
    }
}
