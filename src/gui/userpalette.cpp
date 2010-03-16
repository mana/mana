/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "userpalette.h"

#include "configuration.h"
#include "client.h"

#include "gui/gui.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <math.h>

const std::string ColorTypeNames[] = {
    "Being",
    "Player",
    "Self",
    "GM",
    "NPC",
    "Monster",
    "Party",
    "Guild",
    "Particle",
    "Experience",
    "Pickup",
    "Hit Player Monster",
    "Hit Monster Player",
    "Hit Critical",
    "Miss"
};

std::string UserPalette::getConfigName(const std::string &typeName)
{
    std::string res = "Color" + typeName;

    int pos = 5;
    for (size_t i = 0; i < typeName.length(); i++)
    {
        if (i == 0 || typeName[i] == '_')
        {
            if (i > 0)
                i++;

            res[pos] = typeName[i];
        }
        else
        {
           res[pos] = tolower(typeName[i]);
        }
        pos++;
    }
    res.erase(pos, res.length() - pos);

    return res;
}

UserPalette::UserPalette():
        Palette(USER_COLOR_LAST)
{
    mColors[BEING] = ColorElem();
    mColors[PC] = ColorElem();
    mColors[SELF] = ColorElem();
    mColors[GM] = ColorElem();
    mColors[NPC] = ColorElem();
    mColors[MONSTER] = ColorElem();

    addColor(BEING, 0xffffff, STATIC, _("Being"));
    addColor(PC, 0xffffff, STATIC, _("Other Players' Names"));
    addColor(SELF, 0xff8040, STATIC, _("Own Name"));
    addColor(GM, 0x00ff00, STATIC, _("GM Names"));
    addColor(NPC, 0xc8c8ff, STATIC, _("NPCs"));
    addColor(MONSTER, 0xff4040, STATIC, _("Monsters"));
    addColor(PARTY, 0xff00d8, STATIC, _("Party Members"));
    addColor(GUILD, 0xff00d8, STATIC, _("Guild Members"));
    addColor(PARTICLE, 0xffffff, STATIC, _("Particle Effects"));
    addColor(PICKUP_INFO, 0x28dc28, STATIC, _("Pickup Notification"));
    addColor(EXP_INFO, 0xffff00, STATIC, _("Exp Notification"));
    addColor(HIT_PLAYER_MONSTER, 0x0064ff, STATIC, _("Player Hits Monster"));
    addColor(HIT_MONSTER_PLAYER, 0xff3232, STATIC, _("Monster Hits Player"));
    addColor(HIT_CRITICAL, 0xff0000, RAINBOW, _("Critical Hit"));
    addColor(MISS, 0xffff00, STATIC, _("Misses"));
    commit(true);
}

UserPalette::~UserPalette()
{
    for (Colors::iterator col = mColors.begin(),
         colEnd = mColors.end(); col != colEnd; ++col)
    {
        const std::string &configName = ColorTypeNames[col->type];
        config.setValue(configName + "Gradient", col->committedGrad);

        if (col->grad != STATIC)
            config.setValue(configName + "Delay", col->delay);

        if (col->grad == STATIC || col->grad == PULSE)
        {
            char buffer[20];
            sprintf(buffer, "0x%06x", col->getRGB());
            config.setValue(configName, std::string(buffer));
        }
    }
}

void UserPalette::setColor(int type, int r, int g, int b)
{
    mColors[type].color.r = r;
    mColors[type].color.g = g;
    mColors[type].color.b = b;
}

void UserPalette::setGradient(int type, GradientType grad)
{
    ColorElem *elem = &mColors[type];
    if (elem->grad != STATIC && grad == STATIC)
    {
        for (size_t i = 0; i < mGradVector.size(); i++)
        {
            if (mGradVector[i] == elem)
            {
                mGradVector.erase(mGradVector.begin() + i);
                break;
            }
        }
    }
    else if (elem->grad == STATIC && grad != STATIC)
    {
        mGradVector.push_back(elem);
    }

    if (elem->grad != grad)
    {
        elem->grad = grad;
    }
}

std::string UserPalette::getElementAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return "";
    }
    return mColors[i].text;
}

void UserPalette::commit(bool commitNonStatic)
{
    for (Colors::iterator i = mColors.begin(), iEnd = mColors.end();
         i != iEnd; ++i)
    {
        i->committedGrad = i->grad;
        i->committedDelay = i->delay;
        if (commitNonStatic || i->grad == STATIC)
        {
            i->committedColor = i->color;
        }
        else if (i->grad == PULSE)
        {
            i->committedColor = i->testColor;
        }
    }
}

void UserPalette::rollback()
{
    for (Colors::iterator i = mColors.begin(), iEnd = mColors.end();
         i != iEnd; ++i)
    {
        if (i->grad != i->committedGrad)
        {
            setGradient(i->type, i->committedGrad);
        }
        setGradientDelay(i->type, i->committedDelay);
        setColor(i->type, i->committedColor.r,
                 i->committedColor.g, i->committedColor.b);
        if (i->grad == PULSE)
        {
            i->testColor.r = i->committedColor.r;
            i->testColor.g = i->committedColor.g;
            i->testColor.b = i->committedColor.b;
        }
    }
}

int UserPalette::getColorTypeAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return BEING;
    }

    return mColors[i].type;
}

void UserPalette::addColor(int type, int rgb, Palette::GradientType grad,
                           const std::string &text, int delay)
{
    const std::string &configName = ColorTypeNames[type];
    char buffer[20];
    sprintf(buffer, "0x%06x", rgb);
    const std::string rgbString = config.getValue(configName,
                                                  std::string(buffer));
    unsigned int rgbValue = 0;
    if (rgbString.length() == 8 && rgbString[0] == '0' && rgbString[1] == 'x')
        rgbValue = atox(rgbString);
    else
        rgbValue = atoi(rgbString.c_str());
    gcn::Color trueCol = rgbValue;
    grad = (GradientType) config.getValue(configName + "Gradient", grad);
    delay = (int) config.getValue(configName + "Delay", delay);
    mColors[type].set(type, trueCol, grad, delay);
    mColors[type].text = text;

    if (grad != STATIC)
        mGradVector.push_back(&mColors[type]);
}
