/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "resources/userpalette.h"

#include "configuration.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

static constexpr const char *ColorTypeNames[] = {
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
    "HitPlayerMonster",
    "HitMonsterPlayer",
    "HitCritical",
    "HitLocalPlayerMonster",
    "HitLocalPlayerCritical",
    "HitLocalPlayerMiss",
    "Miss"
};

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
    addColor(HIT_PLAYER_MONSTER, 0x0064ff, STATIC,
             _("Other Player Hits Monster"));
    addColor(HIT_MONSTER_PLAYER, 0xff3232, STATIC, _("Monster Hits Player"));
    addColor(HIT_CRITICAL, 0xff0000, RAINBOW, _("Critical Hit"));
    addColor(HIT_LOCAL_PLAYER_MONSTER, 0x00ff00, STATIC,
             _("Local Player Hits Monster"));
    addColor(HIT_LOCAL_PLAYER_CRITICAL, 0xff0000, RAINBOW,
             _("Local Player Critical Hit"));
    addColor(HIT_LOCAL_PLAYER_MISS, 0x00ffa6, STATIC,
             _("Local Player Miss"));
    addColor(MISS, 0xffff00, STATIC, _("Misses"));
    commit(true);
}

UserPalette::~UserPalette()
{
    config.colors.clear();

    // TODO: Don't write out colors when they have the default value
    for (auto &color : mColors)
    {
        auto &configColor = config.colors[ColorTypeNames[color.type]];

        configColor.gradient = color.committedGrad;

        if (color.grad != STATIC && color.delay != GRADIENT_DELAY)
            configColor.delay = color.delay;

        if (color.grad == STATIC || color.grad == PULSE)
            configColor.color = strprintf("0x%06x", color.getRGB());
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
    for (auto &color : mColors)
    {
        color.committedGrad = color.grad;
        color.committedDelay = color.delay;
        if (commitNonStatic || color.grad == STATIC)
        {
            color.committedColor = color.color;
        }
        else if (color.grad == PULSE)
        {
            color.committedColor = color.testColor;
        }
    }
}

void UserPalette::rollback()
{
    for (auto &color : mColors)
    {
        if (color.grad != color.committedGrad)
        {
            setGradient(color.type, color.committedGrad);
        }
        setGradientDelay(color.type, color.committedDelay);
        setColor(color.type, color.committedColor.r,
                 color.committedColor.g, color.committedColor.b);
        if (color.grad == PULSE)
        {
            color.testColor.r = color.committedColor.r;
            color.testColor.g = color.committedColor.g;
            color.testColor.b = color.committedColor.b;
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

void UserPalette::addColor(int type, unsigned rgb, GradientType grad,
                           const std::string &text, int delay)
{
    auto colorIt = config.colors.find(ColorTypeNames[type]);
    if (colorIt != config.colors.end())
    {
        const UserColor &userColor = colorIt->second;
        rgb = atox(userColor.color);
        grad = static_cast<GradientType>(userColor.gradient);
        delay = userColor.delay.value_or(GRADIENT_DELAY);
    }

    mColors[type].set(type, gcn::Color(rgb), grad, delay);
    mColors[type].text = text;

    if (grad != STATIC)
        mGradVector.push_back(&mColors[type]);
}
